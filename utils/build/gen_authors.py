#!/usr/bin/env python3

import argparse
import re
import yaml

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-o', '--output')
    ap.add_argument('-p', '--preamble')
    ap.add_argument('yamls', nargs='*')
    args = ap.parse_args()
    people = deduplicate(authors(args.yamls))
    with open(args.output, 'w', encoding='utf-8') as out:
        out.write(open(args.preamble, encoding='utf-8').read())
        out.writelines(map('{}\n'.format, people))

def authors(yaml_paths):
    ''' Yield all authors/contributors named in the **_LICENSE.yaml files. '''
    for path in yaml_paths:
        for small_dict in yaml.safe_load(open(path, encoding='utf-8')):
            for credited_name in small_dict:
                # The credited name is basically a free-form field. Naively attempt to break it down.
                for component in re.split(r' *[(),] *| and | \+ ', credited_name):
                    component = re.sub(r'^(modified |retextured |made transparent |made |components |from |by |based on |works? |and )*', '', component, flags=re.I)
                    yield component

def deduplicate(raw_authors):
    ''' Try to dedup the redundancies in the extracted authors, e.g., the same person listed
    both with and without an email address. We get these because of laziness, but from crediting
    someone once by full name and in abbreviated form when they co-author something else.'''
    l = sorted(filter(None, raw_authors), key=len)
    for i in range(len(l)-1, -1, -1):
        k = l[i].lower()
        if any(k in s.lower() for s in l[i+1:]):
            del l[i]
    l.sort(key=str.lower)
    return l


if __name__ == '__main__':
    main()
