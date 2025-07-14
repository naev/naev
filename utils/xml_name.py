# python3

from subprocess import Popen, PIPE
proc = Popen([__file__.replace('.py', '.sed'), '-u'], stdin=PIPE, stdout=PIPE)

def xml_name( s ):
   line = s.strip().lower() + '\n'
   proc.stdin.write(line.encode('utf-8'))
   proc.stdin.flush()
   return proc.stdout.readline().decode('utf-8').strip()
