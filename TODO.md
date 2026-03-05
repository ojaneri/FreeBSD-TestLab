# TODO - AMD IBS Feature Testing Roadmap

## Overview

This document organizes the tasks to implement the AMD IBS (Instruction-Based Sampling) test plan on FreeBSD, following the document **AMD IBS Feature Testing - FreeBSD Kernel**.

---

## Sprint 1: Base Infrastructure
**Estimated duration:** 2-3 weeks

### 1.1 Directory Structure
- [ ] Create directory `tests/sys/amd/ibs/`
- [ ] Create base Makefile for tests
- [ ] Create `ibs_utils.h` with shared helpers

### 1.2 Build Configuration
- [ ] Configure kernel `AMD_IBS` at `sys/amd64/conf/AMD_IBS`
- [ ] Enable options `HWPMC_HOOKS`, `device hwpmc`, `HWPMC_AMD_IBS`
- [ ] Build kernel with IBS support

### 1.3 Test Environment
- [ ] Configure QEMU/KVM with AMD CPU (Family 10h+)
- [ ] Install dependencies: `kyua`, `atf-c`, `pmcstat`
- [ ] Validate hwpmc loads correctly

---

## Sprint 2: Hardware Detection Tests (TC-DET)
**Estimated duration:** 1 week

### 2.1 Create `ibs_detect_test.c`
- [ ] **TC-DET-01**: Verify dmesg shows IBS detected
- [ ] **TC-DET-02**: Test fallback on non-IBS CPU
- [ ] **TC-DET-03**: Verify sysctl `hw.pmc.ibs_*`
- [ ] **TC-DET-04**: Validate CPUID Fn8000_001B

---

## Sprint 3: MSR Control Tests (TC-MSR)
**Estimated duration:** 1 week

### 3.1 Create `ibs_msr_test.c`
- [ ] **TC-MSR-01**: Enable IBS fetch - verify IBS_FETCH_CTL
- [ ] **TC-MSR-02**: Disable IBS fetch
- [ ] **TC-MSR-03**: Enable IBS op - verify IBS_OP_CTL
- [ ] **TC-MSR-04**: Enable/disable cycle 100x (stress)
- [ ] **TC-MSR-05**: Test minimum/maximum interval values

---

## Sprint 4: Interrupt Tests (TC-INT)
**Estimated duration:** 1 week

### 4.1 Create `ibs_interrupt_test.c`
- [ ] **TC-INT-01**: Verify samples received with workload
- [ ] **TC-INT-02**: Verify NMI does not conflict with other NMIs
- [ ] **TC-INT-03**: IBS with kernel watchdog active
- [ ] **TC-INT-04**: Buffer overflow handling
- [ ] **TC-INT-05**: High interrupt rate stress test (60s)

---

## Sprint 5: Data Accuracy Tests (TC-DATA)
**Estimated duration:** 1 week

### 5.1 Create `ibs_data_accuracy_test.c`
- [ ] **TC-DATA-01**: PC in known loop
- [ ] **TC-DATA-02**: Data address in known loads
- [ ] **TC-DATA-03**: L1 cache miss flag
- [ ] **TC-DATA-04**: TLB miss flag
- [ ] **TC-DATA-05**: Branch flags and target address

---

## Sprint 6: SMP Tests (TC-SMP)
**Estimated duration:** 1 week

### 6.1 Create `ibs_smp_test.c`
- [ ] **TC-SMP-01**: IBS on all CPUs simultaneously
- [ ] **TC-SMP-02**: IBS with CPU affinity
- [ ] **TC-SMP-03**: CPU hotplug with active IBS
- [ ] **TC-SMP-04**: Independent per-CPU counters

---

## Sprint 7: Userspace API Tests (TC-API)
**Estimated duration:** 1 week

### 7.1 Create `ibs_api_test.c`
- [ ] **TC-API-01**: pmcstat with IBS event
- [ ] **TC-API-02**: Two processes open IBS simultaneously
- [ ] **TC-API-03**: Unprivileged user tries to open IBS (EPERM)
- [ ] **TC-API-04**: ioctl PMC_OP_CONFIGURELOG with IBS

---

## Sprint 8: Stability Tests (TC-STR)
**Estimated duration:** 1-2 weeks

### 8.1 Create `ibs_stress_test.c`
- [ ] **TC-STR-01**: IBS + memstress + cpustress (1 hour)
- [ ] **TC-STR-02**: Rapid open/close (1000 iterations)
- [ ] **TC-STR-03**: IBS in VM (graceful degradation)
- [ ] **TC-STR-04**: OOM during active IBS session

---

## Sprint 9: Jenkins CI/CD Integration
**Estimated duration:** 1 week

### 9.1 Jenkins Pipeline
- [ ] Create Jenkinsfile at repository root
- [ ] Configure agent `freebsd-amd-ibs`
- [ ] Configure stages: Checkout, Build, Install, Test, Report
- [ ] Configure post-build: JUnit, artifacts, email

### 9.2 Reports
- [ ] Generate JUnit XML
- [ ] Generate HTML report via kyua
- [ ] Configure metrics: pass rate, flaky tests

---

## Sprint 10: Execution and Validation
**Estimated duration:** Continuous

### 10.1 Execution Schedule

| Trigger | Tests | Frequency |
|---------|-------|------------|
| PR/Patch | TC-DET, TC-MSR, TC-INT (smoke) | Every PR |
| Daily | All except TC-STR | Daily |
| Weekly | Full suite + TC-STR | Weekly |
| Pre-release | Full suite x3 | Before release |

---

## Current Status

| Sprint | Status | Notes |
|--------|--------|-------|
| Sprint 1 | 🔴 Pending | Base infrastructure |
| Sprint 2 | 🔴 Pending | Hardware detection |
| Sprint 3 | 🔴 Pending | MSR Control |
| Sprint 4 | 🔴 Pending | Interrupts |
| Sprint 5 | 🔴 Pending | Data accuracy |
| Sprint 6 | 🔴 Pending | SMP |
| Sprint 7 | 🔴 Pending | Userspace API |
| Sprint 8 | 🔴 Pending | Stability |
| Sprint 9 | 🔴 Pending | Jenkins CI/CD |
| Sprint 10 | 🔴 Pending | Execution |

---

## References

- [AMD IBS Feature Testing Document](./docs/AMD_IBS_Feature_Testing.md)
- FreeBSD hwpmc(4): https://man.freebsd.org/hwpmc
- AMD APM Volume 2: Instruction-Based Sampling
- FreeBSD ATF: https://github.com/jmmv/atf
- Kyua: https://github.com/jmmv/kyua
