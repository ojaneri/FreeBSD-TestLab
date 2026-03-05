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
 * Component: Shared utilities for PMC testing
 */

#ifndef _AMD_PMC_TEST_UTILS_H
#define _AMD_PMC_TEST_UTILS_H

#include <sys/param.h>
#include <sys/cpuctl.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <machine/specialreg.h>
#include <machine/cpufunc.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * ============================================================================
 * MSR Addresses for AMD Processors
 * Reference: AMD64 Architecture Programmer's Manual, Volume 2
 * ============================================================================
 */

/* IBS Fetch MSRs */
#define MSR_IBS_FETCH_CTL       0xc0011030
#define MSR_IBS_FETCH_LINAD     0xc0011031
#define MSR_IBS_FETCH_PHYSAD    0xc0011032
#define MSR_IBS_FETCH_STS       0xc0011033

/* IBS Op MSRs */
#define MSR_IBS_OP_CTL          0xc0011034
#define MSR_IBS_OP_LINAD        0xc0011035
#define MSR_IBS_OP_DATA         0xc0011036
#define MSR_IBS_OP_DATA2        0xc0011037
#define MSR_IBS_OP_DATA3        0xc0011038
#define MSR_IBS_OP_STS          0xc0011039

/* L3 Cache MSRs (Family 17h+) */
#define MSR_L3_CNTL             0xc001102c
#define MSR_L3_CNTL2            0xc001102d

/* Data Fabric Counters (Zen+ uncore) */
#define MSR_DF_CTR0             0xc0011020
#define MSR_DF_CTR1             0xc0011021
#define MSR_DF_CTR2             0xc0011022
#define MSR_DF_CTR3             0xc0011023
#define MSR_DF_CFG0             0xc0011024
#define MSR_DF_CFG1             0xc0011025

/*
 * ============================================================================
 * IBS Control Bits
 * ============================================================================
 */

/* IBS Fetch Control */
#define IBS_FETCH_EN            0x00000001
#define IBS_FETCH_CNT           0x00000002
#define IBS_FETCH_VAL           0x00000004
#define IBS_FETCH_COMP          0x00000008

/* IBS Op Control */
#define IBS_OP_EN               0x00000001
#define IBS_OP_CNT              0x00000002
#define IBS_OP_VAL              0x00000004
#define IBS_OP_COMP             0x00000008

/*
 * ============================================================================
 * CPU Family Detection
 * ============================================================================
 */

/**
 * Get CPU family from sysctl
 * @return CPU family (e.g., 0x17 for Zen, 0x19 for Zen3/Zen4)
 */
static inline int
get_cpu_family(void)
{
    int family;
    size_t len = sizeof(family);

    sysctlbyname("hw.cpu_family", &family, &len, NULL, 0);
    return (family);
}

/**
 * Get CPU model from sysctl
 * @return CPU model
 */
static inline int
get_cpu_model(void)
{
    int model;
    size_t len = sizeof(model);

    sysctlbyname("hw.model", &model, &len, NULL, 0);
    return (model);
}

/**
 * Check if CPU supports IBS (Instruction Based Sampling)
 * @return 1 if supported, 0 otherwise
 */
static inline int
cpu_supports_ibs(void)
{
    uint32_t ebx, ecx, edx;

    /* CPUID Fn8000_001B - IBS Features */
    do_cpuid(0x8000001b, ebx, ecx, edx);

    /* Bit 0: IBS fetch sample store
       Bit 1: IBS op sample store
       Bit 2: IBS branch target
    */
    return ((ebx & 0x7) != 0);
}

/**
 * Check if CPU supports L3 cache counters (Family 17h+)
 * @return 1 if supported, 0 otherwise
 */
static inline int
cpu_supports_l3_counters(void)
{
    int family = get_cpu_family();

    /* L3 counters available starting from Family 17h (Zen) */
    return (family >= 0x17);
}

/**
 * Check if CPU supports Data Fabric counters (Zen+)
 * @return 1 if supported, 0 otherwise
 */
static inline int
cpu_supports_df_counters(void)
{
    int family = get_cpu_family();

    /* DF counters available starting from Family 15h (Bulldozer) */
    return (family >= 0x15);
}

/**
 * Check if CPU is Zen4 or Zen5 (Family 19h)
 * @return 1 if Zen4/Zen5, 0 otherwise
 */
static inline int
cpu_is_zen4_or_later(void)
{
    int family = get_cpu_family();

    return (family >= 0x19);
}

/**
 * Check if CPU is Zen5 (Family 19h, model >= 0x20)
 * @return 1 if Zen5, 0 otherwise
 */
static inline int
cpu_is_zen5(void)
{
    int family = get_cpu_family();
    int model = get_cpu_model();

    /* Zen5 is Family 19h, model 0x20-0x2F */
    return (family == 0x19 && model >= 0x20);
}

/*
 * ============================================================================
 * MSR Access Functions
 * ============================================================================
 */

/**
 * Read a Model-Specific Register (MSR)
 * @param msr MSR address to read
 * @return 64-bit MSR value
 */
static inline uint64_t
read_msr(uint32_t msr)
{
    cpuctl_msr_args_t args;
    int fd;
    uint64_t value;

    fd = open("/dev/cpuctl0", O_RDWR);
    if (fd < 0) {
        perror("open /dev/cpuctl0");
        return (0);
    }

    args.msr = msr;
    if (ioctl(fd, CPUCTL_RDMSR, &args) < 0) {
        perror("ioctl CPUCTL_RDMSR");
        close(fd);
        return (0);
    }

    value = args.data;
    close(fd);

    return (value);
}

/**
 * Write a Model-Specific Register (MSR)
 * @param msr MSR address to write
 * @param value 64-bit value to write
 * @return 0 on success, -1 on error
 */
static inline int
write_msr(uint32_t msr, uint64_t value)
{
    cpuctl_msr_args_t args;
    int fd;

    fd = open("/dev/cpuctl0", O_RDWR);
    if (fd < 0) {
        perror("open /dev/cpuctl0");
        return (-1);
    }

    args.msr = msr;
    args.data = value;
    if (ioctl(fd, CPUCTL_WRMSR, &args) < 0) {
        perror("ioctl CPUCTL_WRMSR");
        close(fd);
        return (-1);
    }

    close(fd);
    return (0);
}

/*
 * ============================================================================
 * Utility Functions
 * ============================================================================
 */

/**
 * Print CPU information for debugging
 */
static inline void
print_cpu_info(void)
{
    char model[256];
    size_t len = sizeof(model);

    sysctlbyname("hw.model", model, &len, NULL, 0);

    printf("=== CPU Information ===\n");
    printf("Model: %s\n", model);
    printf("Family: 0x%x\n", get_cpu_family());
    printf("Model: 0x%x\n", get_cpu_model());
    printf("IBS Supported: %s\n", cpu_supports_ibs() ? "Yes" : "No");
    printf("L3 Counters: %s\n", cpu_supports_l3_counters() ? "Yes" : "No");
    printf("DF Counters: %s\n", cpu_supports_df_counters() ? "Yes" : "No");
    printf("Zen4+: %s\n", cpu_is_zen4_or_later() ? "Yes" : "No");
    printf("=======================\n");
}

/**
 * Check if running as root
 * @return 1 if root, 0 otherwise
 */
static inline int
require_root(void)
{
    if (getuid() != 0) {
        fprintf(stderr, "ERROR: This test must be run as root\n");
        return (0);
    }
    return (1);
}

#endif /* _AMD_PMC_TEST_UTILS_H */
