#!/usr/bin/env python
"""
Copyright (C) 2008 by Edgar Simo Serra
bobbens@gmail.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

import sys
from xml.dom import minidom
import data


def main():
   sys.argv.remove(sys.argv[0]) # Remove self
   for file in sys.argv:
      fp = open(file, 'rw+')
      doc = minidom.parse( file )
      data.write_proper_xml( fp, doc )
      doc.unlink()
      fp.close()

if __name__ == "__main__":
   main()
