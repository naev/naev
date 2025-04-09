#!/usr/bin/python

from sys import stderr
import xml.etree.ElementTree as ET

lower_better={'mass','price','delay','ew_range','falloff','trackmin','trackmax','dispersion','speed_dispersion','energy_regen_malus','ew_stealth','ew_stealth_timer','ew_signature','launch_lockon','launch_calibration','fwd_energy','tur_energy','ew_track','cooldown_time','cargo_inertia','land_delay','jump_delay','delay','reload_time','iflockon','jump_warmup','rumble','ammo_mass','time_mod','ew_hide'}


def transpose(M):
   N=max(map(len,M))
   M=[t+['']*(N-len(t)) for t in M]
   return zip(*(tuple(M)))

getfloat=lambda s:float(s.split('/')[0])

#launch_reload
def main(args,gith=False,ter=False,noext=False,sortit=False):
   names=['']*len(args)
   L=[dict() for a in args]
   rang=dict()
   acc=[]

   if args==[]:
      return

   for i in range(len(args)):
      T=ET.parse(args[i]).getroot()
      for t in T.iter():
         try:
            n=getfloat(t.text)
            L[i][t.tag]=t.text
            if not rang.has_key(t.tag):
               rang[t.tag]=(n,n)
               acc.append(t.tag)
            else:
               (mi,ma)=rang[t.tag]
               rang[t.tag]=(min(mi,n),max(ma,n))
         except:
            pass

      if 'name' in T.attrib:
         names[i]=T.attrib['name']

      for e in T.findall("./general/shortname"):
         names[i]=e.text
         break

   for i,(m,M) in rang.iteritems():
      if i in lower_better:
         rang[i]=(M,m)
      if noext:
         rang[i]=(None,None)

   if sortit:
      for i,o in enumerate(L):
         o['name']=names[i]
      L.sort(key=lambda o:getfloat(o['mass']))
      for i,o in enumerate(L):
         names[i]=o['name']

   Res=[[k]+[l[k] if k in l else '' for l in L] for k in acc]
   names=['']+names
   length=[4]*len(names) # :--- at least 3 - required

   if ter:
      Sep,Lm,Rm,LM,RM='\033[34m|\033[37m',"\033[31m","\033[37m","\033[32m","\033[37m"
      mk_pad =lambda i,n:"\033[34m"+n*'-'+"\033[0m"
      leng=lambda x:len(x)- (10 if x!='' and x[0]=='\033' else 0)
   else:
      Sep,Lm,Rm,LM,RM='|',"_","_","**","**"
      mk_pad =lambda i,n:'-'*(n-1)+('-' if i==0 else ':')
      leng=len

   if ter:
      head=transpose([s.split(' ') for s in names])
   else:
      head=[names]

   if not gith:
      for r in Res:
         length=[max(n,len(s)) for (n,s) in zip(length,r)]

      for t in head:
         length=[max(n,len(s)) for (n,s) in zip(length,t)]

   mklin=lambda L:Sep+' '+(' '+Sep+' ').join(L)+' '+Sep
   fmt=lambda (s,n):(n-leng(s))*' '+s
   lfmt=lambda (s,n):s+(n-leng(s))*' '

   def emph(s,(mi,ma)):
      if s=='':
         return "_"
      elif mi!=ma:
         if getfloat(s)==mi:
            return Lm+s+Rm
         elif getfloat(s)==ma:
            return LM+s+RM
      return s

   print
   for t in head:
      print mklin(map(fmt,zip(t,length)))
   print mklin([mk_pad(i,n) for i,n in enumerate(length)])
   for r in Res:
      r=[r[0].replace("_"," ")]+[emph(k,rang[r[0]]) for k in r[1:]]
      print mklin([fmt(x) if i>0 else lfmt(x) for i,x in enumerate(zip(r,length))])

if __name__ == '__main__':
   import argparse

   parser = argparse.ArgumentParser(
      usage=" %(prog)s  [-g|-c] [-n] [-s]  [filename ...]",
      description="By default, outputs text aligned markdown table comparing the outfits respective values."
   )
   parser.add_argument('-g', '--github', action='store_true', help="unaligned (therefore smaller) valid github md, for use in posts.")
   parser.add_argument('-c', '--color', action='store_true', help="colored terminal output. You can pipe to \"less -RS\" if the table is too wide.")
   parser.add_argument('-n', '--nomax', action='store_true', help="Do not emphasize min/max values." )
   parser.add_argument('-s', '--sort', action='store_true', help="inputs are sorted by increasing mass.")
   parser.add_argument('filename', nargs='*', help='An outfit with ".xml" or ".mvx" extension, else will be ignored.')

   args = parser.parse_args()
   if args.github and args.color:
      args.github=args.color=False
      print >>stderr,"Ignored incompatible -g and -c."

   ign=[f for f in args.filename if not f.endswith(".xml") and not f.endswith(".mvx")]
   if ign!=[]:
      print >>stderr,'Ignored: "'+'", "'.join(ign)+'"'

   main([f for f in args.filename if f not in ign],args.github,args.color,args.nomax,args.sort)
