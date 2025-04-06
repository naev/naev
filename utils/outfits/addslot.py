#!/usr/bin/python


from os import path
from sys import argv,stderr
import xml.etree.ElementTree as ET


classes={'Courier','Fighter','Bomber','Destroyer','Armoured Transport','Freighter','Battleship','Carrier'}
# Not obvious
classes.add('Bulk Freighter')

subst={
   'Unicorp PT-68 Core System':'Unicorp PT-16 Core System',
   'Unicorp PT-310 Core System':'Unicorp PT-200 Core System',
   'Unicorp PT-1750 Core System':'Unicorp PT-440 Core System'
}
as_is={"Previous Generation Small Systems","Previous Generation Medium Systems","Previous Generation Large Systems","Dummy Systems"}
for i in subst.itervalues():
   as_is.add(i)

def get_path(s):
   s=path.dirname(s)

   if s=='':
      return ''
   else:
      return s+path.sep

def nam2fil(s):
   return s.replace(' ','_').replace('-','').replace("'",'').lower()

# Does not manage circular inheritance. Should not happen.
def find_class(arg):
   T=ET.parse(arg)
   R=T.getroot()

   for e in R.findall('./class'):
      return e.text

   if R.attrib.has_key('inherits'):
      return find_class(get_path(arg)+nam2fil(R.attrib['inherits']+".xml"))
   else:
      return ''

def main(arg):
   if not arg.endswith('.xml'):
      return

   T=ET.parse(arg)
   R=T.getroot()

   if R.tag!="ship":
      #print >>stderr,"not a ship:",R.tag
      return

   ok=False
   cls=find_class(arg)

   #print >>stderr,arg
   if cls not in classes:
      #print >>stderr,R.attrib['name'],":","not the right class:",cls
      return

   next_time=False
   crt=None
   newdefault=''
   for S in R.findall("./slots"):
      count=0
      for r in S:
         if next_time:
            if r.attrib['prop']=='systems_secondary':
               #print >>stderr,'already done, bye!'
               crt=r
            break
         elif r.tag=='utility' and r.attrib.has_key('prop') and r.attrib['prop']=='systems':
            siz=r.attrib["size"]
            if r.text=='' or r.text in as_is:
               newdefault=''
            elif subst.has_key(r.text):
               newdefault=subst[r.text]
               r.text=newdefault
            else:
               print "Err: unknown outfit",r.text
               return
            next_time=True

         count+=1
      break

   if next_time:
      if crt is None:
         crt = ET.Element('utility')
         crt.attrib['size']=siz
         crt.attrib['prop']='systems_secondary'
         S.insert(count,crt)

      if newdefault!='':
         crt.text=newdefault
      T.write(arg)

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      nam=path.basename(argv[0])
      print "usage:",nam,'[-r]','<outfit.xml> ...'
      print "  Adds a new secondary core system slot to the ships given in input, "
      print "  provided they have the right size and don't already have one."
      print '   -r  undo deprecation'
   else:
      for arg in argv[1:]:
         main(arg)
