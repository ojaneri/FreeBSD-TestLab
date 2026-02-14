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
Date Created: 2026-02-13
Last Modified: 2026-02-13
-->

# FreeBSD Engineering Lab & Research

This laboratory was established for kernel development, low-level debugging, and system research on FreeBSD 15.

### 1. Project Specifications & Goals (`docs/SPECIFICATION.md`)

Detailed technical requirements and architectural goals for the PMC and HWMON subsystems.

### 2. Proof of Concept Implementation (`docs/README.poc.md`)

Step-by-step guide for deploying and verifying the PMC and HWMON components.

### 3. Environment Orchestration (`scripts/qemu_setup.py`)

### 2. Kernel Debugging Toolkit (.gdbinit)

I developed custom macros to streamline the inspection of internal states:

- `curthread`: Quick identification of the active thread and priority.
- `list_procs`: Scanning of the `allproc` list for memory mapping and PIDs.
- `show_pcb`: Direct inspection of the hardware context.

### 3. Diagnostic Kernel Configuration (`DEBUG15`)

A custom `KERNCONF` based on FreeBSD 15 `GENERIC`, optimized with:

- **`WITNESS`**: Essential for detecting lock order reversals in complex drivers.
- **`INVARIANTS`**: Activation of sanity assertions at runtime.
- **`KTR`**: Ring buffer for tracing asynchronous events.

## Engineering Workflow

1. **Kernel Build:**

   ```bash
   cp DEBUG15 /usr/src/sys/amd64/conf/
   make buildkernel KERNCONF=DEBUG15
   make installkernel KERNCONF=DEBUG15
   ```

2. **Image Preparation:**
   The lab requires a FreeBSD disk image (default: `workloads/freebsd_debug.qcow2`).
   Use the fast download script to get the 15.0-STABLE version automatically:

   ```bash
   python3 scripts/fast_download.py
   ```

3. **Lab Launch:**

   ```bash
   python3 scripts/qemu_setup.py
   ```

   _Note: The script now runs in text mode (headless). To exit QEMU, press `Ctrl+A` and then `x`._

   **IMPORTANT:** The system will start **PAUSED**. To continue the boot:
   1. Open another terminal in the same folder.
   2. Run: `gdb -ex "target remote :1234" -ex "continue"`
   3. The boot process will proceed in the original terminal where the script was run.

4. **Debugging:**
   On host: `gdb kernel.debug -x .gdbinit`
