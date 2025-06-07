#!/usr/bin/env python3


from math import cos, sin, pi, sqrt, asin



# vec

class _vec(tuple):
   def __add__( self, other ):
      return _vec([x+y for (x, y) in zip(self, other)])

   def __sub__( self, other ):
      return self + other*-1.0

   def orth( self ):
      return _vec((-self[1], self[0]))

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

   def __repr__( self ):
      return 'vec' + tuple.__repr__(self)

   def rotate_rad( self, angle ):
      return self._rotate(sin(angle), cos(angle))

   def rotate( self, degrees ):
      return self.rotate_rad(degrees / 180.0 * pi)

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
      if self.vec > 1.0:
         self.vec = 1.0
      elif self.vec < -1.0:
         self.vec = -1.0
      self.trn = trn
      if v1*v2 < 0:
         if self.vec < 0:
            self.trn -= 1
         else:
            self.trn += 1
         self.vec = -self.vec

   def __str__( self ):
      return str({'fact': self.fact, 'vec': self.vec})

   def __call__( self, other ):
      sa = self.vec
      sign = -1 if (self.trn%2 == 1) else 1
      return other._rotate(sa, sign * sqrt(1.0 - sa*sa)) * self.fact

   def __matmul__( self, other ):
      if isinstance(other, _transf):
         # Composition
         vi = _vec((1, 0))
         v1 = self(vi)
         s = v1 * vi
         vf = other(v1)
         if s * (other(vi) * vi) > 0 and s * (vf*vi) < 0:
            trn = 1 if s >= 0 else -1
         else:
            trn = 0
         return _transf(vi, vf, self.trn + other.trn + trn)
      else:
         raise TypeError('transf does not compose with ' + str(type(other)))

   def get_angle( self ):
      return asin(self.vec) + self.trn * pi

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

def transf():
   return vec(1, 0) / vec(1, 0)


# general geometry

class line:
   def __init__( self, P, v ):
      self.P = P
      self.v = v

   def dist( self, P ):
      u = P - self.P
      v = self.v
      u-= (u * v) * v / (v * v)
      return u.size()

   # use & to call
   def __and__( self, other ):
      u = self.v
      v = other.v.orth()
      s = u*v
      if abs(s) < 0.0000001:
         return None
      else:
         return self.P + (other.P - self.P) * v / s * u

   def __str__( self ):
      return str({'P': self.P, 'v': self.v})

   def __repr__( self ):
      return 'line' + repr((self.P, self.v))

class circle:
   # radius = True means infinity, radius = False means <= 0.0
   def __init__( self, center = vec(), radius = 0.0 ):
      self.center = center
      self.radius = radius

   def __str__(self):
      return str({'center': self.center, 'radius': self.radius})

   def __repr__( self ):
      return 'circle' + repr((self.center, self.radius))

   def __contains__(self, P):
      u = P - self.center
      if self.radius in [False, True]:
         return radius
      else:
         return u * u <= self.radius * self.radius

def inscribed( A, B, C ):
   la = line( A , (B - A).normalize() + (C - A).normalize() )
   lb = line( B , (C - B).normalize() + (A - B).normalize() )
   P = la & lb
   return circle(P, line(A, B - A).dist(P))

def circumscribed( A, B, C ):
   la = line ((B + C) / 2.0, (B - C).orth())
   lb = line ((C + A) / 2.0, (C - A).orth())
   P = la & lb
   return circle(P, (A - P).size())

def subsets( S, n ):
   if len(S) == n or n == 0:
      yield tuple(S[:n])
   else:
      S0, S = S[0], S[1:]
      for i in subsets(S, n):
         yield i
      for i in subsets(S, n-1):
         yield (S0,) + i

def bounding_circle( L ):
   def candidates( ):
      for (i, j) in subsets(L, 2):
         yield circle((i+j)/2, (i-j).size()/2)
      for (i, j, k) in subsets(L, 3):
         yield circumscribed(i, j, k)
   best = None
   for c in candidates():
      if (best is None or best.radius > c.radius):
         best, prv = c, best
         for p in L:
            if p not in c:
               best = prv
               break
   return best


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

   def size( self ):
      return vec(self.maxx - self.minx, self.maxy - self.miny)

   def __imul__( self, other ):
      other /= 2.0
      xd, yd = self.size()
      self.minx -= xd * other
      self.maxx += xd * other
      self.miny -= yd * other
      self.maxy += yd * other
      return self

   def mini( self ):
      return vec(self.minx, self.miny) if not self.empty else None

   def maxi( self ):
      return vec(self.maxx, self.maxy) if not self.empty else None

   def __str__( self ):
      return str(round(self.mini())) + ':' + str(round(self.maxi()))



# example

if __name__ == '__main__':
   A = vec(0, 0)
   B = vec(1, 0)
   C = vec(0, 1)
   print('triangle', A, B, C)
   print('inscribed', inscribed(A, B, C))
   print('circumscribed', circumscribed(A, B, C))
   print('bounding', bounding_circle([A, B, C]))
