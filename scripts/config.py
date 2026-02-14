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

"""Centralized configuration for the FreeBSD lab project.

This module defines all paths, ports, and hardware specifications used by the 
lab scripts.
"""

import os

# Base paths
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPTS_DIR = os.path.join(BASE_DIR, "scripts")
WORKLOADS_DIR = os.path.join(BASE_DIR, "workloads")
DOCS_DIR = os.path.join(BASE_DIR, "docs")
POC_DIR = os.path.join(BASE_DIR, "poc")

# Image configuration
IMAGE_NAME = "freebsd_debug.qcow2"
IMAGE_PATH = os.path.join(WORKLOADS_DIR, IMAGE_NAME)
IMAGE_URL = "https://download.freebsd.org/releases/VM-IMAGES/14.3-RELEASE/amd64/Latest/FreeBSD-14.3-RELEASE-amd64.qcow2.xz"

# QEMU configuration
DEFAULT_SSH_PORT = 2222
DEFAULT_GDB_PORT = 1234
DEFAULT_MEMORY = "2G"
DEFAULT_CPU_CORES = "2"

# AMD Specialized configuration
AMD_SSH_PORT = 2223
AMD_GDB_PORT = 1235
AMD_MEMORY = "4G"
AMD_CPU_CORES = "4"
AMD_CPU_MODEL = "EPYC-v4"
