#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:
"""
A sanity check tool to know where the crap is.

For usage information, run ``main.py --usage``

For licensing information, see the LICENSE file in this directory.
"""

import os, sys
from argparse import ArgumentParser

__version__="0.1"

def lineNumber(string, start):
    """
    Return the line number and offset from a regex match
    """
    lineno = string.count('\n', 0, start) + 1
    offset = start - string.rfind('\n', 0, start)
    return (lineno, offset)

class sanitizer:

    _errorstring = "Can not found element %(content)s for function %(func)s "\
                   "at line %(line)d offset %(offset)d"

    def __init__(self, **args):
        """
        needs to be coded when I know how this class will be called
        """
        self.config = args
        self.luaScripts = list()

        if self.config['use'] is 'missionxml':
            self.dirtyfiles_from_xml()
        elif self.config['use'] is 'rawfiles':
            self.dirtyfiles_from_directory()

    def dirtyfiles_from_directory(self):
        """
        retrieve a list of files from the directory (the wild viking way)
        """
        sys.stdout.write('Compiling file list like a wild viking ...')
        for root, dir, files in os.walk(self.config['missionpath']):
            for file in files:
                if file[-3:] is "lua":
                    realname = os.path.join(root,file)
                    self.luaScripts.append(realname)
        print '\t\tDONE'
        return True

    def dirtyfiles_from_xml(self):
        """
        retrieve a list of files from the mission.xml (the quiet british way)
        """
        import xml.etree.ElementTree as ET
        file = os.path.join(self.config['datpath'],'mission.xml')
        missionxml = ET.parse(file)
        sys.stdout.write('Compiling file list ...')
        for mission in missionxml.findall('mission'):
            for lf in mission.findall('lua'):
                lf = os.path.join(os.abspath(missionspath + lf + '.lua'))
                self.luaScripts.append(missionspath+lf+'.lua' )
        print "\t\tDONE"
        return True

    def dah_doctor(self):
        """
        That's the doctor, it detect wrong stuff but can never heal you.
        """
        from readers import fleet

        fleetdata = fleet(datpath=self.config['datpath'],
                          verbose=self.config['verbose'])

        rawstr = r"""
        (?P<func>
            pilot\.add\(|
            player\.addShip\(|
            addOutfit\(
        )['"](?P<content>[^"']+)"""

        # XXX not used
        #outfit_rawstr = r"""
        #    [:\.](?P<func>addOutfit)\(['"](?P<content>[^'"]+)
        #"""
        #outfit_cobj = re.compile(outfit_rawstr, re.VERBOSE| re.UNICODE)
        search_cobj = re.compile(rawstr, re.VERBOSE| re.UNICODE)

        entry = dict()

        print "Checking now ..."
        for file in self.luaScripts:
            sys.stdout.write("Processing file %(file)s..." % {'file': file})
            try:
                line = open(file, 'rU').read()
                for match in search_cobj.finditer(line):
                    lineno, offset = lineNumber(file, match.start())
                    info = dict(
                            ('lineno', lineno),
                            ('offset', offset),
                            ('func', match.group('func')[:-1]),
                            ('content', match.group('content'))
                    )

                    if info['func'] is 'pilot.add':
                        if not fleet.find(content):
                            print self._errorstring % info
                    elif info['func'] is 'addOutfit':
                        pass
                    elif info['func'] is 'player.addShip':
                        pass

            except IOError as errno, strerror:
                print "I/O error {0}: {1}".format(errno, strerror)
            except:
                raise
            print "\t\tDONE"

if __name__ == "__main__":

    parser = ArgumentParser(description="""
                Naev sanity check v%s.
                Please, don't go insane.
             """ %  __version__)
    parser.add_argument('--use', '-u',
                        choices=['missionxml','rawfiles'], default='missionxml',
                        help="""
                        Use the missionxml option to parse only the
                        actives files. Otherwise, the script will walk through
                        all the lua files.
                        """)

    parser.add_argument('--version', action='version',
                        version='%(prog)s '+__version__)
    parser.add_argument('--verbose', action='store_true', default=False)
    parser.add_argument('datpath',
                        nargs='?',
                        help='Path to naev/dat/ directory')

    args = parser.parse_args()

    datpath = os.path.abspath(args.datpath)
    missionpath = os.path.abspath(datpath + '/missions')
    insanity = sanitizer(datpath=datpath, missionpath=missionpath,
                         verbose=args.verbose,use=args.use)

    insanity.dah_doctor()
