class UnionFind:
    '''Represents a partition of range(n) into mergeable disjoint subsets.'''

    def __init__(self, n):
        self.parent = list(range(n))
        self.rank = [0] * n

    def union(self, x, y):
        '''Declares x and y to be in the same subset.'''
        x = self.find(x)
        y = self.find(y)
        if self.rank[x] > self.rank[y]:
            self.parent[y] = x
        else:
            self.parent[x] = y
            if self.rank[x] == self.rank[y]:
                self.rank[y] += 1

    def find(self, x):
        '''Finds the designated representative of the subset containing x.'''
        if self.parent[x] != x:
            self.parent[x] = self.find(self.parent[x])
        return self.parent[x]

    def findall(self):
        '''Returns a designated representative of each subset.'''
        return [i for i,x in enumerate(self.parent) if i==x]
