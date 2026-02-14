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

/**
 * @file pmc_demo.c
 * @brief Demonstration utility for architectural event sampling using libpmc.
 *
 * This utility allocates and starts a performance monitoring counter for a 
 * specified architectural event (default: instructions-retired). It executes 
 * a controlled workload and reports the raw counter value.
 */

int main(int argc, char **argv) {
    pmc_value_t value;
    pmc_id_t pmcid;
    const char *event = "instructions-retired";
    
    if (argc > 1) event = argv[1];

    if (pmc_init() < 0)
        err(1, "pmc_init failed (check if hwpmc.ko is loaded)");

    printf("[INFO] Monitoring hardware event: %s\n", event);

    /* Allocate counter in Thread-private Counting mode */
    if (pmc_allocate(event, PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid) < 0)
        err(1, "pmc_allocate failed");

    if (pmc_start(pmcid) < 0)
        err(1, "pmc_start failed");

    /* Simulated Computational Workload */
    printf("[INFO] Executing workload...\n");
    for (volatile int i = 0; i < 1000000; i++);

    if (pmc_stop(pmcid) < 0)
        err(1, "pmc_stop failed");

    if (pmc_read(pmcid, &value) < 0)
        err(1, "pmc_read failed");

    printf("[DATA] Resulting Counter Value: %ju\n", (uintmax_t)value);

    pmc_release(pmcid);
    return 0;
}
