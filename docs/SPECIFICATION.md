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
Last Modified: 2026-02-14
-->

# Technical Specification: FreeBSD PMC & Automotive HWMON Extension

This document outlines the technical requirements and architectural goals for the FreeBSD Performance Monitoring Counter (PMC) extension and the specialized Hardware Monitoring (HWMON) subsystem.

## 1. Project Scope

The project focuses on the development and optimization of the FreeBSD Performance Monitoring framework, specifically targeting the **AMD Zen** architecture and automotive-grade hardware interaction.

- **Primary Components:** `hwpmc` kernel module, `libpmc` userspace library, and the `pmcstat` diagnostic tool.
- **Microarchitecture Focus:** AMD Zen 3/4 Instruction Based Sampling (IBS) and standard architecture events.
- **Target Sector:** Automotive Embedded Systems (C-based).

## 2. Technical Requirement Matrix

| Requirement              | Component                    | Implementation Detail                                                                                            |
| :----------------------- | :--------------------------- | :--------------------------------------------------------------------------------------------------------------- |
| **PMC Extension**        | `sys/dev/hwpmc`              | Implementation of architecture-specific performance event handlers for the AMD PMU.                              |
| **Latency Analysis**     | `pmcstat` / `libpmc`         | Development of sampling mechanisms to identify critical paths and "hot loops" in real-time automotive workloads. |
| **Kernel Observability** | `kgdb` / `DTrace`            | Deep integration for tracing stack corruption and memory dump analysis in AMD x86_64 environments.               |
| **AMD Zen Optimization** | x86_64 PMU Driver            | Leveraging AMD-exclusive IBS counters to provide higher fidelity sampling than traditional LBR methods.          |
| **HWMON System**         | `newbus` Driver Architecture | Registration of automotive-grade thermal and voltage monitoring drivers within the FreeBSD driver hierarchy.     |

## 3. Core Competency Areas

### 3.1 x86 Microarchitecture

Deep understanding of the execution pipeline, including **Instruction Retirement**, **Branch Prediction Unit (BPU)**, and how PMC events correlate with pipeline stalls.

### 3.2 FreeBSD Kernel Internals

- **Driver Model:** Mastery of the `newbus` hierarchy and resource allocation (`rman`).
- **Synchronization:** Expert use of `mutexes`, `rwlocks`, and `sxlocks` in SMP environments.
- **CPU Affinity:** Precise control over task placement and interrupt steering.

## 4. Automotive Vertical Requirements

High-performance FreeBSD deployments in the automotive sector demand:

- **Predictable Latency:** Optimization of the network stack (e.g., `netmap`) and interrupt handling.
- **Resilient Data Layers:** Advanced ZFS configurations for power-loss protection and silent data corruption detection.
- **Standard Compliance Path:** Implementation following rigorous safety-critical coding standards (MISRA C principles where applicable to the kernel).

---

_Document maintained in docs/SPECIFICATION.md_
