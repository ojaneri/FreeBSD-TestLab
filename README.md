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

# FreeBSD Engineering Lab & Research (Enterprise Edition)

This laboratory is established for kernel development, low-level debugging, and system research on **FreeBSD 14.3-RELEASE x64**.

## Architecture & Components

The project is structured for modularity and enterprise-grade research:

### 1. Technical Documentation (`docs/`)

- **`SPECIFICATION.md`**: Architectural goals for the PMC (Performance Monitoring Counters) and HWMON (Hardware Monitoring) subsystems.
- **`README.poc.md`**: Step-by-step deployment and verification guide for the demonstration components.

### 2. Orchestration Suite (`scripts/`)

- **`qemu_setup.py`**: Main lab orchestrator for launching the FreeBSD environment.
- **`automate_boot.py`**: Robust automation script for initial system provisioning and serial console configuration.
- **`fast_download.py`**: Visual downloader (`tqdm` integrated) for fetching the 14.3-RELEASE VM image.
- **`utils.py` / `config.py`**: Centralized utility logic and project-wide configuration.

### 3. Proof of Concept Implementation (`poc/`)

- **`pmc_demo.c`**: High-performance userspace utility for hardware event sampling.
- **`hwmon_skeleton.c`**: Reference kernel driver utilizing the FreeBSD `newbus` architecture.
- **`Makefile`**: Standardized BSD build infrastructure for POC components.

---

## Engineering Workflow

### 1. Environment Setup

Initialize the Python virtual environment and verify system dependencies:

```bash
python3 setup_env.py
source .venv/bin/activate
```

### 2. Image Preparation

Download and extract the FreeBSD 14.3-RELEASE production image:

```bash
python3 scripts/fast_download.py
```

### 3. Lab Orchestration

Launch the engineering lab:

```bash
python3 scripts/qemu_setup.py
```

_Note: The system starts **PAUSED** to allow debugger attachment. To resume boot, open another terminal and run:_

```bash
# Using the GDB batch unpause command
gdb -batch -ex "target remote :1234" -ex "continue" -ex "detach"
```

### 4. Diagnostic Toolkit

The `findings/` directory contains diagnostic artifacts, including `gdb` macros and kernel configuration templates for advanced research.
