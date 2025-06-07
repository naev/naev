# python3

class Slst(list):
   def __call__( self, st, debug = False ):
      for T in self:
         if debug:
            print('"' + st + '"')
         (s, t, c) = (T[0], (T+('',))[1], (T+(None, None))[2])
         if c is None:
            st = st.replace(s, t)
         elif c<0:
            st = ((st[::-1]).replace(s[::-1], t[::-1], -c))[::-1]
         else:
            st = st.replace(s, t, c)
      if debug:
         print('"' + st + '"')
      return st

   __add__= lambda *t: Slst(list.__add__(*t))

   def __getitem__( self, sl ):
      out = list.__getitem__(self, sl)
      return Slst(out) if isinstance(sl, slice) else out

   def __mul__( self, other ):
      return Slst([ (t[0], other((t+('',))[1])) + t[2:] for t in self])

   def __neg__( self ):
      return Slst([ ((t+('',))[1], t[0]) + t[2:] for t in reversed(self)])

   def __repr__(self):
      return 'Slst(' + list.__repr__(self) + ')'
   __str__ = __repr__
