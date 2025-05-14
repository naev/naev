#!/usr/bin/env python3

from core_outfit import some_outfit

def mk_combine(args, combine, autostack, good):
   if combine or autostack:
      acc = []
      for i in args:
         if i[:2] == '2x' or i[-2:] == 'x2':
            stderr.write('"2x"')
         elif i[:2] == '1x' or i[-2:] == 'x1':
            stderr.write('"1x"')
         elif '+' in i:
            stderr.write('"+"')
         else:
            o = some_outfit(i)
            (p, s) = o.can_pri_sec()
            if p or s:
               acc.append((i, p, s, o.size(), o))
            else:
               stderr.write('"'+i+'" is not multicore. Ignored\n')
            continue
         stderr.write(' incompatible with -A/-C options -> "' + i + '" ignored.\n')

      out = ['1x'+t for t, p, _s, _S, o in acc if p and o.can_alone()]
      if combine:
         for n, (i, p, s, s1, o1) in enumerate(acc):
            for j, q, t, s2, o2 in acc[n:]:
               if p and t and o1.can_stack(o2) and not (good and s1<s2):
                  out.append(i + '+' + j)
               if i != j and q and s and o2.can_stack(o1) and not (good and s2<s1):
                  out.append(j + '+' + i)
      elif autostack:
         out.extend( [i+'+'+i for i, p, s, _S, o in acc if p and s and o.can_stack(o) ] )
      return out
   else:
      def t(s):
         if s[:2] == '2x' or s[-2:] == 'x2':
            s = s[2:] if s[:2] == '2x' else s[:-2]
            return s + '+' + s
      return [t(i) or i for i in args]
