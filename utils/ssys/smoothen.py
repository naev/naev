# python3


from geometry import vec

def smoothen( pos, neigh, hard = False ):
   newp = dict()
   for i in pos:
      for j in neigh[i]:
         if i not in neigh[j]:
            neigh[i].add(j)
   for k in pos:
      if (n := len(neigh[k])) > 1:
         if hard:
            acc = vec()
            count = 0
         else:
            acc = pos[k]
            count = 1
         for s in neigh[k]:
            acc2 = vec()
            count2 = 0
            for u in neigh[s]:
               if u != k:
                  acc2 += pos[u]
                  count2 += 1
            if count2 == 0:
               acc += pos[s]
            else:
               acc += 1.5*pos[s] - 0.5*acc2/count2
            count += 1
         newp[k] = acc / count
   return newp

from math import sqrt
def circleify( pos, L, center ):
   newp = dict()
   V = [(pos[i] - pos[center]).size() for i in L]
   avg = sum(V) / len(V)
   for i in L:
      p = pos[center] + (pos[i]-pos[center]).normalize(avg)
      pos[i] = (pos[i] + 2.0*p) / 3.0
   """
   for i, j, k in zip(L[:-2],L[1:-1],L[2:]):
      c = (pos[i] + pos[k]) / 2.0
      v1, v2 = pos[i] - pos[center], pos[k] - pos[center]
      #avg = lambda x, y: (x + y) / 2.0
      avg = lambda x, y: sqrt(x * y)
      newp[j] = pos[center] + (pos[j] - pos[center]).normalize(avg(v1.size(), v2.size()))
   """
   return newp
