#!/usr/bin/env python3


from math import cos, sin, pi, sqrt

class _vec(tuple):
   def __add__( self, other ):
      return _vec([x+y for (x, y) in zip(self, other)])

   def __sub__( self, other ):
      return self + other*-1.0

   def _rotate( self, sa, ca ):
      return _vec((self[0]*ca-self[1]*sa, self[0]*sa+self[1]*ca))

   def __mul__( self, other ):
      if isinstance(other, _vec):
         """
         Dot product
         """
         return sum([a*b for (a,b) in zip(self, other)])
      elif isinstance(other, transf):
         """
         Apply transformation
         """
         sa = other.vec
         return self._rotate(sa, sqrt(1.0 - sa*sa)) * other.fact
      else:
         """
         External product
         """
         return _vec([x*other for x in self])

   def __neg__( self ):
      return self * -1.0

   def __truediv__( self, other ):
      if isinstance(other, _vec):
         """
         The transformation that turns other into self.
         """
         return transf(other, self)
      else:
         return self * (1.0/other)

   def __round__( self, dig = 0 ):
      return _vec([round(x, dig) for x in self])

   def __str__( self ):
      return str(tuple([int(a) if int(a) == a else a for a in self]))

   def rotate( self, degrees ):
      angle = degrees / 180.0 * pi
      return self._rotate(sin(angle), cos(angle))

   def size_sq( self ):
      return self*self

   def size( self ):
      return sqrt(self.size_sq())

   def normalize( self, new_size = 1.0 ):
      return self / self.size() * new_size

   def to_dict( self ):
      return {'x':self[0], 'y':self[1]}

class transf:
   """
   A rotation and a scaling.
   Defined by a pair of vectors (before trans, after trans)
   """
   def __init__( self, v1, v2 ):
      l1 = v1.size()
      if v1*v2 < 0:
         l1 = -l1
         v1 = -v1
      self.fact = v2.size()/l1
      v1 = v1.normalize()
      v2 = v2.normalize()
      self.vec = v1[0]*v2[1] - v1[1]*v2[0]
   def __mul__( self, other ):
      """
      Application
      """
      if not isinstance(other, _vec):
         raise Exception('transf only applies to vec')
      return other*self
   def __add__( self, other ):
      """
      Composition
      """
      if not isinstance(other, transf):
         raise Exception('transf only adds with itself')
      v1 = _vec(1.0, 0.0)
      v2 = v1 * self * other
      return transf(v1, v2)

def vec( *args ):
   if len(args) == 0:
      return _vec((0.0, 0.0))
   else:
      if len(args) == 1:
         args = args[0]
      return _vec((float(x) for x in args))

# bounding boxes
class bb:
   def __init__( self ):
      self.empty = True

   def __contains__( self, v):
      return not (
         self.empty or
         self.minx > v[0] or self.maxx < v[0] or
         self.miny > v[1] or self.maxy < v[1]
      )
      
   def __iadd__( self, t ):
      if self.empty:
         (self.maxx, self.maxy) = t
         (self.minx, self.miny) = t
         self.empty = False
      else:
         if self.minx > t[0]:
            self.minx = t[0]
         elif self.maxx < t[0]:
            self.maxx = t[0]
         if self.miny > t[1]:
            self.miny = t[1]
         elif self.maxy < t[1]:
            self.maxy = t[1]
      return self

   def mini( self ):
      return vec(self.minx,self.miny) if not self.empty else None

   def maxi( self ):
      return vec(self.maxx,self.maxy) if not self.empty else None

   def __str__( self ):
      return str(round(self.mini()))+":"+str(round(self.maxi()))


def find_hole(L):
   # init pos with gravity center
   pos = vec()
   count = 0
   for p in L:
      pos += p
      count += 1
   pos /= 1.0*count

   for i in range(10):
      print('pos:'+str(round(pos,4)))
      maxi = 0.0
      for p in L:
         l = (pos-p).size()
         if l>maxi:
            maxi = l

      acc = vec()
      for p in L:
         v = p-pos
         acc += v.normalize(-((maxi/v.size())**2))

      pos += acc/(2**(i+3))
   return pos

"""
L=[(0.0,0.0), (1.0,1.0), (1.0, 0.0)]

find_hole([vec(*t) for t in L])
"""

