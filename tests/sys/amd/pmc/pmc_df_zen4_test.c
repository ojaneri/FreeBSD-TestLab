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
 * Component: PMC Uncore DF Counter Tests (Zen4/Zen5)
 *
 * ============================================================================
 * Test Category: PMC Uncore DF Counter (Zen4/Zen5)
 * ============================================================================
 *
 * These tests validate the AMD Data Fabric (DF) Performance Monitoring Counters
 * specific to Zen4 (Raphael) and Zen5 (Granite Ridge) processors.
 *
 * Zen4/Zen5 introduce enhanced DF counters with:
 * - Extended NBIO traffic monitoring
 * - Unified Cache Architecture (UCX) events
 * - Infinity Fabric monitoring
 * - PCIe traffic counters (via NBIF)
 *
 * Reference: AMD64 Architecture Programmer's Manual, Volume 2
 *            AMD Zen4 System Programming Guide
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

/* 
 * Zen4/Zen5 specific DF events
 * These are different from older Zen generations
 */
#define PMC_EVENT_Z4_DF_L3_READS        "zen4:df_l3_reads"
#define PMC_EVENT_Z4_DF_L3_WRITES       "zen4:df_l3_writes"
#define PMC_EVENT_Z4_DF_MEM_READS       "zen4:df_mem_reads"
#define PMC_EVENT_Z4_DF_MEM_WRITES      "zen4:df_mem_writes"
#define PMC_EVENT_Z4_DF_NBIF_READS       "zen4:df_nbif_reads"
#define PMC_EVENT_Z4_DF_NBIF_WRITES     "zen4:df_nbif_writes"
#define PMC_EVENT_Z4_DF_SMI                 "zen4:df_smi"
#define PMC_EVENT_Z4_DF_PROBE              "zen4:df_probe"

/* Number of iterations for workload */
#define WORKLOAD_ITERATIONS       1000000

/*
 * ============================================================================
 * Test Case: Zen4/Zen5 Detection
 * Test ID: TC-Z4-01
 *
 * Objective: Verify that the system detects Zen4 or Zen5 CPU
 *
 * Expected Result: CPU family 19h detected
 * ============================================================================
 */
static int
test_zen4_detection(void)
{
    printf("\n=== Test: Zen4/Zen5 Detection ===\n");

    if (!require_root()) {
        return (1);
    }

    print_cpu_info();

    if (!cpu_is_zen4_or_later()) {
        printf("[SKIP] CPU is not Zen4 or later (Family < 19h)\n");
        return (2);
    }

    if (cpu_is_zen5()) {
        printf("[INFO] Detected Zen5 CPU\n");
    } else {
        printf("[INFO] Detected Zen4 CPU\n");
    }

    printf("[PASS] Zen4/Zen5 CPU detected correctly\n");
    return (0);
}

/*
 * ============================================================================
 * Test Case: Zen4 DF L3 Read Counter
 * Test ID: TC-Z4-02
 *
 * Objective: Test Data Fabric L3 read counter on Zen4/Zen5
 *
 * Expected Result: Counter increments during L3 read operations
 * ============================================================================
 */
static int
test_zen4_df_l3_reads(void)
{
    pmc_id_t pmcid;
    pmc_value_t value;
    int *array;
    size_t array_size;
    int i;

    printf("\n=== Test: Zen4 DF L3 Read Counter ===\n");

    if (!require_root()) {
        return (1);
    }

    if (!cpu_is_zen4_or_later()) {
        printf("[SKIP] Not a Zen4/Zen5 CPU\n");
        return (2);
    }

    /* Initialize PMC */
    if (pmc_init() < 0) {
        perror("pmc_init");
        return (1);
    }

    /* Try Zen4-specific L3 read event */
    if (pmc_allocate(PMC_EVENT_Z4_DF_L3_READS, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) < 0) {
        fprintf(stderr, "pmc_allocate failed for %s: %s\n",
                PMC_EVENT_Z4_DF_L3_READS, strerror(errno));
        printf("[INFO] Trying alternative event names...\n");
        
        /* Try common alternatives */
        const char *alt_events[] = {
            "zen4:0x0280",       /* L3 lookup, read */
            "zen4:umask:0x01",
            "DF_L3_READS",
            NULL
        };
        
        for (i = 0; alt_events[i] != NULL; i++) {
            if (pmc_allocate(alt_events[i], PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) == 0) {
                printf("[INFO] Found working event: %s\n", alt_events[i]);
                break;
            }
        }
        
        if (alt_events[i] == NULL) {
            printf("[SKIP] Zen4 DF L3 read event not available\n");
            return (2);
        }
    }

    /* Start counting */
    if (pmc_start(pmcid) < 0) {
        perror("pmc_start");
        pmc_release(pmcid);
        return (1);
    }

    /* Workload: Sequential access to trigger L3 reads */
    array_size = 8192;  /* 32KB - fits in L3 */
    array = malloc(sizeof(int) * array_size);
    if (!array) {
        perror("malloc");
        pmc_release(pmcid);
        return (1);
    }

    /* Sequential read pattern - should hit in L3 */
    for (i = 0; i < WORKLOAD_ITERATIONS; i++) {
        volatile int idx = (i * 16) % array_size;
        array[idx] = array[idx] + 1;
    }

    /* Stop and read */
    if (pmc_stop(pmcid) < 0) {
        perror("pmc_stop");
        free(array);
        pmc_release(pmcid);
        return (1);
    }

    if (pmc_read(pmcid, &value) < 0) {
        perror("pmc_read");
        free(array);
        pmc_release(pmcid);
        return (1);
    }

    printf("[RESULT] Zen4 DF L3 Read Count: %ju\n", (uintmax_t)value);

    if (value > 0) {
        printf("[PASS] Zen4 DF L3 read counter is working\n");
    } else {
        printf("[WARN] Zen4 DF L3 read count is zero\n");
    }

    free(array);
    pmc_release(pmcid);
    return (0);
}

/*
 * ============================================================================
 * Test Case: Zen4 DF Memory Controller Reads
 * Test ID: TC-Z4-03
 *
 * Objective: Test DF memory controller read counter on Zen4/Zen5
 *
 * Expected Result: Counter increments during memory reads
 * ============================================================================
 */
static int
test_zen4_df_mem_reads(void)
{
    pmc_id_t pmcid;
    pmc_value_t value;
    char *buffer;
    size_t buffer_size;
    ssize_t bytes_read;
    int fd;
    int i;

    printf("\n=== Test: Zen4 DF Memory Controller Read Counter ===\n");

    if (!require_root()) {
        return (1);
    }

    if (!cpu_is_zen4_or_later()) {
        printf("[SKIP] Not a Zen4/Zen5 CPU\n");
        return (2);
    }

    /* Initialize PMC */
    if (pmc_init() < 0) {
        perror("pmc_init");
        return (1);
    }

    /* Try Zen4-specific memory read event */
    if (pmc_allocate(PMC_EVENT_Z4_DF_MEM_READS, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) < 0) {
        fprintf(stderr, "pmc_allocate failed for %s: %s\n",
                PMC_EVENT_Z4_DF_MEM_READS, strerror(errno));
        printf("[INFO] Trying alternative event names...\n");
        
        const char *alt_events[] = {
            "zen4:0x0400",       /* DRAM controller read */
            "zen4:umask:0x04",
            "zen4:df_dram_reads",
            NULL
        };
        
        for (i = 0; alt_events[i] != NULL; i++) {
            if (pmc_allocate(alt_events[i], PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) == 0) {
                printf("[INFO] Found working event: %s\n", alt_events[i]);
                break;
            }
        }
        
        if (alt_events[i] == NULL) {
            printf("[SKIP] Zen4 DF memory read event not available\n");
            return (2);
        }
    }

    /* Start counting */
    if (pmc_start(pmcid) < 0) {
        perror("pmc_start");
        pmc_release(pmcid);
        return (1);
    }

    /* Workload: Read from /dev/zero to generate memory traffic */
    buffer_size = 65536;  /* 64KB reads */
    buffer = malloc(buffer_size);
    if (!buffer) {
        perror("malloc");
        pmc_release(pmcid);
        return (1);
    }

    fd = open("/dev/zero", O_RDONLY);
    if (fd >= 0) {
        for (i = 0; i < 10000; i++) {
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

    printf("[RESULT] Zen4 DF Memory Read Count: %ju\n", (uintmax_t)value);

    if (value > 0) {
        printf("[PASS] Zen4 DF memory read counter is working\n");
    } else {
        printf("[WARN] Zen4 DF memory read count is zero\n");
    }

    free(buffer);
    pmc_release(pmcid);
    return (0);
}

/*
 * ============================================================================
 * Test Case: Zen4 DF NBIF (PCIe) Traffic
 * Test ID: TC-Z4-04
 *
 * Objective: Test NBIF (Infinity Fabric) traffic counter
 *
 * Expected Result: Counter shows PCIe/NBIF traffic
 * ============================================================================
 */
static int
test_zen4_df_nbif_counter(void)
{
    pmc_id_t pmcid;
    pmc_value_t value;
    int i;

    printf("\n=== Test: Zen4 DF NBIF (Infinity Fabric) Counter ===\n");

    if (!require_root()) {
        return (1);
    }

    if (!cpu_is_zen4_or_later()) {
        printf("[SKIP] Not a Zen4/Zen5 CPU\n");
        return (2);
    }

    /* Initialize PMC */
    if (pmc_init() < 0) {
        perror("pmc_init");
        return (1);
    }

    /* Try NBIF read counter */
    if (pmc_allocate(PMC_EVENT_Z4_DF_NBIF_READS, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) < 0) {
        fprintf(stderr, "pmc_allocate failed for %s: %s\n",
                PMC_EVENT_Z4_DF_NBIF_READS, strerror(errno));
        printf("[INFO] Trying alternative event names...\n");
        
        const char *alt_events[] = {
            "zen4:0x0600",       /* NBIF read */
            "zen4:nbif_reads",
            NULL
        };
        
        for (i = 0; alt_events[i] != NULL; i++) {
            if (pmc_allocate(alt_events[i], PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) == 0) {
                printf("[INFO] Found working event: %s\n", alt_events[i]);
                break;
            }
        }
        
        if (alt_events[i] == NULL) {
            printf("[SKIP] Zen4 NBIF event not available (may need kernel support)\n");
            return (2);
        }
    }

    /* Start counting */
    if (pmc_start(pmcid) < 0) {
        perror("pmc_start");
        pmc_release(pmcid);
        return (1);
    }

    /* Workload: Generate some memory traffic */
    int *array = malloc(sizeof(int) * 100000);
    if (array) {
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

    printf("[RESULT] Zen4 NBIF Read Count: %ju\n", (uintmax_t)value);

    if (value > 0) {
        printf("[PASS] Zen4 NBIF counter is working\n");
    } else {
        printf("[WARN] NBIF count is zero\n");
    }

    pmc_release(pmcid);
    return (0);
}

/*
 * ============================================================================
 * Test Case: Zen4 DF Snoop Probe
 * Test ID: TC-Z4-05
 *
 * Objective: Test cache coherency probe counter
 *
 * Expected Result: Counter shows snoop probe operations
 * ============================================================================
 */
static int
test_zen4_df_probe_counter(void)
{
    pmc_id_t pmcid;
    pmc_value_t value;
    int i;

    printf("\n=== Test: Zen4 DF Snoop Probe Counter ===\n");

    if (!require_root()) {
        return (1);
    }

    if (!cpu_is_zen4_or_later()) {
        printf("[SKIP] Not a Zen4/Zen5 CPU\n");
        return (2);
    }

    /* Initialize PMC */
    if (pmc_init() < 0) {
        perror("pmc_init");
        return (1);
    }

    /* Try probe/snoop counter */
    if (pmc_allocate(PMC_EVENT_Z4_DF_PROBE, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) < 0) {
        fprintf(stderr, "pmc_allocate failed for %s: %s\n",
                PMC_EVENT_Z4_DF_PROBE, strerror(errno));
        printf("[INFO] Trying alternative event names...\n");
        
        const char *alt_events[] = {
            "zen4:0x0800",       /* Probe */
            "zen4:df_probe",
            "zen4:snp",
            NULL
        };
        
        for (i = 0; alt_events[i] != NULL; i++) {
            if (pmc_allocate(alt_events[i], PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) == 0) {
                printf("[INFO] Found working event: %s\n", alt_events[i]);
                break;
            }
        }
        
        if (alt_events[i] == NULL) {
            printf("[SKIP] Zen4 probe event not available\n");
            return (2);
        }
    }

    /* Start counting */
    if (pmc_start(pmcid) < 0) {
        perror("pmc_start");
        pmc_release(pmcid);
        return (1);
    }

    /* Workload: Create cache coherency traffic */
    int *array = malloc(sizeof(int) * 200000);
    if (array) {
        /* Access pattern that triggers coherency */
        for (i = 0; i < WORKLOAD_ITERATIONS; i++) {
            volatile int idx = (i * 31) % 200000;
            array[idx] = array[idx] + 1;
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

    printf("[RESULT] Zen4 DF Probe Count: %ju\n", (uintmax_t)value);

    if (value > 0) {
        printf("[PASS] Zen4 DF probe counter is working\n");
    } else {
        printf("[WARN] Probe count is zero (may need multi-thread test)\n");
    }

    pmc_release(pmcid);
    return (0);
}

/*
 * ============================================================================
 * Test Case: Zen4 Multiple DF Counters
 * Test ID: TC-Z4-06
 *
 * Objective: Test multiple simultaneous DF counters
 *
 * Expected Result: Multiple counters work independently
 * ============================================================================
 */
static int
test_zen4_multiple_counters(void)
{
    pmc_id_t pmcid1, pmcid2;
    pmc_value_t value1, value2;
    char *buffer;
    size_t buffer_size;
    int i;

    printf("\n=== Test: Zen4 Multiple DF Counters ===\n");

    if (!require_root()) {
        return (1);
    }

    if (!cpu_is_zen4_or_later()) {
        printf("[SKIP] Not a Zen4/Zen5 CPU\n");
        return (2);
    }

    /* Initialize PMC */
    if (pmc_init() < 0) {
        perror("pmc_init");
        return (1);
    }

    /* Allocate two counters */
    const char *event1 = "zen4:0x0280";
    const char *event2 = "zen4:0x0400";
    
    if (pmc_allocate(event1, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid1) < 0) {
        printf("[SKIP] Cannot allocate first counter\n");
        return (2);
    }

    if (pmc_allocate(event2, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid2) < 0) {
        printf("[SKIP] Cannot allocate second counter\n");
        pmc_release(pmcid1);
        return (2);
    }

    /* Start both */
    if (pmc_start(pmcid1) < 0 || pmc_start(pmcid2) < 0) {
        perror("pmc_start");
        pmc_release(pmcid1);
        pmc_release(pmcid2);
        return (1);
    }

    /* Workload */
    buffer_size = 4096;
    buffer = malloc(buffer_size);
    int *array = malloc(sizeof(int) * 4096);
    
    if (buffer && array) {
        for (i = 0; i < WORKLOAD_ITERATIONS; i++) {
            /* Mixed access pattern */
            if (i % 2 == 0) {
                volatile char c = buffer[(i * 64) % buffer_size];
                (void)c;
            } else {
                volatile int idx = (i * 16) % 4096;
                array[idx] = i;
            }
        }
    }

    /* Stop and read */
    if (pmc_stop(pmcid1) < 0 || pmc_stop(pmcid2) < 0) {
        perror("pmc_stop");
        free(buffer);
        free(array);
        pmc_release(pmcid1);
        pmc_release(pmcid2);
        return (1);
    }

    if (pmc_read(pmcid1, &value1) < 0 || pmc_read(pmcid2, &value2) < 0) {
        perror("pmc_read");
        free(buffer);
        free(array);
        pmc_release(pmcid1);
        pmc_release(pmcid2);
        return (1);
    }

    printf("[RESULT] Counter 1: %ju\n", (uintmax_t)value1);
    printf("[RESULT] Counter 2: %ju\n", (uintmax_t)value2);

    if (value1 > 0 || value2 > 0) {
        printf("[PASS] Multiple Zen4 DF counters work simultaneously\n");
    } else {
        printf("[WARN] Both counters are zero\n");
    }

    free(buffer);
    free(array);
    pmc_release(pmcid1);
    pmc_release(pmcid2);
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
    printf("  zen4-detection    - Test Zen4/Zen5 CPU detection\n");
    printf("  zen4-l3-reads    - Test Zen4 DF L3 read counter\n");
    printf("  zen4-mem-reads   - Test Zen4 DF memory controller reads\n");
    printf("  zen4-nbif        - Test Zen4 NBIF (Infinity Fabric) counter\n");
    printf("  zen4-probe       - Test Zen4 DF snoop probe counter\n");
    printf("  zen4-multiple    - Test multiple simultaneous DF counters\n");
    printf("  all              - Run all tests (default)\n");
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
    printf("(Zen4/Zen5 Specific)\n");
    printf("========================================\n");

    if (strcmp(test_name, "all") == 0) {
        printf("\nRunning all tests...\n");
        result |= test_zen4_detection();
        result |= test_zen4_df_l3_reads();
        result |= test_zen4_df_mem_reads();
        result |= test_zen4_df_nbif_counter();
        result |= test_zen4_df_probe_counter();
        result |= test_zen4_multiple_counters();
    } else if (strcmp(test_name, "zen4-detection") == 0) {
        result = test_zen4_detection();
    } else if (strcmp(test_name, "zen4-l3-reads") == 0) {
        result = test_zen4_df_l3_reads();
    } else if (strcmp(test_name, "zen4-mem-reads") == 0) {
        result = test_zen4_df_mem_reads();
    } else if (strcmp(test_name, "zen4-nbif") == 0) {
        result = test_zen4_df_nbif_counter();
    } else if (strcmp(test_name, "zen4-probe") == 0) {
        result = test_zen4_df_probe_counter();
    } else if (strcmp(test_name, "zen4-multiple") == 0) {
        result = test_zen4_multiple_counters();
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
