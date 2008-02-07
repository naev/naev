#!/usr/bin/env python

from xml.dom import minidom

def load(xmlfile, tag, has_name=True, do_array=None):
   dom = minidom.parse(xmlfile)
   xmlNodes = dom.getElementsByTagName(tag)

   dictionary = {}
   for xmlNode in xmlNodes:

      mdic = {}
      # name is stored as a property and not a node
      if (has_name):
         name = xmlNode.attributes["name"].value

      # process the nodes
      for bignode in filter(lambda x: x.nodeType==x.ELEMENT_NODE, xmlNode.childNodes):
         # load the nodes
         section = {}
         array = []
         for node in filter(lambda x: x.nodeType==x.ELEMENT_NODE,
               bignode.childNodes):
            if bignode.nodeName in do_array: # big ugly hack to use list instead of array
               array.append(node.firstChild.data)
            else: # normal way (but will overwrite lists)
               section[node.nodeName] = node.firstChild.data

         if len(array) > 0:
            mdic[bignode.nodeName] = array
         else:
            mdic[bignode.nodeName] = section

      # append the element to the dictionary
      dictionary[name] = mdic
   
   dom.unlink()
   return dictionary

def save(xmlfile, basetag, tag, has_name=True):
	print "TODO"
