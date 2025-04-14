#!/usr/bin/python


from os import path
from sys import argv,stderr
import xml.etree.ElementTree as ET
from outfit import nam2fil


classes={'Courier','Fighter','Bomber','Destroyer','Armoured Transport','Freighter','Battleship','Carrier'}
# Not obvious
classes.add('Bulk Freighter')


#subst={
#   'Unicorp PT-68 Core System':'Unicorp PT-16 Core System',
#   'Unicorp PT-310 Core System':'Unicorp PT-200 Core System',
#   'Unicorp PT-1750 Core System':'Unicorp PT-440 Core System'
#}
#as_is={"Previous Generation Small Systems","Previous Generation Medium Systems","Previous Generation Large Systems","Dummy Systems"}
#subst={
#   "Unicorp D-72 Heavy Plating":"Unicorp D-58 Heavy Plating",
#   "Unicorp D-9 Light Plating":"Unicorp D-2 Light Plating",
#   "Unicorp D-38 Medium Plating":"Unicorp D-23 Medium Plating",
#   "SK Light Combat Plating":"SK Ultralight Combat Plating",
#   "SK Mediumheavy Combat Plating":"SK Medium Combat Plating",
#   "SK Superheavy Combat Plating":"SK Heavy Combat Plating",
#}
#as_is={"Dummy Plating","Patchwork Light Plating","Patchwork Medium Plating"}
subst={
	"Melendez Mammoth XL Engine":"Melendez Mammoth Engine",
	"Nexus Bolt 6500 Engine":"Nexus Bolt 3000 Engine",
	"Tricon Typhoon II Engine":"Tricon Typhoon Engine",
	"Unicorp Eagle 6500 Engine":"Unicorp Eagle 3000 Engine",
	"Melendez Buffalo XL Engine":"Melendez Buffalo Engine",
	"Nexus Arrow 1400 Engine":"Nexus Arrow 700 Engine",
	"Tricon Cyclone II Engine":"Tricon Cyclone Engine",
	"Unicorp Falcon 1400 Engine":"Unicorp Falcon 700 Engine",
	"Melendez Ox XL Engine":"Melendez Ox Engine",
	"Nexus Dart 360 Engine":"Nexus Dart 160 Engine",
	"Tricon Zephyr II Engine":"Tricon Zephyr Engine",
	"Unicorp Hawk 360 Engine":"Unicorp Hawk 160 Engine",
}
as_is={
   "Dummy Engine",
   "Beat Up Large Engine",
   "Krain Remige Engine",
   "Melendez Old Mammoth Engine",
   "Beat Up Medium Engine",
   "Krain Patagium Engine",
   "Beat Up Small Engine",
   "Zalek Test Engine",
}


for i in subst.itervalues():
   as_is.add(i)

def get_path(s):
   s=path.dirname(s)

   if s=='':
      return ''
   else:
      return s+path.sep

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

from outfit import outfit

def main(arg):
   o=outfit(arg)

   if o is None:
      return

   T,R=o.T,o.r

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
            #if r.attrib['prop']=='systems_secondary':
            #if r.attrib.has_key('prop') and r.attrib['prop']=='hull_secondary':
            if r.attrib.has_key('prop') and r.attrib['prop']=='engines_secondary':
               #print >>stderr,'already done, bye!'
               crt=r
            break
         #elif r.tag=='utility' and r.attrib.has_key('prop') and r.attrib['prop']=='systems':
         #elif r.tag=='structure' and r.attrib.has_key('prop') and r.attrib['prop']=='hull':
         elif r.tag=='structure' and r.attrib.has_key('prop') and r.attrib['prop']=='engines':
            siz=r.attrib["size"]
            txt=r.text
            txt=txt.strip()
            if txt=='' or txt in as_is:
               newdefault=''
            elif subst.has_key(txt):
               newdefault=subst[txt]
               r.text=newdefault
            else:
               print "Err: unknown outfit",txt,as_is
               return
            next_time=True
         count+=1
      break

   if next_time:
      if crt is None:
         #crt = ET.Element('utility')
         crt = ET.Element('structure')
         crt.attrib['size']=siz
         #crt.attrib['prop']='systems_secondary'
         #crt.attrib['prop']='hull_secondary'
         crt.attrib['prop']='engines_secondary'
         S.insert(count,crt)

      if newdefault!='':
         crt.text=newdefault

      o.write(arg)

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      nam=path.basename(argv[0])
      print "usage:",nam,'[-r]','<outfit.xml> ...'
      print "  Adds a new secondary core engines slot to the ships given in input, "
      print "  provided they have the right size and don't already have one."
   else:
      for arg in argv[1:]:
         print >>stderr,arg
         main(arg)
