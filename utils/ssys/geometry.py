#!/usr/bin/env python3


from math import cos, sin, pi, sqrt, asin

EPS = 0.0000001


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
   __hash__ = tuple.__hash__

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

   def __eq__( self, other ):
      if isinstance(other, tuple):
         other = _vec(other)
      if not isinstance(other, _vec):
         return False
      else:
         d = self - other
         return d*d < EPS*EPS

   def rotate_rad( self, angle ):
      return self._rotate(sin(angle), cos(angle))

   def rotate( self, degrees ):
      return self.rotate_rad(degrees / 180.0 * pi)

   def size( self ):
      return sqrt(self * self)

   def normalize( self, new_size = 1.0 ):
      if (size:= self.size()) < EPS:
         return self
      else:
         return self / size * new_size


class _transf:
   """
   A rotation and a scaling.
   Defined by a pair of vectors (before trans, after trans)
   """
   def __init__( self, v1, v2 ):
      l1 = v1.size()
      self.fact = v2.size()/l1
      v1, v2 = v1.normalize(), v2.normalize()
      self.vec = v1[0]*v2[1] - v1[1]*v2[0]
      if self.vec > 1.0:
         self.vec = 1.0
      elif self.vec < -1.0:
         self.vec = -1.0
      if v1*v2 < 0:
         self.vec = -self.vec

   def __str__( self ):
      return str({'fact': self.fact, 'vec': self.vec})

   def __call__( self, other ):
      sa = self.vec
      return other._rotate(sa, sqrt(1.0 - sa*sa)) * self.fact


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

   def __sub__( self, P ):
      u = self.P - P
      v = self.v
      u-= (u * v) * v / (v * v)
      return u

   def closest(self, P):
      return P + (self - P)

   # use & to call
   def __and__( self, other ):
      if isinstance(other, line):
         u = self.v
         v = other.v.orth()
         s = u*v
         if abs(s) < EPS:
            return None
         else:
            return self.P + (other.P - self.P) * v / s * u
      elif isinstance(other, segment):
         P = self & other.line()
         if P and P in other:
            return P
         else:
            return None
      elif isinstance(other, circle):
         u = self - other.center
         if (cl := (other.center + u)) not in other:
            return []
         else:
            v = u.orth().normalize() * sqrt(other.radius*other.radius - u*u)
            if v.size() < EPS:
               return [cl]
            else:
               return [cl - v, cl + v]
      else:
         return NotImplemented

   def __repr__( self ):
      return 'line' + repr((self.P, self.v))

   __str__ = __repr__

class segment:
   def __init__( self, A, B ):
      self.A, self.B = A, B

   def line( self ):
      return line(self.B, self.B - self.A)

   def bisector( self ):
      return line((self.A + self.B) / 2.0, (self.B - self.A).orth())

   def __and__ ( self, other ):
      if isinstance(other, line):
         return other & self
      elif isinstance(other, segment):
         if self.line() & other:
            return other.line() & self
         else:
            return None
      else:
         return NotImplemented
   def points( self ):
      return set([self.A, self.B])

   # !!! only test whether C strictly between
   # the lines orth to (A B) though resp A and B.
   # If you also want A, B, C to be colinear, you have to do the test separately.
   def __contains__( self, C ):
      u = self.B - self.A
      return ((C-self.A)*u) * ((C-self.B)*u) < - EPS

   def closest( self, P ):
      H = self.line().closest(P);
      if H in self:
         return H
      elif self.A in segment(self.B, H):
         return self.A
      else:
         return self.B

class circle:
   def __init__( self, center = vec(), radius = 0.0, strict = False ):
      self.center = center
      self.radius = radius
      self.strict = False

   def __repr__( self ):
      return 'circle' + repr((self.center, self.radius))

   __str__ = __repr__

   def strictness( self, strict ):
      prv, self.strict = self.strict, strict
      return prv

   def __eq__( self, other ):
      if not isinstance(other, circle):
         return False
      d = self.radius - other.radius
      return self.center == other.center and d*d < EPS*EPS

   def __contains__( self, P ):
      u = P - self.center
      epsilon = -EPS if self.strict else +EPS
      return u * u <= self.radius * self.radius * (1.0 + epsilon)

def inscribed( A, B, C ):
   la = line(A , (B - A).normalize() + (C - A).normalize())
   lb = line(B , (C - B).normalize() + (A - B).normalize())
   P = la & lb
   return circle(P, (line(A, B - A) - P).size())

def inscribed_lines( l1, l2, l3 ):
   P3, P1, P2 = (l1 & l2, l2 & l3, l3 & l1)
   if P1 and P2 and P3:
      return inscribed(P3, P1, P2)
   # TODO: manage other cases

def circumscribed( A, B, C ):
   la = line ((B + C) / 2.0, (B - C).orth())
   lb = line ((C + A) / 2.0, (C - A).orth())
   P = la & lb
   return circle(P, (A - P).size())

# subsets of S that have size n
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

def acute_T( A, B, C ):
   return (B-A)*(B-C)>=0.0-EPS and (C-B)*(C-A)>=0.0-EPS and (A-C)*(A-B)>=0.0-EPS

def interstices( edges, vertices = [], povfp = None ):
   S = [segment(*t) for t in edges]
   sides = set([(b,a) for (a,b) in edges] + edges)
   L = list(set([a for (a, b) in edges] + [b for (a, b) in edges] + vertices))

   def candidates( ):
      # line, line, line -> inscribed
      for t in subsets(S, 3):
         lines = tuple(x.line() for x in t)
         if C := inscribed_lines(*lines):
            c = C.center
            if c in t[0] and c in t[1] and c in t[2]:
               yield C, tuple(l.closest(c) for l in lines), 0
      # line, line, point -> manual solution
      # TODO: manage the case the lines are parallel
      for t1, t2 in subsets(S, 2):
         l1, l2 = t1.line(), t2.line()
         I = l1 & l2
         for P in set(L).difference(t1.points(), t2.points()):
            u = l1.v * (P-I) * l1.v
            v = l2.v * (P-I) * l2.v
            if u*u < EPS or v*v < EPS:
               continue
            O = I + u.normalize() + v.normalize()
            UC = circle(O, (l1 - O).size())
            li = segment(I, P).line()
            if (P2 := (li & UC)) == []:
               continue
            P2 = P2[-1] if (P2[0]-I).size()<=(P2[-1]-I).size() else P2[0]
            ratio = ((P-I) * (P-I)) / ((P2-I) * (P-I))
            C = circle(I + (O-I)*ratio, UC.radius * abs(ratio))
            c = C.center
            if c in t1 and c in t2:
               yield C, (P, l1.closest(c), l2.closest(c)), 1
      # point, point, line -> the point point line problem of Apollonius (PPL)
      for t in S:
         l1 = t.line()
         for (A, B) in subsets(list(set(L).difference(t.points())), 2):
            if (A, B) in sides:
               continue
            AB = segment(A, B)
            l2 = AB.line()
            O = l1 & l2
            if O is None:
               T2 = AB.bisector() & l1
            else:
               C = (A + (O + (O - B))) / 2.0
               C1 = circle(C, (C-A).size())
               T = line(O, l2.v.orth()) & C1
               if T == []:
                  continue
               T = T[-1] # take one intersection
               v = l1.v.normalize((T-O).size())
               if v*((A+B)/2-O) <= 0.0:
                  v = -v
               T2 = O + v
            if T2 in t:
               yield circumscribed(A, B, T2), (A, B, T2), 2
      # point, point, point -> circumscribed
      for t in subsets(L, 3):
         if sides.isdisjoint(subsets(t,2)):
            yield circumscribed(*t), t, 3
   out = []
   for c, T, n in candidates():
      if acute_T(*T):
         out.append(c)
         prev = c.strictness(True)
         for t in S:
            if t.closest(c.center) in c:
               if povfp and n!=3:
                  print_circle(povfp, out.pop(), "Magenta")
               else:
                  out.pop()
               break
         c.strictness(prev)
   prv = None
   for C in sorted(out, key = lambda c: c.radius, reverse = True):
      if C != prv:
         prv = C
         yield C

def bounded_circles( L, povfp = None ):
   sides = list(zip(L, L[1:]+L[:1]))
   ext = vec(min([x for (x,_) in L]), min([y for (_,y) in L])) - vec(1, 1)
   for C in interstices(sides , povfp = povfp):
      l = segment(C.center, ext)
      if len([1 for s in sides if l & segment(*s)]) % 2 == 1:
         yield C

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
   from tempfile import NamedTemporaryFile
   from subprocess import run
   from random import random
   import os
   pov_head = """
   #version 3.7;
   global_settings{ assumed_gamma 1.6 ambient_light 1.0 }
   #include "colors.inc"
   camera{
      orthographic
      sky         <0,1,0>
      location    z
      up          -2*y
      right       2*x*image_width/image_height
      look_at     0
   }
   light_source{ <10,10,100> White }
   //cylinder{ <0,0,0>, <0,1.0,0>, 0.002 pigment{color Green} }
   //cylinder{ <0,0,0>, <1.0,0,0>, 0.002 pigment{color Red} }
   """
   getpath = lambda *x: os.path.realpath(os.path.join(*x))
   script_dir = os.path.dirname(__file__)
   povfp = NamedTemporaryFile(mode='wt', buffering=-1, suffix='.pov')
   povname = povfp.name
   povfp.write(pov_head)

   point_it = lambda P: '<'+str(P[0])+','+str(P[1])+',0>'
   def print_point( fp, P, c = 'White' ):
      fp.write('sphere{ ' + point_it(P) + ', 0.002 pigment{color ' + c + '}}\n')

   def print_seg( fp, A, B, c = 'White' ):
      fp.write('cylinder{ ' + point_it(A) + ','+ point_it(B))
      fp.write( ', 0.002 pigment{color ' + c + '}}\n')

   def print_cross( fp, A, c = 'White', alt_cross = False ):
      u = (0.016 * vec(1,0)) if alt_cross else (0.008 * vec(1,1))
      print_seg(fp, A - u, A + u, c)
      print_seg(fp, A - u.orth(), A + u.orth(), c)

   def print_poly( fp, L, c = 'White' ):
      [print_seg(fp, *t, c) for t in zip(L,L[1:]+L[:1])]
      [print_point(povfp, P) for P in L]

   def print_circle( fp, C, c = 'White', alt_cross = False ):
      print_cross(fp, C.center, c, alt_cross)
      fp.write('torus{ ' + str(C.radius) + ', 0.002 ')
      fp.write('rotate 90*x ' + 'translate '+point_it(C.center))
      fp.write('pigment{color ' + c + '}}\n')

   ranv = lambda: vec(random()-0.5, random()-0.5)

   W, H = 3, 2
   Pols = [[[ranv() for _ in range(4)] for _ in range(W)] for _ in range(H)]
   Pols[0][0] = [0.35*vec(*t) for t in [
      (-0.96,-1), (0.91,-1), (1.02,1),
      (0.2,-0.52), (0.35,-0.54), (0.01, 1.0), (-0.33,-0.51), (-0.43,-0.5),
      (-1.03,1.03)
   ]]
   u = vec(0,1)
   v = vec(0,1.01).rotate(120)
   w = vec(0,1.02).rotate(-120)
   Pols[0][1] = [0.35*t for t in [u,-0.3*v,w,-0.3*u,v,-0.3*w]]
   Pols[0][2] = [vec(0.35,0).rotate(360.0/5.0*i) for i in range(5)]
   # TODO: manage parallel cases
   #u, v = vec(0.3,-0.2), vec(0.3,0.2)
   #Pols[0][2] = [u, v, -u, -v]

   for j in range(W):
      u = 16.0/9.0*(1.0*j/(W-1)-0.5)
      for i in range(H):
         v = 1.0 * i / (H-1) - 0.5
         Pol = [P - vec(u, v) for P in Pols[i][j]]
         B = bounding_circle(Pol)
         if B:
            d = B.center - vec(u, v)
            Pol = [P - d for P in Pol]
            B.center -= d
            print_circle(povfp, B, 'Blue', alt_cross = True)
         print_poly(povfp, Pol, 'Yellow')
         color = 'Orange'
         for B in bounded_circles(Pol, None):
            print_circle(povfp, B, color)
            color = 'Red'

   povfp.flush()
   run(['povray', '+W1280', '+H720', '+A0.1', '+AM2', '+R3', '+J',
      '+I' + povfp.name, '+O' + getpath(script_dir, 'geometry.png') ])
   povfp.close()
