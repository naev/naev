#!/usr/bin/env python3

import os
import sys
import subprocess

command = sys.argv[1]
pattern = sys.argv[2]

print(f'Running {command}')
print(f'Waiting for {pattern}')

result = 1

proc = subprocess.Popen([command],
            bufsize=1,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            encoding='utf-8')

for line in proc.stdout:
    print(line, end='')
    if pattern in line:
        result = 0
        break

proc.kill()
sys.exit(result)