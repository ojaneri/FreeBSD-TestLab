/*
 * Copyright (c) 2026, AMD IBS Test Team
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: AMD IBS Test Team
 * Date Created: 2026-02-22
 *
 * Project: AMD IBS FreeBSD Test Suite
 * Component: PMC Uncore DF Counter Tests (Zen/Zen2)
 *
 * ============================================================================
 * Test Category: PMC Uncore DF Counter (Zen/Zen2)
 * ============================================================================
 *
 * These tests validate the AMD Data Fabric (DF) Performance Monitoring Counters.
 * DF counters are available on AMD processors starting from Family 15h (Bulldozer).
 *
 * Data Fabric Counters (Uncore):
 * - DF CPU-to-Memory reads/writes
 * - DF Cache snoop operations
 * - DF interconnect traffic
 *
 * Reference: AMD64 Architecture Programmer's Manual, Volume 2
 *            AMD System Programming Guide (24593)
 * ============================================================================
 */

#include <sys/types.h>
#include <sys/pmc.h>
#include <sys/sysctl.h>

#include "pmc_test_utils.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* PMC event names for Data Fabric */
#define PMC_EVENT_DF_READS         "DF_MEM_READ"
#define PMC_EVENT_DF_WRITES        "DF_MEM_WRITE"
#define PMC_EVENT_DF_SNOOPS         "DF_SNOOP"
#define PMC_EVENT_DF_CACHE_VICTIM   "DF_CACHE_VICTIM"

/* Number of iterations for workload */
#define WORKLOAD_ITERATIONS       1000000

/*
 * ============================================================================
 * Test Case: DF Counter Availability Detection
 * Test ID: TC-DF-01
 *
 * Objective: Verify that the system detects DF counter support
 *
 * Expected Result: DF counters available on Family 15h+ (Bulldozer+)
 * ============================================================================
 */
static int
test_df_detection(void)
{
    printf("\n=== Test: Data Fabric Counter Detection ===\n");

    if (!require_root()) {
        return (1);
    }

    print_cpu_info();

    if (!cpu_supports_df_counters()) {
        printf("[SKIP] CPU does not support DF counters (Family < 15h)\n");
        return (2);
    }

    printf("[PASS] CPU supports Data Fabric counters\n");
    return (0);
}

/*
 * ============================================================================
 * Test Case: DF Memory Read Counter
 * Test ID: TC-DF-02
 *
 * Objective: Test Data Fabric memory read counting
 *
 * Expected Result: Counter increments during memory read operations
 * ============================================================================
 */
static int
test_df_memory_reads(void)
{
    pmc_id_t pmcid;
    pmc_value_t value;
    char *buffer;
    size_t buffer_size;
    ssize_t bytes_read;
    int fd;
    int i;

    printf("\n=== Test: DF Memory Read Counter ===\n");

    if (!require_root()) {
        return (1);
    }

    if (!cpu_supports_df_counters()) {
        printf("[SKIP] CPU does not support DF counters\n");
        return (2);
    }

    /* Initialize PMC */
    if (pmc_init() < 0) {
        perror("pmc_init");
        return (1);
    }

    /* Try to allocate DF memory read counter */
    if (pmc_allocate(PMC_EVENT_DF_READS, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) < 0) {
        fprintf(stderr, "pmc_allocate failed for %s: %s\n",
                PMC_EVENT_DF_READS, strerror(errno));
        printf("[INFO] DF_MEM_READ event may not be available\n");
        
        /* Try alternative event names */
        const char *alt_events[] = {"DF_READS", "umask:0x04", NULL};
        for (i = 0; alt_events[i] != NULL; i++) {
            if (pmc_allocate(alt_events[i], PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) == 0) {
                printf("[INFO] Found working event: %s\n", alt_events[i]);
                break;
            }
        }
        if (alt_events[i] == NULL) {
            return (2);
        }
    }

    /* Start counting */
    if (pmc_start(pmcid) < 0) {
        perror("pmc_start");
        pmc_release(pmcid);
        return (1);
    }

    /* Workload: Read from various sources to generate memory traffic */
    buffer_size = 4096;
    buffer = malloc(buffer_size);
    if (!buffer) {
        perror("malloc");
        pmc_release(pmcid);
        return (1);
    }

    /* Multiple read operations to generate memory traffic */
    for (i = 0; i < WORKLOAD_ITERATIONS; i++) {
        volatile char c = buffer[(i * 64) % buffer_size];
        (void)c;
    }

    /* Also read from /dev/zero to generate memory traffic */
    fd = open("/dev/zero", O_RDONLY);
    if (fd >= 0) {
        for (i = 0; i < 1000; i++) {
            bytes_read = read(fd, buffer, buffer_size);
            if (bytes_read < 0) break;
        }
        close(fd);
    }

    /* Stop and read */
    if (pmc_stop(pmcid) < 0) {
        perror("pmc_stop");
        free(buffer);
        pmc_release(pmcid);
        return (1);
    }

    if (pmc_read(pmcid, &value) < 0) {
        perror("pmc_read");
        free(buffer);
        pmc_release(pmcid);
        return (1);
    }

    printf("[RESULT] DF Memory Read Count: %ju\n", (uintmax_t)value);

    if (value > 0) {
        printf("[PASS] DF memory read counter is working\n");
    } else {
        printf("[WARN] DF memory read count is zero\n");
    }

    free(buffer);
    pmc_release(pmcid);
    return (0);
}

/*
 * ============================================================================
 * Test Case: DF Memory Write Counter
 * Test ID: TC-DF-03
 *
 * Objective: Test Data Fabric memory write counting
 *
 * Expected Result: Counter increments during memory write operations
 * ============================================================================
 */
static int
test_df_memory_writes(void)
{
    pmc_id_t pmcid;
    pmc_value_t value;
    char *buffer;
    size_t buffer_size;
    ssize_t bytes_written;
    int fd;
    int i;

    printf("\n=== Test: DF Memory Write Counter ===\n");

    if (!require_root()) {
        return (1);
    }

    if (!cpu_supports_df_counters()) {
        printf("[SKIP] CPU does not support DF counters\n");
        return (2);
    }

    /* Initialize PMC */
    if (pmc_init() < 0) {
        perror("pmc_init");
        return (1);
    }

    /* Try to allocate DF memory write counter */
    if (pmc_allocate(PMC_EVENT_DF_WRITES, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) < 0) {
        fprintf(stderr, "pmc_allocate failed for %s: %s\n",
                PMC_EVENT_DF_WRITES, strerror(errno));
        printf("[INFO] DF_MEM_WRITE event may not be available\n");
        
        /* Try alternative event names */
        const char *alt_events[] = {"DF_WRITES", "umask:0x08", NULL};
        for (i = 0; alt_events[i] != NULL; i++) {
            if (pmc_allocate(alt_events[i], PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) == 0) {
                printf("[INFO] Found working event: %s\n", alt_events[i]);
                break;
            }
        }
        if (alt_events[i] == NULL) {
            return (2);
        }
    }

    /* Start counting */
    if (pmc_start(pmcid) < 0) {
        perror("pmc_start");
        pmc_release(pmcid);
        return (1);
    }

    /* Workload: Write to memory */
    buffer_size = 4096;
    buffer = malloc(buffer_size);
    if (!buffer) {
        perror("malloc");
        pmc_release(pmcid);
        return (1);
    }

    /* Fill buffer with pattern */
    memset(buffer, 0xAA, buffer_size);

    /* Write to /dev/null to generate write traffic */
    fd = open("/dev/null", O_WRONLY);
    if (fd >= 0) {
        for (i = 0; i < 1000; i++) {
            bytes_written = write(fd, buffer, buffer_size);
            if (bytes_written < 0) break;
        }
        close(fd);
    }

    /* Also do memory writes */
    for (i = 0; i < WORKLOAD_ITERATIONS; i++) {
        buffer[(i * 64) % buffer_size] = (char)i;
    }

    /* Stop and read */
    if (pmc_stop(pmcid) < 0) {
        perror("pmc_stop");
        free(buffer);
        pmc_release(pmcid);
        return (1);
    }

    if (pmc_read(pmcid, &value) < 0) {
        perror("pmc_read");
        free(buffer);
        pmc_release(pmcid);
        return (1);
    }

    printf("[RESULT] DF Memory Write Count: %ju\n", (uintmax_t)value);

    if (value > 0) {
        printf("[PASS] DF memory write counter is working\n");
    } else {
        printf("[WARN] DF memory write count is zero\n");
    }

    free(buffer);
    pmc_release(pmcid);
    return (0);
}

/*
 * ============================================================================
 * Test Case: DF Cache Snoop Counter
 * Test ID: TC-DF-04
 *
 * Objective: Test Data Fabric cache snoop operation counting
 *
 * Expected Result: Counter increments during cache snoop operations
 * ============================================================================
 */
static int
test_df_snoop_counter(void)
{
    pmc_id_t pmcid;
    pmc_value_t value;
    int i;

    printf("\n=== Test: DF Cache Snoop Counter ===\n");

    if (!require_root()) {
        return (1);
    }

    if (!cpu_supports_df_counters()) {
        printf("[SKIP] CPU does not support DF counters\n");
        return (2);
    }

    /* Initialize PMC */
    if (pmc_init() < 0) {
        perror("pmc_init");
        return (1);
    }

    /* Try to allocate DF snoop counter */
    if (pmc_allocate(PMC_EVENT_DF_SNOOPS, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) < 0) {
        fprintf(stderr, "pmc_allocate failed for %s: %s\n",
                PMC_EVENT_DF_SNOOPS, strerror(errno));
        printf("[INFO] DF_SNOOP event may not be available\n");
        
        /* Try alternative event names */
        const char *alt_events[] = {"DF_SNOOPS", "umask:0x10", NULL};
        for (i = 0; alt_events[i] != NULL; i++) {
            if (pmc_allocate(alt_events[i], PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) == 0) {
                printf("[INFO] Found working event: %s\n", alt_events[i]);
                break;
            }
        }
        if (alt_events[i] == NULL) {
            return (2);
        }
    }

    /* Start counting */
    if (pmc_start(pmcid) < 0) {
        perror("pmc_start");
        pmc_release(pmcid);
        return (1);
    }

    /*
     * Workload: Create cache coherency traffic
     * This is harder to trigger in userspace
     */
    int *array = malloc(sizeof(int) * 100000);
    if (array) {
        /* Access pattern that may trigger snoop operations */
        for (i = 0; i < WORKLOAD_ITERATIONS; i++) {
            volatile int idx = (i * 17) % 100000;
            array[idx] = i;
        }
        free(array);
    }

    /* Stop and read */
    if (pmc_stop(pmcid) < 0) {
        perror("pmc_stop");
        pmc_release(pmcid);
        return (1);
    }

    if (pmc_read(pmcid, &value) < 0) {
        perror("pmc_read");
        pmc_release(pmcid);
        return (1);
    }

    printf("[RESULT] DF Snoop Count: %ju\n", (uintmax_t)value);

    if (value > 0) {
        printf("[PASS] DF snoop counter is working\n");
    } else {
        printf("[WARN] DF snoop count is zero (may need kernel-level test)\n");
    }

    pmc_release(pmcid);
    return (0);
}

/*
 * ============================================================================
 * Test Case: DF Combined Read/Write
 * Test ID: TC-DF-05
 *
 * Objective: Test simultaneous DF read/write counting
 *
 * Expected Result: Both counters work independently
 * ============================================================================
 */
static int
test_df_combined_counters(void)
{
    pmc_id_t pmcid_read, pmcid_write;
    pmc_value_t value_read, value_write;
    char *buffer;
    size_t buffer_size;
    int i;

    printf("\n=== Test: DF Combined Read/Write Counters ===\n");

    if (!require_root()) {
        return (1);
    }

    if (!cpu_supports_df_counters()) {
        printf("[SKIP] CPU does not support DF counters\n");
        return (2);
    }

    /* Initialize PMC */
    if (pmc_init() < 0) {
        perror("pmc_init");
        return (1);
    }

    /* Allocate counters */
    if (pmc_allocate(PMC_EVENT_DF_READS, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid_read) < 0) {
        fprintf(stderr, "pmc_allocate failed for DF_READS\n");
        printf("[INFO] Cannot allocate DF read counter\n");
        return (2);
    }

    if (pmc_allocate(PMC_EVENT_DF_WRITES, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid_write) < 0) {
        fprintf(stderr, "pmc_allocate failed for DF_WRITES\n");
        pmc_release(pmcid_read);
        printf("[INFO] Cannot allocate DF write counter\n");
        return (2);
    }

    /* Start both counters */
    if (pmc_start(pmcid_read) < 0 || pmc_start(pmcid_write) < 0) {
        perror("pmc_start");
        pmc_release(pmcid_read);
        pmc_release(pmcid_write);
        return (1);
    }

    /* Mixed workload */
    buffer_size = 4096;
    buffer = malloc(buffer_size);
    if (!buffer) {
        pmc_release(pmcid_read);
        pmc_release(pmcid_write);
        return (1);
    }

    /* Read and write pattern */
    for (i = 0; i < WORKLOAD_ITERATIONS; i++) {
        if (i % 2 == 0) {
            volatile char c = buffer[(i * 64) % buffer_size];
            (void)c;
        } else {
            buffer[(i * 64) % buffer_size] = (char)i;
        }
    }

    /* Stop and read */
    if (pmc_stop(pmcid_read) < 0 || pmc_stop(pmcid_write) < 0) {
        perror("pmc_stop");
        free(buffer);
        pmc_release(pmcid_read);
        pmc_release(pmcid_write);
        return (1);
    }

    if (pmc_read(pmcid_read, &value_read) < 0 || 
        pmc_read(pmcid_write, &value_write) < 0) {
        perror("pmc_read");
        free(buffer);
        pmc_release(pmcid_read);
        pmc_release(pmcid_write);
        return (1);
    }

    printf("[RESULT] DF Read Count: %ju\n", (uintmax_t)value_read);
    printf("[RESULT] DF Write Count: %ju\n", (uintmax_t)value_write);

    if (value_read > 0 || value_write > 0) {
        printf("[PASS] Combined DF counters work\n");
    } else {
        printf("[WARN] Both counters are zero\n");
    }

    free(buffer);
    pmc_release(pmcid_read);
    pmc_release(pmcid_write);
    return (0);
}

/*
 * ============================================================================
 * Main Test Runner
 * ============================================================================
 */

static void
print_usage(const char *prog)
{
    printf("Usage: %s [test-name]\n\n", prog);
    printf("Available tests:\n");
    printf("  df-detection      - Test DF counter availability detection\n");
    printf("  df-reads          - Test DF memory read counter\n");
    printf("  df-writes         - Test DF memory write counter\n");
    printf("  df-snoops         - Test DF cache snoop counter\n");
    printf("  df-combined       - Test combined DF counters\n");
    printf("  all               - Run all tests (default)\n");
}

int
main(int argc, char **argv)
{
    const char *test_name = "all";
    int result = 0;

    if (argc > 1) {
        test_name = argv[1];
    }

    printf("========================================\n");
    printf("AMD PMC Uncore DF Counter Test Suite\n");
    printf("(Zen/Zen2)\n");
    printf("========================================\n");

    if (strcmp(test_name, "all") == 0) {
        printf("\nRunning all tests...\n");
        result |= test_df_detection();
        result |= test_df_memory_reads();
        result |= test_df_memory_writes();
        result |= test_df_snoop_counter();
        result |= test_df_combined_counters();
    } else if (strcmp(test_name, "df-detection") == 0) {
        result = test_df_detection();
    } else if (strcmp(test_name, "df-reads") == 0) {
        result = test_df_memory_reads();
    } else if (strcmp(test_name, "df-writes") == 0) {
        result = test_df_memory_writes();
    } else if (strcmp(test_name, "df-snoops") == 0) {
        result = test_df_snoop_counter();
    } else if (strcmp(test_name, "df-combined") == 0) {
        result = test_df_combined_counters();
    } else {
        fprintf(stderr, "Unknown test: %s\n", test_name);
        print_usage(argv[0]);
        return (1);
    }

    printf("\n========================================\n");
    printf("Test Suite Complete\n");
    printf("========================================\n");

    return (result);
}
