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
import pexpect
import sys
import time
import os
import config
import utils

def main():
    print("Starting automated boot process...")
    utils.kill_existing_qemu()
    
    # In a refactored environment, we might want to pass the port as argument
    # Defaulting to standard lab port
    ssh_port = config.DEFAULT_SSH_PORT
    
    # The command should ideally use the same parameters as qemu_setup.py
    # but for automation we just need the HDA path and ports.
    cmd = (f"qemu-system-x86_64 -m {config.DEFAULT_MEMORY} -smp {config.DEFAULT_CPU_CORES} "
           f"-enable-kvm -cpu host -hda {config.IMAGE_PATH} "
           f"-netdev user,id=net0,hostfwd=tcp::{ssh_port}-:22 -device e1000,netdev=net0 "
           f"-nographic -serial mon:stdio")
    
    print(f"Executing: {cmd}")
    child = pexpect.spawn(cmd, encoding='utf-8')
    child.logfile = sys.stdout

    try:
        print("\n[+] Waiting for Autoboot countdown...")
        child.expect('Autoboot', timeout=60)
        
        print("\n[+] Sending '3' once to escape to loader prompt...")
        child.send('3') 
        
        print("\n[+] Waiting for loader prompt...")
        # Wait for the loader prompt 'OK' or 'loader>'
        child.expect(['OK', 'loader>'], timeout=30)
        
        # Flush buffer
        child.sendline('')
        child.expect(['OK', 'loader>'], timeout=5)

        print("\n[+] Setting console to comconsole...")
        # Send character by character to avoid dropping in slow emulation
        for char in 'set console="comconsole"':
            child.send(char)
            time.sleep(0.05)
        child.sendline('')
        child.expect(['OK', 'loader>'], timeout=10)
        
        print("\n[+] Booting kernel...")
        child.sendline('boot')
        
        print("\n[+] Monitoring boot process. This will take a few minutes...")
        child.expect('login:', timeout=600)
        
        print("\n[+] Logging in as root...")
        child.sendline('root')
        
        i = child.expect(['Password:', '#', 'root@'], timeout=10)
        if i == 0:
            child.sendline('')
            child.expect(['#', 'root@'], timeout=10)
            
        print("\n[+] Logged in!")
        
        print("\n[+] Configuring /boot/loader.conf...")
        child.sendline('echo \'console="comconsole"\' >> /boot/loader.conf')
        child.expect(['#', 'root@'], timeout=5)
        
        print("\n[+] Configuration successful! Rebooting to test...")
        child.sendline('reboot')
        
        print("\n[+] Script finished.")
        
    except Exception as e:
        print(f"\n[!] Error: {e}")
        child.close()

if __name__ == "__main__":
    main()
