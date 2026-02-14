<!--
Copyright (c) 2026, Mauro Risonho de Paula Assumpção
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Author: Mauro Risonho de Paula Assumpção
Date Created: 2026-02-14
Last Modified: 2026-02-14
-->

# Proof of Concept: Performance Monitoring & Hardware Interaction

This PoC demonstrates the implementation of low-level performance monitoring and hardware abstraction layers within the FreeBSD operating system, specifically targeting AMD-based systems.

## 1. Environment Simulation (AMD Zen)

To simulate the target hardware environment (AMD EPYC/Zen), the laboratory environment must be initialized with the following orchestrator:

```bash
python3 scripts/qemu_setup.py --amd
```

_Note: The orchestrator targets the `EPYC-v4` CPU model to expose advanced performance counters to the guest VM._

## 2. Performance Monitoring Counter (PMC) Demonstration

The PMC PoC provides a mechanism to sample architectural events directly from userspace using `libpmc`.

### 2.1 Deployment Steps

1. **Kernel Initialization:**
   Load the hardware counters kernel module:

   ```bash
   kldload hwpmc
   ```

2. **Source Compilation:**
   Execute the build process for the PoC utility:

   ```bash
   cd poc/
   make
   ```

3. **Execution:**
   Monitor retired instructions:
   ```bash
   ./pmc_demo instructions-retired
   ```

## 3. Automotive Hardware Monitoring (HWMON) Skeleton

The HWMON PoC illustrates the integration of a specialized monitoring driver into the FreeBSD `newbus` architecture.

### 3.1 Driver Deployment

1. **Kernel Module Load:**

   ```bash
   kldload ./hwmon_skeleton.ko
   ```

2. **Verification:**
   Examine the kernel message buffer to confirm successful attachment:
   ```bash
   dmesg | grep PoC
   ```

## 4. Advanced Kernel Debugging

For deep-dive analysis of PMC internal states and kernel structures, use the provided `kgdb` macros:

```bash
kgdb kernel.debug -x findings/kgdb_pmc.gdb
```

Available command: `show_pmcs` (inspects the `hwpmc` state machine).
