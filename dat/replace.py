#!/usr/bin/python

from sys import argv

subs={
   "Nexus Ultralight Stealth Plating":"Nexus Shadow Plating",
   "Nexus Medium Stealth Plating":"Nexus Ghost Plating",
   "Nexus Heavy Stealth Plating":"Nexus Phantasm Plating",
   "S&K Ultralight Combat Plating":"S&K Skirmish Plating",
   "S&K Medium Combat Plating":"S&K Combat Plating",
   "S&K Heavy Combat Plating":"S&K Battle Plating"
}

args=[s for s in argv[1:] if s.split('.lua')[-1]=='' or s.split('.mvx')[-1]=='' or s.split('.xml')[-1]=='']
args=[s for s in args if s.split('/')[-1]!="save_updater.lua"]

for a in args:
   fp=file(a,"r")
   contents=fp.read()
   fp.close()
   for k,v in subs.items():
      contents=contents.replace(k[2:],v[2:])

   fp=file(a,"w")
   fp.write(contents)
   fp.close()

