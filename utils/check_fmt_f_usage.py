#!/usr/bin/env python3

# This script is a fragile attempt to audit fmt.f() usages.
# BACKGROUND
# In the common case, we call fmt.f( _([[Format string requesting {foo}, {bar}, {1}]]), {"pos", foo="kw1", bar="kw2"} )
# The table keys requested in the format string should match the ones provided in the table.
# If this is not the case, flag the call.
# USAGE
# cd to the git repo root. (The script assumes it's working in a git checkout, not an extracted source tarball.)
# Then just execute this script (no arguments).

import logging
import re
import sys
import tempfile

logging.basicConfig()
logging.getLogger().setLevel(logging.INFO)

def main():
   errors = 0
   for filename in sys.argv[1:] or all_files_in_repo():
      for keys, tabkeys, fstr in analyze(filename):
         errors += 1
         print(f'In {filename} requested {keys} and provided {tabkeys}')
         print(fstr)
         print()
   if errors:
      sys.exit(f'The {sys.argv[0]} script reported {errors} errors. Please fix, or disable this step in .pre-commit-config.yaml in case of unfixable false positives.')

def all_files_in_repo():
   import subprocess
   with subprocess.Popen(['git', 'grep', '-Il', r'\<fmt\.f(', 'dat/**.lua'], stdout=subprocess.PIPE, encoding='utf-8') as pipe:
      for line in pipe.stdout:
         yield line.rstrip()

def analyze(fn):
   with open(fn) as f:
      matches = re.finditer(
         r'''
         \bfmt\.f\(\s*
            _\(\s*
               (?:
                  \[\[
                  (?P<bstr>(?: \\. | [^]\\] | ](?![]]) )*)
                  \]\]\s*
               |
                  "
                  (?P<qstr>(?: \\. | [^"\\] )*)
                  "\s*
               )
            \)\s*
            ,\s*
            (?P<table>
               {[^{}]*}      # Lua table constructor
               | _\(.*?      # Unquestionably a screwup, not a table (no need to capture precisely, just get context)
               | ".*?        # Ditto
               | \[\[.*?      # Ditto
            )\s*
         \)
         ''',
         f.read(),
         re.M | re.S | re.X
      )
      for m in matches:
         if not m.group('table').startswith('{'):
            yield '[a table]', m.group('table'), m.group(0)
            continue

         keys = set(re.findall(r'{([^{}:]*)(?::[^{}]*|)}', m.group('bstr') or m.group('qstr') or ''))
         tab = m.group('table')[1:-1].strip().rstrip(',')
         l = 1 + len(tab)
         while l > len(tab):
            l = len(tab)
            tab = re.sub(r'\([^()]*\)', '', tab)
         tabkey_expr = r'(?:^|,)\s*([a-zA-Z0-9_]+)\s*=[^,]*'
         tabkeys = set(re.findall(tabkey_expr, tab))
         tab = re.sub(tabkey_expr, '', tab)
         if tab:
            tabkeys.update(map(str, range(1, 2+tab.count(','))))
         keys, tabkeys = keys - tabkeys, tabkeys - keys
         if keys or tabkeys:
            yield keys, tabkeys, m.group(0)


if __name__ == '__main__':
   main()
