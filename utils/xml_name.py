#!/usr/bin/env python3

from subprocess import Popen, PIPE
from os import getpid

class sedserver:
   def __init__( self, cmd = [__file__.replace('.py', '.sed'), '-u'] ):
      self.proc = Popen(cmd, stdin=PIPE, stdout=PIPE)
      self.parent = getpid()

   def __del__( self ):
      self.proc.terminate()

   def __call__( self, inp ):
      line = inp.rstrip('\n') + '\n'
      line = line.encode('ISO-8859-1').lower()
      self.proc.stdin.write(line)
      self.proc.stdin.flush()
      return self.proc.stdout.readline().decode('ISO-8859-1').rstrip('\n')

   def mine( self ):
      return self.parent == getpid()

def end_xml_name():
   global xml_filter
   xml_filter = None

end_xml_name()

def xml_name( s ):
   global xml_filter
   if xml_filter is None or not xml_filter.mine():
      xml_filter = sedserver()
   return xml_filter(s)

if __name__ == '__main__':
   from sys import stdin
   for i in stdin:
      print(xml_name(i.strip()))
