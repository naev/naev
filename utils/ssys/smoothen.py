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
