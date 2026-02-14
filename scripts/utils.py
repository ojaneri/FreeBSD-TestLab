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

"""
Common utility functions for the FreeBSD lab scripts.

Includes image validation, process management, and shell command execution helpers.
"""

import os
import sys
import subprocess
import time
import config

def get_qemu_base_command(amd_mode=False):
    """
    Constructs the base QEMU command based on the provided configuration.

    Args:
        amd_mode (bool): If True, use AMD-specific configurations.

    Returns:
        list: A list containing the command and its arguments.
    """
    memory = config.AMD_MEMORY if amd_mode else config.DEFAULT_MEMORY
    cores = config.AMD_CPU_CORES if amd_mode else config.DEFAULT_CPU_CORES
    ssh_port = config.AMD_SSH_PORT if amd_mode else config.DEFAULT_SSH_PORT
    cpu_model = config.AMD_CPU_MODEL if amd_mode else "host"

    return [
        "qemu-system-x86_64",
        "-m", memory,
        "-smp", cores,
        "-cpu", cpu_model,
        "-enable-kvm",
        "-hda", config.IMAGE_PATH,
        "-netdev", f"user,id=net0,hostfwd=tcp::{ssh_port}-:22",
        "-device", "e1000,netdev=net0",
        "-nographic"
    ]

def check_image_exists():
    """
    Checks if the specified FreeBSD disk image exists on disk.
    
    Returns:
        bool: True if image exists, False otherwise.
    """
    if not os.path.exists(config.IMAGE_PATH):
        print(f"[-] Error: Image {config.IMAGE_PATH} not found.")
        print(f"[-] Please run fast_download.py first.")
        return False
    return True

def kill_existing_qemu():
    """Kills any running qemu-system-x86_64 processes."""
    print("[*] Checking for existing QEMU instances...")
    try:
        # Use -f to match the full command line (handles names > 15 chars)
        subprocess.run(["pkill", "-9", "-f", "qemu-system-x86_64"], check=False)
        time.sleep(1)
    except Exception:
        pass

def run_command(cmd, shell=True, check=True):
    """Utility to run shell commands and handle errors."""
    try:
        result = subprocess.run(cmd, shell=shell, check=check, capture_output=True, text=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"[-] Command failed: {e}")
        print(f"[-] Output: {e.output}")
        sys.exit(1)
