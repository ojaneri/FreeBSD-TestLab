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
import os
import sys
import urllib.request
import shutil
import subprocess
from tqdm import tqdm
import config
import utils

def download_file(url, dest):
    """
    Downloads a file from the specified URL with a progress bar.

    Args:
        url (str): The URL to download the file from.
        dest (str): The local destination path to save the file.
    """
    print(f"[+] Downloading: {url}")
    try:
        req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
        with urllib.request.urlopen(req) as response:
            total_size = int(response.info().get('Content-Length', 0))
            block_size = 1024 * 8  # 8 KiB
            
            with open(dest, 'wb') as out_file:
                with tqdm(total=total_size, unit='iB', unit_scale=True, desc="FreeBSD Image") as pbar:
                    while True:
                        buffer = response.read(block_size)
                        if not buffer:
                            break
                        out_file.write(buffer)
                        pbar.update(len(buffer))
        print("[+] Download complete.")
    except Exception as e:
        print(f"[-] Error downloading: {e}")
        sys.exit(1)

def extract_file(archive, output):
    """
    Extracts an XZ archive using the system 'xz' utility.

    Args:
        archive (str): Path to the .xz archive.
        output (str): Destination path for the extracted file.
    """
    print(f"[+] Extracting {archive} to {output}...")
    try:
        # Construct path to 'xz' if needed, but assuming it's in PATH
        subprocess.run(f"xz -d -c {archive} > {output}", shell=True, check=True)
        print("[+] Extraction complete.")
    except subprocess.CalledProcessError as e:
        print(f"[-] Error extracting: {e}")
        sys.exit(1)

def main():
    print("--- FreeBSD Image Downloader ---")
    
    if os.path.exists(config.IMAGE_PATH):
        print(f"[!] File {config.IMAGE_PATH} already exists.")
        choice = input("Overwrite? (y/N): ").lower()
        if choice != 'y':
            print("Exiting.")
            sys.exit(0)

    # Ensure workloads directory exists
    os.makedirs(config.WORKLOADS_DIR, exist_ok=True)
    
    archive_path = os.path.join(config.WORKLOADS_DIR, "freebsd_image.xz")

    # Download
    download_file(config.IMAGE_URL, archive_path)
    
    # Extract
    extract_file(archive_path, config.IMAGE_PATH)
    
    # Cleanup
    if os.path.exists(archive_path):
        os.remove(archive_path)
        print(f"[+] Removed archive {archive_path}")

    print(f"[+] Done! Image ready: {config.IMAGE_PATH}")

if __name__ == "__main__":
    main()
