#!/usr/bin/python
import argparse
import re

def parse_api( filename ):
	with open( filename, 'r' ) as f:
		data = f.read()

	result = re.findall("static const luaL_Reg.*?\[\] = {(.+?)};", data, re.S)
	result = re.findall( "\".*\"", result[0] )
	funclist = []
	for r in result:
		funclist += [r.replace("\"","")]

	return funclist

def generate_api( modname, filename ):
	api = parse_api( filename )
	api.sort()

	apilist = ""
	for a in api:
		apilist += f"         {a} = {{}},\n"

	return f"""return read_globals = {{
   {modname} = {{
      fields = {{
{apilist}      }},
   }},
}}"""

if __name__ == "__main__":
	parser = argparse.ArgumentParser( description='Generates API for luacheckrc from a C source file.' )
	parser.add_argument('modname',  metavar='MODNAME',  type=str, help='Name of the module to generate.')
	parser.add_argument('filename', metavar='FILENAME', type=str, help='Name of the file name to parse.')
	args = parser.parse_args()
	print( generate_api( args.modname, args.filename ) )

#api = generate_api( "news", "/home/ess/naev/src/nlua_news.c" )
