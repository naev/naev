# python3


def add_virtual_edges( E, virtual ):
   already = {}
   virtual_edges = [x for x in virtual]
   for t in virtual_edges:
      i, j = tuple(t[:2])
      if i == j:
         stderr.write(t[0] + ' appears twice in the same edge!\n')
      elif (i, j) in already or (j, i) in already:
         stderr.write(str(tuple(t[:2])) + ' appears twice in virtual_edges list !\n')
      elif i in [ x for x, _ in E[j]] or j in [ x for x, _ in E[i]]:
         stderr.write(str(tuple(t[:2])) + ' already implied by an existing edge!\n')
      else:
         already.add((i,j))
         E[i].append((j, a + ['virtual']))
