#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

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

class LuaFetcher:

    def __init__(self, **args):
        """
        needs to be coded when I know how this class will be called
        """
        self.config = args
        self.luaScripts = list()

    def filenames_from_directory(self):
        """
        retrieve a list of files from the directory (the wild viking way)
        """

    def filenames_from_xml(self):
        """
        retrieve a list of files from the mission.xml (the quiet british way)
        """
        import xml.etree.ElementTree as ET
        file = os.path.normpath(self.config['datpath'] + '/mission.xml')
        missionxml = ET.parse(file))
        print 'Compiling lua scripts list ...'
        for mission in missionxml.findall('mission'):
            for lf in mission.findall('lua'):
                lf = os.path.join(os.abspath(missionspath + lf + '.lua'))
                self.luaScripts.append(missionspath+lf+'.lua' )
        print "\t\tDONE"
        return True

    def sanitizer(self):
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

        for file in self.luaScripts:
            try:
                line = open(file, 'rU').read()
                for match in search_cobj.finditer(line):
                    lineno, offset = lineNumber(file, match.start())
                    func = match.group('func')[:-1]
                    content = match.group('content')
                    # TODO check if content is in xml data
                for match in outfit_cobj.finditer(line):
            except IOError as errno, strerror:
                print "I/O error {0}: {1}".format(errno, strerror)
            except:
                raise

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
