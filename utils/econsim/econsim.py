#!/usr/bin/env python

__version__="0.1"

import os, sys
from optparse import OptionParser

import pydot

config = dict({
        'datPath': '../../dat/',
        'verbose': True})

def planet_filter( p ):
   details = assetsObj.getPlanetDetails( p )
   return details and int(details['population']) > 0


if __name__ == "__main__":
   from datreader import readers

   # Load data
   ssysObj   = readers.ssys(**config)
   assetsObj = readers.assets(**config)

   # Create graph
   print("Creating graph")
   graph     = pydot.Dot( 'Naev_Universe', graph_type='graph' )

   # Iterate over systems and planets
   ssys_added = []
   jumps_added = []
   i = 1
   for ssysName, planets in ssysObj.assetsList.items():

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
            graph.add_node( node )
            jumps_added.append( name )
         # Add edges between jumps for empty systems
         for jn in jump_nodes:
            edge = pydot.Edge( name, jn )
            graph.add_edge( edge )
         # Add node
         jump_nodes.append( name )

      # We don't want all the planets
      planets_filtered = list(filter( planet_filter, planets ))
      planets_added = list()
      # Create cluster
      if len(planets_filtered) > 0:
         subg = pydot.Cluster( "sys_%d"%i, label=ssysName )
         # Iterate to create maps
         for p in planets_filtered:
            # Add the node
            node     = pydot.Node( p )
            subg.add_node( node )
         graph.add_subgraph( subg )
         for p in planets_filtered:
            # Add planet links
            for t in planets_filtered:
               if p == t or t in planets_added:
                  continue
               edge = pydot.Edge( p, t, style='dotted' )
               graph.add_edge( edge )
            # Add jump edges
            for j in jump_nodes:
               edge = pydot.Edge( p, j )
               graph.add_edge( edge )
            # Visited planet
            planets_added.append(p)
      
      ssys_added.append( ssysName )
      i += 1

   #graph.set_prog( 'neato' )
   graph.set_simplify( False )
   graph.set( 'aspect',    1 )
   #graph.set( 'maxiter',   100000 )
   #graph.set( 'overlap',   False )
   nodes = graph.get_nodes()
   edges = graph.get_edges()
   v     = len(nodes)
   e     = len(edges)
   sparse = float(e+v)/float(v**2)
   print(("   %d nodes and %d edges (%.3f%% sparse)" % (v, e, sparse*100)))
   print("Outputting as naev_universe")
   graph.write_raw('naev_universe.dot')
   #graph.write_png('naev_universe.png', prog='neato', format='png')
   graph.write('naev_universe.png', prog='neato', format='png')



