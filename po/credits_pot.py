#!/usr/bin/env python
import json
import os
import re
import sys

for fn in sys.argv[1:]:
    with open(fn) as f:
        fn = re.sub('.*/dat', 'dat', fn)
        for i, line in enumerate(f):
            line = line.rstrip('\r\n')
            if line and not line.startswith('['):
                quoted_escaped_line = json.dumps(line, ensure_ascii=False)
                print('#: {}:{}\nmsgid {}\nmsgstr ""\n'.format(fn, i+1, quoted_escaped_line))
