# python3

from outfit import outfit
from copy import deepcopy
from sys import stderr


def mk_combine(args, combine, autostack, good):
   if combine or autostack:
      acc = []
      for i in args:
         if {i[:2], i[-2:][::-1]} & {'1x', '2x'} or '+' in i:
            stderr.write('"' + i + '" incompatible with -A/-C options -> ignored.\n')
         else:
            o = outfit(i, read_only = True)
            (p, s) = o.can_pri_sec()
            if p or s:
               acc.append((p, s, o.size(), o))
            else:
               stderr.write('"' + i + '" is not multicore. Ignored.\n')

      for p, _s, _S, o in acc:
         if p and o.can_alone():
            yield deepcopy(o).stack()

      if combine:
         for n, (p, s, s1, o1) in enumerate(acc):
            if p and s and o1.can_stack(o1):
                  yield (deepcopy(o1)).stack(o1)
            for q, t, s2, o2 in acc[n+1:]:
               if p and t and o1.can_stack(o2) and not (good and s1<s2):
                  yield (deepcopy(o1)).stack(o2)
               if q and s and o2.can_stack(o1) and not (good and s2<s1):
                  yield (deepcopy(o2)).stack(o1)
      elif autostack:
         for p, s, _S, o in acc:
            if p and s and o.can_stack(o):
               yield o.stack(o)
   else: # No need for copy here.
      for s in args:
         if '1x' in {s[:2], s[-2:][::-1]}:
            s = s[2:] if s[:2] == '1x' else s[:-2]
            s = s + '+'
         elif '2x' in {s[:2], s[-2:][::-1]}:
            s = s[2:] if s[:2] == '2x' else s[:-2]
            s = s + '+' + s
         s = s.split('+')
         if o := outfit(s[0], read_only = True):
            if len(s) == 2 :
               if s[1].strip() == '':
                  if o.can_alone():
                     yield o.stack()
               elif o.can_pri():
                  o2 = outfit(s[1], read_only = True)
                  if o2 and o2.can_sec() and o.can_stack(o2):
                     yield o.stack(o2)
            elif len(s) == 1 and o.can_alone():
               yield o
