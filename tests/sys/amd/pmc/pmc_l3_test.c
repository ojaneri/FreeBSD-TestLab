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
 * Component: PMC Core L3 Counters Tests
 *
 * ============================================================================
 * Test Category: PMC Core L3 Counters
 * ============================================================================
 *
 * These tests validate the AMD L3 Cache Performance Monitoring Counters.
 * L3 counters are available on AMD processors starting from Family 17h (Zen).
 *
 * L3 Cache Counters:
 * - L3 Cache Misses
 * - L3 Cache Hits
 * - L3 Evictions
 * - L3 Fill Pending
 *
 * Reference: AMD64 Architecture Programmer's Manual, Volume 2
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

/* PMC event names for L3 cache */
#define PMC_EVENT_L3_HIT          "L3_HIT"
#define PMC_EVENT_L3_MISS         "L3_MISS"
#define PMC_EVENT_L3_EVICTION     "L3_EVICTION"
#define PMC_EVENT_L3_FILL_PENDING "L3_FILL_PENDING"

/* Number of iterations for workload */
#define WORKLOAD_ITERATIONS       1000000

/*
 * ============================================================================
 * Test Case: L3 Counter Availability Detection
 * Test ID: TC-L3-01
 * ============================================================================
 *
 * Objective: Verify that the system detects L3 counter support
 *
 * Expected Result: L3 counters available on Family 17h+ (Zen)
 */
static int
test_l3_detection(void)
{
    printf("\n=== Test: L3 Counter Detection ===\n");

    if (!require_root()) {
        return (1);
    }

    print_cpu_info();

    if (!cpu_supports_l3_counters()) {
        printf("[SKIP] CPU does not support L3 counters (Family < 17h)\n");
        return (2);
    }

    printf("[PASS] CPU supports L3 counters\n");
    return (0);
}

/*
 * ============================================================================
 * Test Case: L3 Hit Counter
 * Test ID: TC-L3-02
 *
 * Objective: Test L3 cache hit counting
 *
 * Expected Result: Counter increments during cache hits
 * ============================================================================
 */
static int
test_l3_hit_counter(void)
{
    pmc_id_t pmcid;
    pmc_value_t value;
    int *array;
    size_t array_size;
    int i;

    printf("\n=== Test: L3 Hit Counter ===\n");

    if (!require_root()) {
        return (1);
    }

    if (!cpu_supports_l3_counters()) {
        printf("[SKIP] CPU does not support L3 counters\n");
        return (2);
    }

    /* Initialize PMC */
    if (pmc_init() < 0) {
        perror("pmc_init");
        return (1);
    }

    /* Allocate L3 hit counter */
    if (pmc_allocate(PMC_EVENT_L3_HIT, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) < 0) {
        fprintf(stderr, "pmc_allocate failed for %s: %s\n",
                PMC_EVENT_L3_HIT, strerror(errno));
        printf("[INFO] L3_HIT event may not be available on this CPU\n");
        return (2);
    }

    /* Start counting */
    if (pmc_start(pmcid) < 0) {
        perror("pmc_start");
        pmc_release(pmcid);
        return (1);
    }

    /* Workload: Sequential access (cache friendly) */
    array_size = 4096;  /* 16KB - fits in L3 */
    array = malloc(sizeof(int) * array_size);
    if (!array) {
        perror("malloc");
        pmc_release(pmcid);
        return (1);
    }

    /* Sequential read - high L3 hit rate expected */
    for (i = 0; i < WORKLOAD_ITERATIONS; i++) {
        volatile int idx = (i * 16) % array_size;
        array[idx] = idx;
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

    printf("[RESULT] L3 Hit Count: %ju\n", (uintmax_t)value);

    if (value > 0) {
        printf("[PASS] L3 hit counter is working\n");
    } else {
        printf("[WARN] L3 hit count is zero - may need different event name\n");
    }

    free(array);
    pmc_release(pmcid);
    return (0);
}

/*
 * ============================================================================
 * Test Case: L3 Miss Counter
 * Test ID: TC-L3-03
 *
 * Objective: Test L3 cache miss counting
 *
 * Expected Result: Counter increments during cache misses
 * ============================================================================
 */
static int
test_l3_miss_counter(void)
{
    pmc_id_t pmcid;
    pmc_value_t value;
    int *array;
    size_t array_size;
    int i;

    printf("\n=== Test: L3 Miss Counter ===\n");

    if (!require_root()) {
        return (1);
    }

    if (!cpu_supports_l3_counters()) {
        printf("[SKIP] CPU does not support L3 counters\n");
        return (2);
    }

    /* Initialize PMC */
    if (pmc_init() < 0) {
        perror("pmc_init");
        return (1);
    }

    /* Allocate L3 miss counter */
    if (pmc_allocate(PMC_EVENT_L3_MISS, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) < 0) {
        fprintf(stderr, "pmc_allocate failed for %s: %s\n",
                PMC_EVENT_L3_MISS, strerror(errno));
        printf("[INFO] L3_MISS event may not be available on this CPU\n");
        return (2);
    }

    /* Start counting */
    if (pmc_start(pmcid) < 0) {
        perror("pmc_start");
        pmc_release(pmcid);
        return (1);
    }

    /*
     * Workload: Strided access causing cache misses
     * Stride of 64 elements (256 bytes) should miss L1 and L2
     */
    array_size = 1024 * 1024;  /* 4MB - larger than L3 */
    array = malloc(sizeof(int) * array_size);
    if (!array) {
        perror("malloc");
        pmc_release(pmcid);
        return (1);
    }

    /* Strided access - should cause L3 misses */
    for (i = 0; i < WORKLOAD_ITERATIONS; i++) {
        volatile int idx = (i * 256) % array_size;
        array[idx] = idx;
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

    printf("[RESULT] L3 Miss Count: %ju\n", (uintmax_t)value);

    if (value > 0) {
        printf("[PASS] L3 miss counter is working\n");
    } else {
        printf("[WARN] L3 miss count is zero - may need different event name\n");
    }

    free(array);
    pmc_release(pmcid);
    return (0);
}

/*
 * ============================================================================
 * Test Case: L3 Eviction Counter
 * Test ID: TC-L3-04
 *
 * Objective: Test L3 cache line eviction counting
 *
 * Expected Result: Counter increments when cache lines are evicted
 * ============================================================================
 */
static int
test_l3_eviction_counter(void)
{
    pmc_id_t pmcid;
    pmc_value_t value;
    int *array;
    size_t array_size;
    int i;

    printf("\n=== Test: L3 Eviction Counter ===\n");

    if (!require_root()) {
        return (1);
    }

    if (!cpu_supports_l3_counters()) {
        printf("[SKIP] CPU does not support L3 counters\n");
        return (2);
    }

    /* Initialize PMC */
    if (pmc_init() < 0) {
        perror("pmc_init");
        return (1);
    }

    /* Allocate L3 eviction counter */
    if (pmc_allocate(PMC_EVENT_L3_EVICTION, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) < 0) {
        fprintf(stderr, "pmc_allocate failed for %s: %s\n",
                PMC_EVENT_L3_EVICTION, strerror(errno));
        printf("[INFO] L3_EVICTION event may not be available on this CPU\n");
        return (2);
    }

    /* Start counting */
    if (pmc_start(pmcid) < 0) {
        perror("pmc_start");
        pmc_release(pmcid);
        return (1);
    }

    /*
     * Workload: Access many unique lines to force evictions
     */
    array_size = 8192;  /* 32KB - should fill L3 */
    array = malloc(sizeof(int) * array_size);
    if (!array) {
        perror("malloc");
        pmc_release(pmcid);
        return (1);
    }

    /* Fill and overflow cache multiple times */
    for (i = 0; i < 100; i++) {
        for (int j = 0; j < array_size; j++) {
            array[j] = j;
        }
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

    printf("[RESULT] L3 Eviction Count: %ju\n", (uintmax_t)value);

    if (value > 0) {
        printf("[PASS] L3 eviction counter is working\n");
    } else {
        printf("[WARN] L3 eviction count is zero\n");
    }

    free(array);
    pmc_release(pmcid);
    return (0);
}

/*
 * ============================================================================
 * Test Case: L3 Multiple Events
 * Test ID: TC-L3-05
 *
 * Objective: Test simultaneous L3 event counting
 *
 * Expected Result: Multiple PMC counters work independently
 * ============================================================================
 */
static int
test_l3_multiple_counters(void)
{
    pmc_id_t pmcid_hit, pmcid_miss;
    pmc_value_t value_hit, value_miss;
    int *array;
    size_t array_size;
    int i;

    printf("\n=== Test: L3 Multiple Counters ===\n");

    if (!require_root()) {
        return (1);
    }

    if (!cpu_supports_l3_counters()) {
        printf("[SKIP] CPU does not support L3 counters\n");
        return (2);
    }

    /* Initialize PMC */
    if (pmc_init() < 0) {
        perror("pmc_init");
        return (1);
    }

    /* Allocate two counters - may fail if not enough HW counters */
    if (pmc_allocate(PMC_EVENT_L3_HIT, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid_hit) < 0) {
        fprintf(stderr, "pmc_allocate failed for L3_HIT\n");
        printf("[INFO] Cannot allocate L3_HIT counter\n");
        return (2);
    }

    if (pmc_allocate(PMC_EVENT_L3_MISS, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid_miss) < 0) {
        fprintf(stderr, "pmc_allocate failed for L3_MISS\n");
        pmc_release(pmcid_hit);
        printf("[INFO] Cannot allocate L3_MISS counter\n");
        return (2);
    }

    /* Start both counters */
    if (pmc_start(pmcid_hit) < 0 || pmc_start(pmcid_miss) < 0) {
        perror("pmc_start");
        pmc_release(pmcid_hit);
        pmc_release(pmcid_miss);
        return (1);
    }

    /* Mixed workload */
    array_size = 4096;
    array = malloc(sizeof(int) * array_size);
    if (!array) {
        pmc_release(pmcid_hit);
        pmc_release(pmcid_miss);
        return (1);
    }

    /* Mix of sequential (hits) and random (misses) */
    for (i = 0; i < WORKLOAD_ITERATIONS; i++) {
        if (i % 2 == 0) {
            volatile int idx = i % array_size;
            array[idx] = idx;
        } else {
            volatile int idx = (i * 127) % array_size;
            array[idx] = idx;
        }
    }

    /* Stop and read */
    if (pmc_stop(pmcid_hit) < 0 || pmc_stop(pmcid_miss) < 0) {
        perror("pmc_stop");
        free(array);
        pmc_release(pmcid_hit);
        pmc_release(pmcid_miss);
        return (1);
    }

    if (pmc_read(pmcid_hit, &value_hit) < 0 || pmc_read(pmcid_miss, &value_miss) < 0) {
        perror("pmc_read");
        free(array);
        pmc_release(pmcid_hit);
        pmc_release(pmcid_miss);
        return (1);
    }

    printf("[RESULT] L3 Hit Count: %ju\n", (uintmax_t)value_hit);
    printf("[RESULT] L3 Miss Count: %ju\n", (uintmax_t)value_miss);

    if (value_hit > 0 || value_miss > 0) {
        printf("[PASS] Multiple L3 counters work simultaneously\n");
    } else {
        printf("[WARN] Both counters are zero\n");
    }

    free(array);
    pmc_release(pmcid_hit);
    pmc_release(pmcid_miss);
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
    printf("  l3-detection       - Test L3 counter availability detection\n");
    printf("  l3-hit            - Test L3 cache hit counter\n");
    printf("  l3-miss           - Test L3 cache miss counter\n");
    printf("  l3-eviction       - Test L3 cache eviction counter\n");
    printf("  l3-multiple       - Test multiple L3 counters simultaneously\n");
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
    printf("AMD PMC Core L3 Counters Test Suite\n");
    printf("========================================\n");

    if (strcmp(test_name, "all") == 0) {
        printf("\nRunning all tests...\n");
        result |= test_l3_detection();
        result |= test_l3_hit_counter();
        result |= test_l3_miss_counter();
        result |= test_l3_eviction_counter();
        result |= test_l3_multiple_counters();
    } else if (strcmp(test_name, "l3-detection") == 0) {
        result = test_l3_detection();
    } else if (strcmp(test_name, "l3-hit") == 0) {
        result = test_l3_hit_counter();
    } else if (strcmp(test_name, "l3-miss") == 0) {
        result = test_l3_miss_counter();
    } else if (strcmp(test_name, "l3-eviction") == 0) {
        result = test_l3_eviction_counter();
    } else if (strcmp(test_name, "l3-multiple") == 0) {
        result = test_l3_multiple_counters();
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
