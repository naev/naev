#!/usr/bin/env python3

import argparse
import os
import re
import subprocess

def main():
    ''' Extract the number of strings in (usually) po/ '''
    ap = argparse.ArgumentParser(description='Count the strings in po/<naev>.pot; save to <dat>/gettext_stats/<naev>.txt.')
    ap.add_argument('-o', '--output')
    ap.add_argument('input')
    args = ap.parse_args()
    cmd = ['msgfmt', '--statistics', args.input, '-o', '/dev/null']
    env = dict(LC_ALL='C', PATH=os.getenv('PATH', ''))
    out = subprocess.check_output(cmd, env=env, encoding='ascii', stderr=subprocess.STDOUT)
    msgs = re.match(r'0 translated messages, (\d+) untranslated', out).group(1)
    with open(args.output, 'w', encoding='utf-8') as out:
        out.write(f'{msgs}\n')


if __name__ == '__main__':
    main()
