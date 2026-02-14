#!/usr/bin/env python3
#
# Copyright (c) 2026, Mauro Risonho de Paula Assumpção
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Author: Mauro Risonho de Paula Assumpção
# Date Created: 2026-02-14
# Last Modified: 2026-02-14
#

import subprocess
import sys
import os
import argparse
import config
import utils

def run_qemu(amd_mode=False):
    utils.kill_existing_qemu()
    
    if not utils.check_image_exists():
        sys.exit(1)

    # Base configuration
    memory = config.AMD_MEMORY if amd_mode else config.DEFAULT_MEMORY
    cores = config.AMD_CPU_CORES if amd_mode else config.DEFAULT_CPU_CORES
    ssh_port = config.AMD_SSH_PORT if amd_mode else config.DEFAULT_SSH_PORT
    gdb_port = config.AMD_GDB_PORT if amd_mode else config.DEFAULT_GDB_PORT
    cpu_model = config.AMD_CPU_MODEL if amd_mode else "host"

    print(f"[+] Launching FreeBSD Lab {'(AMD Zen Mode)' if amd_mode else ''}")
    print(f"[+] Resources: {cores} Cores, {memory} RAM")
    print(f"[+] Connectivity: SSH:{ssh_port}, GDB:{gdb_port}")

    cmd = [
        "qemu-system-x86_64",
        "-m", memory,
        "-smp", cores,
        "-cpu", cpu_model,
        "-enable-kvm",
        "-hda", config.IMAGE_PATH,
        "-netdev", f"user,id=net0,hostfwd=tcp::{ssh_port}-:22",
        "-device", "e1000,netdev=net0",
        "-nographic",
        "-s", "-S",
        "-serial", "mon:stdio"
    ]

    print("--- Launching QEMU ---")
    try:
        subprocess.run(cmd)
    except KeyboardInterrupt:
        print("\n[!] QEMU interrupted by user.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="FreeBSD QEMU Setup Orchestrator")
    parser.add_argument("--amd", action="store_true", help="Enable AMD Zen simulation mode")
    args = parser.parse_args()

    print("--- FreeBSD Engineering Lab: QEMU Orchestrator ---")
    run_qemu(amd_mode=args.amd)
