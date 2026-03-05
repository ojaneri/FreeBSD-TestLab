/*
 * Copyright (c) 2026, Mauro Risonho de Paula Assumpção
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
 * Author: Mauro Risonho de Paula Assumpção
 * Date Created: 2026-02-14
 * Last Modified: 2026-02-14
 *
 * Project: FreeBSD Performance Monitoring Counter (PMC) Extension
 * Component: Userspace Diagnostic Utility (pmc_demo)
 */

#include <sys/types.h>
#include <sys/pmc.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <libpmc.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

/* Workload function pointers */
typedef void (*workload_fn)(void);

/**
 * Workload 1: Cache Miss (Memory Access Pattern)
 * Triggers cache misses by accessing memory with large stride
 */
static void workload_cache_miss(void) {
    #define CACHE_SIZE 10000000
    int *array = malloc(sizeof(int) * CACHE_SIZE);
    if (!array) return;
    
    printf("[WORKLOAD] Running Cache Miss simulation (stride=16)...\n");
    for (int i = 0; i < CACHE_SIZE; i += 16) {
        array[i] = i;
    }
    free(array);
}

/**
 * Workload 2: Branch Prediction Stress
 * Uses random branches to stress the branch predictor
 */
static void workload_branch_predictor(void) {
    printf("[WORKLOAD] Running Branch Prediction stress test...\n");
    srand(time(NULL));
    volatile long long sum = 0;
    
    for (int i = 0; i < 1000000; i++) {
        if (rand() > RAND_MAX / 2)
            sum += i;
        else
            sum -= i;
    }
}

/**
 * Workload 3: SIMD/Vector Operations
 * Tests floating-point SIMD operations
 */
static void workload_simd(void) {
    #define SIMD_SIZE 10000
    float *a = malloc(sizeof(float) * SIMD_SIZE);
    float *b = malloc(sizeof(float) * SIMD_SIZE);
    if (!a || !b) return;
    
    printf("[WORKLOAD] Running SIMD/Vector operations...\n");
    
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < SIMD_SIZE; i++) {
            a[i] = a[i] * b[i] + a[i];  // FMA-like operation
        }
    }
    free(a);
    free(b);
}

/**
 * Workload 4: Syscall Overhead
 * Tests syscall overhead with repeated getpid() calls
 */
static void workload_syscall(void) {
    printf("[WORKLOAD] Running Syscall overhead test (getpid)...\n");
    for (int i = 0; i < 100000; i++) {
        getpid();
    }
}

/**
 * Workload 5: CPU Intensive (Original loop)
 * Basic CPU burn loop
 */
static void workload_cpu_intensive(void) {
    printf("[WORKLOAD] Running CPU Intensive loop...\n");
    for (volatile int i = 0; i < 1000000; i++);
}

/* Workload registry */
static struct {
    const char *name;
    workload_fn fn;
    const char *recommended_event;
} workloads[] = {
    {"cpu",         workload_cpu_intensive,      "instructions-retired"},
    {"cache-miss",  workload_cache_miss,         "cache-misses"},
    {"branch",      workload_branch_predictor,   "branch-misses"},
    {"simd",       workload_simd,                "simd-instructions"},
    {"syscall",    workload_syscall,            "syscall"},
    {NULL, NULL, NULL}
};

/**
 * @file pmc_demo.c
 * @brief Demonstration utility for architectural event sampling using libpmc.
 *
 * This utility allocates and starts a performance monitoring counter for a
 * specified architectural event (default: instructions-retired). It executes
 * a controlled workload and reports the raw counter value.
 *
 * Usage: ./pmc_demo [workload-name [pmc-event]]
 * 
 * Available workloads:
 *   cpu         - CPU intensive loop (default)
 *   cache-miss - Memory access pattern causing cache misses
 *   branch     - Branch prediction stress test
 *   simd       - SIMD/Vector floating-point operations
 *   syscall    - Syscall overhead test (getpid)
 */

void print_usage(const char *prog) {
    printf("Usage: %s [workload-name [pmc-event]]\n\n", prog);
    printf("Available workloads:\n");
    printf("  cpu         - CPU intensive loop (default)\n");
    printf("  cache-miss - Memory access pattern causing cache misses\n");
    printf("  branch     - Branch prediction stress test\n");
    printf("  simd       - SIMD/Vector floating-point operations\n");
    printf("  syscall    - Syscall overhead test (getpid)\n");
    printf("\nExample: %s branch branch-misses\n", prog);
}

int main(int argc, char **argv) {
    pmc_value_t value;
    pmc_id_t pmcid;
    const char *event = "instructions-retired";
    const char *workload_name = "cpu";
    
    /* Parse arguments */
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        workload_name = argv[1];
    }
    if (argc > 2) event = argv[2];

    /* Find and validate workload */
    workload_fn selected_workload = NULL;
    const char *recommended_event = NULL;
    
    for (int i = 0; workloads[i].name != NULL; i++) {
        if (strcmp(workloads[i].name, workload_name) == 0) {
            selected_workload = workloads[i].fn;
            recommended_event = workloads[i].recommended_event;
            break;
        }
    }
    
    if (!selected_workload) {
        fprintf(stderr, "[ERROR] Unknown workload: %s\n", workload_name);
        print_usage(argv[0]);
        return 1;
    }

    if (pmc_init() < 0)
        err(1, "[ERROR] pmc_init failed: Ensure hwpmc(4) is loaded via 'kldload hwpmc'");

    printf("[INFO] Workload: %s\n", workload_name);
    printf("[INFO] Monitoring PMC event: %s\n", event);
    if (recommended_event && strcmp(event, "instructions-retired") == 0) {
        printf("[HINT] Recommended event for this workload: %s\n", recommended_event);
    }

    /* Allocate counter in Thread-private Counting mode (TC) */
    if (pmc_allocate(event, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) < 0)
        err(1, "[ERROR] pmc_allocate failed: check if the event is supported by the hardware");

    if (pmc_start(pmcid) < 0)
        err(1, "pmc_start failed");

    /* Execute selected workload */
    selected_workload();

    if (pmc_stop(pmcid) < 0)
        err(1, "pmc_stop failed");

    if (pmc_read(pmcid, &value) < 0)
        err(1, "pmc_read failed");

    printf("[DATA] Resulting Counter Value: %ju\n", (uintmax_t)value);

    pmc_release(pmcid);
    return 0;
}
