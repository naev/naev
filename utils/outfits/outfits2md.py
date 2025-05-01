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

def main(args,gith=False,ter=False,noext=False,sortit=False,comb=False):
   if args==[]:
      return 0

   if comb:
      for i in args:
         if '+' in i:
            stderr.write('"+" incompatible with -C option\n')
            return 1
      args=[i+'+'+j for i in args for j in args]

   names=['']*len(args)
   L=[dict() for a in args]
   rang=dict()
   acc=[]

   for i in range(len(args)):
      if len(args[i].split('+'))==2:
         o,o2=args[i].split('+')
         o,o2=outfit(o.strip()),outfit(o2.strip())
         o.stack(o2)
      else:
         o=outfit(args[i])

      d=o.to_dict()
      for k,v in d.items():
         if type(v) == type((1.0,)):
            (val,_)=v
            s=str(int(v[0]))+'/'+str(int(v[1]))
         elif type(v) == type(1.0):
            val=v
            s=str(int(v))
         else:
            continue

         L[i][k]=s
         if k not in rang:
            rang[k]=(val,val)
            acc.append(k)
         else:
            (mi,ma)=rang[k]
            rang[k]=(min(mi,val),max(ma,val))

      names[i]=o.shortname()

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
      print(mklin(map(fmt,zip(t,length))))
   print(mklin([mk_pad(i,n) for i,n in enumerate(length)]))
   for r in Res:
      r=[r[0].replace("_"," ")]+[emph(k,rang[r[0]]) for k in r[1:]]
      print(mklin([fmt(x) if i>0 else lfmt(x) for i,x in enumerate(zip(r,length))]))
   return 0

if __name__ == '__main__':
   import argparse

   parser = argparse.ArgumentParser(
      usage=" %(prog)s  [-g|-c] [-n] [(-s SORT) | -S]  [filename ...]",
      description="By default, outputs text aligned markdown table comparing the outfits respective values."
   )
   parser.add_argument('-g', '--github', action='store_true', help="unaligned (therefore smaller) valid github md, for use in posts.")
   parser.add_argument('-c', '--color', action='store_true', help="colored terminal output. You can pipe to \"less -RS\" if the table is too wide.")
   parser.add_argument('-n', '--nomax', action='store_true', help="Do not emphasize min/max values." )
   parser.add_argument('-s', '--sort', help="inputs are sorted by their SORT key (mass if SORT is empty).")
   parser.add_argument('-S', '--sortbymass', action='store_true', help="Like -s mass." )
   parser.add_argument('-C', '--combinations', action='store_true', help="Does all the combinations." )
   parser.add_argument('filename', nargs='*', help='An outfit with ".xml" or ".mvx" extension, else will be ignored.')

   args = parser.parse_args()
   if args.github and args.color:
      args.github=args.color=False
      print("Ignored incompatible -g and -c.",file=stderr,flush=True)

   ign=[f for f in args.filename if not f.endswith(".xml") and not f.endswith(".mvx")]
   if ign!=[]:
      print('Ignored: "'+'", "'.join(ign)+'"',file=stderr,flush=True)

   if args.sort is None and args.sortbymass:
      args.sort=''

   if args.sort is None:
      sortby=False
   elif args.sort=='':
      sortby='mass'
   else:
      sortby=args.sort

   if sortby:
      print('sorted by "'+str(sortby)+'"',file=stderr,flush=True)

   main([f for f in args.filename if f not in ign],args.github,args.color,args.nomax,sortby,args.combinations)

