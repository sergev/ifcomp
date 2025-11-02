#!/usr/bin/env python3
import re
import os

total = 0
executed = 0

files = ['ifcomp.cpp', 'pass1.cpp', 'pass2.cpp', 'pass3.cpp', 'pass4.cpp', 'pass5.cpp', 'pass6.cpp', 'pass7.cpp', 'pass8.cpp']

for f in files:
    gcov_file = f'{f}.gcov'
    if os.path.exists(gcov_file):
        with open(gcov_file, 'r') as g:
            for line in g:
                m = re.match(r'^\s*(\d+|#####):\s*\d+:\s*', line)
                if m:
                    if m.group(1).isdigit() and int(m.group(1)) > 0:
                        executed += 1
                    total += 1

if total > 0:
    coverage = (executed / total) * 100
    print(f'C++ Test Coverage: {coverage:.1f}%')
else:
    print('Coverage data not available')

