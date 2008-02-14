#!/usr/bin/env python

from xml.dom import minidom
import sets

def uniq(alist):    # Fastest order preserving
   s = sets.Set(alist)
   del alist[:]
   for a in s:
      alist.append(a)


def load(xmlfile, tag, has_name=True, do_array=None, do_special=None):
   dom = minidom.parse(xmlfile)
   xmlNodes = dom.getElementsByTagName(tag)

   dictionary = {}
   for xmlNode in xmlNodes:

      mdic = {}
      # name is stored as a property and not a node
      if (has_name):
         name = xmlNode.attributes["name"].value

      # append the element to the dictionary
      for bignode in filter(lambda x: x.nodeType==x.ELEMENT_NODE, xmlNode.childNodes):
         mdic[bignode.nodeName], size = load_Tag( bignode, do_array, do_special )

      dictionary[name] = mdic
   
   dom.unlink()
   return dictionary


def load_Tag( node, do_array=None, do_special=None ):

   i = 0

   # figure out if we need an array or dic
   if node.nodeName in do_array:
      array = []
      use_array = True
   else:
      section = {}
      use_array = False

   for child in filter(lambda x: x.nodeType==x.ELEMENT_NODE, node.childNodes):

      n = 0
      if node.nodeName not in do_array and \
            do_special != None and \
            node.nodeName not in do_special.keys():
         children, n = load_Tag( child, do_array, do_special )

      # just slap the children on
      if n > 0:
         section[child.nodeName] = children
      
      # big ugly hack to use list instead of array
      elif use_array:
         array.append(child.firstChild.data)

      # uglier hack for special things
      elif do_special != None and node.nodeName in do_special.keys():
         section[child.firstChild.data] = \
               child.attributes[do_special[node.nodeName]].value

      # normal way (but will overwrite lists)
      else:
         section[child.nodeName] = child.firstChild.data

      i = i+1

   # return
   if use_array:
      return array, i
   else:
      return section, i



def save(xmlfile, data, basetag, tag, has_name=True, do_array=None, do_special=None):
   """
   do_array is a DICTIONARY, not a list here
   """
   xml = minidom.Document()

   base = xml.createElement(basetag)

   for key, value in data.items():

      elem = xml.createElement(tag)
      if has_name:
         elem.setAttribute("name",key)

      for key2, value2 in value.items():
         node = xml.createElement(key2)

         # checks if it needs to parse an array instead of a dictionary
         if do_array != None and key2 in do_array.keys():
            for text in value2:
               node2 = xml.createElement( do_array[key2] )
               txtnode = xml.createTextNode( str(text) )
               node2.appendChild(txtnode)
               node.appendChild(node2)

         # checks to see if we need to run the ULTRA UBER HACK
         elif do_special != None and key2 in do_special.keys():
            for key3, value3 in value2.items():
               node2 = xml.createElement( do_special[key2][0] )
               node2.setAttribute(do_special[key2][1], value3)
               txtnode = xml.createTextNode( str(key3) )
               node2.appendChild(txtnode)
               node.appendChild(node2)

         # standard dictionary approach
         else:
            for key3, value3 in value2.items():

               node2 = xml.createElement( key3 )

               if isinstance(value3,dict):
                  for key4, value4 in value3.items():
                     node3 = xml.createElement( key4 )
                     txtnode = xml.createTextNode( str(value4) )
                     node3.appendChild(txtnode)
                     node2.appendChild(node3)

               else:
                  txtnode = xml.createTextNode( str(value3) )
                  node2.appendChild(txtnode)

               node.appendChild(node2)

         elem.appendChild(node)
      base.appendChild(elem)
   xml.appendChild(base)

   fp = open(xmlfile,"w")
   xml.writexml(fp, "", "", "", "UTF-8")

   xml.unlink()




