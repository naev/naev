#!/usr/bin/env python3

from sys import stderr
from outfit import outfit
from outfit import LOWER_BETTER

def transpose(M):
   N=max(map(len,M))
   M=[t+['']*(N-len(t)) for t in M]
   return list(zip(*(tuple(M))))

getfloat=lambda s:float(s.split('/')[0])

def keyfunc(s):
   def key(o):
      try:
         return getfloat(o[s])
      except:
         return None
   return key

def nfmt(n):
   f=float(n)
   if f==round(f):
      f=int(f)
   return str(f)

def main(args,gith=False,ter=False,noext=False,sortit=False,autostack=False,comb=False):
   if args==[]:
      return 0

   sec_args=[]
   if comb or autostack:
      for i in args:
         if i[:2]=='2x' or i[-2:]=='x2':
            stderr.write('"2x"')
         elif '+' in i:
            stderr.write('"+"')
         else:
            if outfit(i).can_sec():
               sec_args.append(i)
            continue
         stderr.write(' incompatible with -A/-C options\n')
         return 1

   if comb:
      args=[i+'+'+j for i in args for j in sec_args]
   elif autostack:
      args=args+['2x'+i for i in sec_args]

   rang=dict()
   L=[]
   names=[]
   acc=[]

   for i in range(len(args)):
      if args[i][:2]=='2x' or args[i][-2:]=='x2':
         args[i]=args[i][2:] if args[i][:2]=='2x' else args[i][:-2]
         args[i]=args[i]+'+'+args[i]

      if len(args[i].split('+'))==2:
         o,o2=args[i].split('+')
         o,o2=outfit(o.strip()),outfit(o2.strip())
         if not o.can_stack(o2):
            continue
         o.stack(o2)
      else:
         o=outfit(args[i])
         if not o.can_alone():
            continue

      d=o.to_dict()
      for k,v in d.items():
         if type(v) == type((1.0,)):
            (val,_)=v
            s=nfmt(v[0])+'/'+nfmt(v[1])
         elif type(v) == type(1.0):
            val=v
            s=nfmt(v)
         else:
            continue

         d[k]=s
         if k not in rang:
            rang[k]=(val,val)
            acc.append(k)
         else:
            (mi,ma)=rang[k]
            rang[k]=(min(mi,val),max(ma,val))

      names.append(o.shortname())
      L.append(d)

   for i,t in rang.items():
      (m,M)=t
      if i in LOWER_BETTER:
         rang[i]=(M,m)
      if noext:
         rang[i]=(None,None)

   if sortit:
      for i,o in enumerate(L):
         o['name']=names[i]
      L.sort(key=keyfunc(sortit))
      for i,o in enumerate(L):
         names[i]=o['name']

   Res=[[k]+[l[k] if k in l else '' for l in L] for k in acc]
   names=['']+names
   length=[4]*len(names) # :--- at least 3 - required

   if ter:
      Sep,SepAlt,Lm,Rm,LM,RM='\033[30;1m|\033[0m','\033[34m|\033[0m',"\033[31m","\033[37m","\033[32m","\033[37m"
      mk_pad =lambda i,n:"\033[34m"+n*'-'+"\033[0m"
      leng=lambda x:len(x)- (10 if x!='' and x[0]=='\033' else 0)
   else:
      Sep,SepAlt,Lm,Rm,LM,RM='|','|',"_","_","**","**"
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

   mklinsep=lambda sep:lambda L:sep+' '+(' '+sep+' ').join(L)+' '+sep
   mklinalt=lambda alt:mklinsep(SepAlt) if alt else mklinsep(Sep)
   mklin=mklinalt(False)
   fmt=lambda t:(t[1]-leng(t[0]))*' '+t[0]
   lfmt=lambda t:t[0]+(t[1]-leng(t[0]))*' '

   def emph(s,m):
      (mi,ma)=m
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
      acc=mklin(map(fmt,zip(t,length)))
      if ter:
         acc=' '+acc[len(Sep):]
      print(acc)
   print(mklinalt(True)([mk_pad(i,n) for i,n in enumerate(length)]))

   count = 0
   for r in Res:
      r=[r[0].replace("_"," ")]+[emph(k,rang[r[0]]) for k in r[1:]]
      print(mklinalt(count%3==2)([fmt(x) if i>0 else lfmt(x) for i,x in enumerate(zip(r,length))]))
      count+=1
   return 0

if __name__ == '__main__':
   import argparse

   parser = argparse.ArgumentParser(
      usage=" %(prog)s  [-g|-c] [-n] [(-s SORT) | -S]  [filename ...]",
      formatter_class=argparse.RawTextHelpFormatter,
      description="By default, outputs text aligned markdown table comparing the outfits respective values."
   )
   parser.add_argument('-g', '--github', action='store_true', help="unaligned (therefore smaller) valid github md, for use in posts.")
   parser.add_argument('-c', '--color', action='store_true', help="colored terminal output. You can pipe to \"less -RS\" if the table is too wide.")
   parser.add_argument('-n', '--nomax', action='store_true', help="Do not emphasize min/max values." )
   parser.add_argument('-s', '--sort', help="inputs are sorted by their SORT key.")
   parser.add_argument('-S', '--sortbymass', action='store_true', help="Like -s mass." )
   parser.add_argument('-A', '--autostack', action='store_true', help="Adds 2x outfits" )
   parser.add_argument('-C', '--combinations', action='store_true', help="Does all the combinations." )
   parser.add_argument('filename', nargs='*', help="""An outfit with ".xml" or ".mvx" extension, else will be ignored.
Can also be two outfits separated by \'+\', or an outfit prefixed with \'2x\' or suffixed with \'x2\'.""")

   args = parser.parse_args()
   if args.github and args.color:
      args.github=args.color=False
      print("Ignored incompatible -g and -c.",file=stderr,flush=True)

   if args.autostack and args.combinations:
      print("-A subsumed by -C",file=stderr,flush=True)
      args.autostack=False

   ign=[f for f in args.filename if not f.endswith(".xml") and not f.endswith(".mvx")]
   if ign!=[]:
      print('Ignored: "'+'", "'.join(ign)+'"',file=stderr,flush=True)

   if args.sort is None and args.sortbymass:
      args.sort='mass'

   if args.sort is None:
      sortby=False
   else:
      sortby=args.sort

   if sortby:
      print('sorted by "'+str(sortby)+'"',file=stderr,flush=True)

   main([f for f in args.filename if f not in ign],args.github,args.color,args.nomax,sortby,args.autostack,args.combinations)
else:
   raise Exception("This module is only intended to be used as main.")
