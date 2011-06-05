#!/usr/bin/env python2

__version__="0.1"

import os, sys
from optparse import OptionParser

import pydot

config = dict({
        'datPath': '../../dat/',
        'verbose': True})

def planet_filter( p ):
   details = assetsObj.getPlanetDetails( p )
   if not details:
      return False
   if details['population'] <= 0:
      return False
   return True

if __name__ == "__main__":
   from datreader import readers

   # Load data
   ssysObj   = readers.ssys(**config)
   assetsObj = readers.assets(**config)

   # Create graph
   graph     = pydot.Dot( 'Naev_Universe', graph_type='graph' )

   # Iterate over systems and planets
   ssys_added = []
   jumps_added = []
   for (ssysName, planets) in ssysObj.assetsList.iteritems():
      # Create cluster
      #subg = pydot.Subgraph( graph_name='A', rank='same', label=ssysName )
      subg = graph

      # Create jumps
      jumps = ssysObj.jumpgatesForSystem( ssysName )
      jump_nodes = []
      for j in jumps:
         # Sort by name
         jname = j['target']
         if jname < ssysName:
            name = '%s-%s' % (jname, ssysName)
         else:
            name = '%s-%s' % (ssysName, jname)
         # First time addition
         if name not in jumps_added:
            node = pydot.Node( name, shape='diamond' )
            subg.add_node( node )
            jumps_added.append( name )
         # Add edges between jumps for empty systems
         for jn in jump_nodes:
            edge = pydot.Edge( name, jn )
            subg.add_edge( edge )
         # Add node
         jump_nodes.append( name )

      # We don't want all the planets
      planets_filtered = filter( planet_filter, planets )
      planets_added = list()
      # Iterate to create maps
      for p in planets_filtered:
         # Add the node
         node     = pydot.Node( p )
         subg.add_node( node )
         # Add planet links
         for t in planets_filtered:
            if p == t or t in planets_added:
               continue
            edge = pydot.Edge( p, t, style='dotted' )
            subg.add_edge( edge )
         # Add jump edges
         for j in jump_nodes:
            edge = pydot.Edge( p, j )
            subg.add_edge( edge )
         # Visited planet
         planets_added.append(p)

      #graph.add_subgraph( subg )
      ssys_added.append( ssysName )

   graph.write_png('test.png')



