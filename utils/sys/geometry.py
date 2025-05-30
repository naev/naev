#!/usr/bin/env python3


from math import cos, sin, pi, sqrt, asin



# vec

class _vec(tuple):
   def __add__( self, other ):
      return _vec([x+y for (x, y) in zip(self, other)])

   def __sub__( self, other ):
      return self + other*-1.0

   def _rotate( self, sa, ca ):
      return _vec((self[0]*ca - self[1]*sa, self[0]*sa + self[1]*ca))

   def __mul__( self, other ):
      if isinstance(other, _vec):
         # Dot product
         return sum([a*b for (a,b) in zip(self, other)])
      elif isinstance(other, _transf):
         # Apply transformation
         return other(self)
      else:
         # External product
         return _vec([x*other for x in self])

   __rmul__ = __mul__

   def __neg__( self ):
      return self * -1.0

   def __truediv__( self, other ):
      if isinstance(other, _vec):
         # The transformation that turns other into self.
         return _transf(other, self)
      else:
         return self * (1.0/other)

   def __round__( self, dig = 0 ):
      return _vec([round(x, dig) for x in self])

   def __str__( self ):
      return str(tuple([int(a) if int(a) == a else a for a in self]))

   def rotate( self, degrees ):
      angle = degrees / 180.0 * pi
      return self._rotate(sin(angle), cos(angle))

   def sq( self ):
      return self*self

   def size( self ):
      return sqrt(self.sq())

   def normalize( self, new_size = 1.0 ):
      return self / self.size() * new_size

   def to_dict( self ):
      return { 'x': self[0], 'y': self[1] }


class _transf:
   """
   A rotation and a scaling.
   Defined by a pair of vectors (before trans, after trans)
   """
   def __init__( self, v1, v2, trn = 0 ):
      l1 = v1.size()
      self.fact = v2.size()/l1
      v1 = v1.normalize()
      v2 = v2.normalize()
      self.vec = v1[0]*v2[1] - v1[1]*v2[0]
      self.trn = trn

   def __str__( self ):
      return str({'fact': self.fact, 'vec': self.vec})

   def __call__( self, other ):
      sa = self.vec
      return other._rotate(sa, sqrt(1.0 - sa*sa)) * self.fact

   def __matmul__( self, other ):
      if isinstance(other, _transf):
         # Composition
         vi = _vec((1, 0))
         v1 = self(vi)
         s = v1 * vi
         vf = other(v1)
         if s * (other(vi) * vi) > 0 and s * (vf*vi) < 0:
            trn = 1 if s > 0 else -1
         else:
            trn = 0
         return _transf(vi, vf, self.trn + other.trn + trn)
      else:
         raise TypeError('transf does not compose with ' + str(type(other)))

   def get_angle( self ):
      return asin(self.vec) + trn * 2.0 * pi

   def __itruediv__( self, other ):
      if isinstance(other, int):
         out = transf()
         out.fact = pow(self.fact, 1.0/other)
         a = self.get_angle() / other
         out.vec = sin(a)
         #TODO: compute trn
         return out
      else:
         raise TypeError('transf does not divide with ' + str(type(other)))

def vec( *args ):
   if len(args) == 0:
      args = (0, 0)
   elif len(args) == 1:
      args = args[0]
   return _vec((float(x) for x in args))

id_transf = vec(1, 0) / vec(1, 0)
def transf():
   return copy(id_transf)


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
      return str(round(self.mini())) + ':' + str(round(self.maxi()))
