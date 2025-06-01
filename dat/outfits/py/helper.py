import sys
import xmltodict

INPUT = sys.argv[1]
OUTPUT = sys.argv[2]

def read():
    with open(INPUT,'r') as f:
        return xmltodict.parse( f.read() )

def write( data ):
    with open(OUTPUT,'w') as f:
        f.write( xmltodict.unparse( data, pretty=True ) )

def mul_i( d, f, v ):
    d[f] = str(round(float(d[f])*v))

def add_i( d, f, v ):
    d[f] = str(int(d[f])+v)

def mul_f( d, f, v ):
    d[f] = str(float(d[f])*v)
