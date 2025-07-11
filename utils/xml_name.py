# python3

from PythonSed import Sed
from io import StringIO


sed = Sed()
#sed.regexp_extended = False
sed.load_script(__file__.replace('.py', '.sed'))


def xml_name( s ):
   input_f = StringIO(s.lower())
   return "\n".join(sed.apply(input_f, output = None)).rstrip('\n')
