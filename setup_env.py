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
# Project: FreeBSD Engineering Lab
# Component: Environment Setup Utility
#

import os
import sys
import subprocess
import shutil
import venv

def check_dependencies():
    """
    Verifies that required system utilities are installed.
    """
    required_tools = ["qemu-system-x86_64", "xz", "ssh", "gh"]
    print("[*] Verifying system dependencies...")
    missing = []
    for tool in required_tools:
        if shutil.which(tool) is None:
            missing.append(tool)
    
    if missing:
        print(f"[!] Warning: Missing system tools: {', '.join(missing)}")
        print("[!] Please install them using your system package manager.")
    else:
        print("[+] All core system dependencies found.")

def main():
    """
    Sets up a Python virtual environment and installs project dependencies.
    """
    project_root = os.path.dirname(os.path.abspath(__file__))
    venv_dir = os.path.join(project_root, ".venv")
    requirements_file = os.path.join(project_root, "requirements.txt")

    print(f"[INFO] Initializing environment setup in: {project_root}")

    # 0. Check System Dependencies
    check_dependencies()

    # 1. Create Virtual Environment
    if not os.path.exists(venv_dir):
        print(f"[INFO] Creating virtual environment in {venv_dir}...")
        venv.create(venv_dir, with_pip=True)
    else:
        print(f"[INFO] Virtual environment already exists.")

    # 2. Identify the pip executable within the venv
    if os.name == 'nt':
        pip_exe = os.path.join(venv_dir, "Scripts", "pip.exe")
    else:
        pip_exe = os.path.join(venv_dir, "bin", "pip")

    # 3. Upgrade pip
    print("[INFO] Upgrading pip...")
    subprocess.run([pip_exe, "install", "--upgrade", "pip"], check=True)

    # 4. Install requirements
    if os.path.exists(requirements_file):
        print(f"[INFO] Installing dependencies from {requirements_file}...")
        subprocess.run([pip_exe, "install", "-r", requirements_file], check=True)
        print("[SUCCESS] Dependencies installed successfully.")
    else:
        print("[WARNING] requirements.txt not found. Skipping dependency installation.")

    print("\n[FINISH] Environment setup complete.")
    print("[HINT] To activate the virtual environment, run:")
    if os.name == 'nt':
        print(f"    {venv_dir}\\Scripts\\activate")
    else:
        print(f"    source {venv_dir}/bin/activate")

if __name__ == "__main__":
    main()
