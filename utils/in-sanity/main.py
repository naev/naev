#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

__doc__ = """
A sanity check tool to know where the crap is.

For usage information, run ``main.py --usage``

For licensing information, see the LICENSE file in this directory.
"""

import os, sys
from argparse import ArgumentParser
import re
from types import *

__version__="0.2"

def lineNumber(string, start):
    """
    Return the line number and offset from a regex match
    """
    lineno = string.count('\n', 0, start) + 1
    offset = start - string.rfind('\n', 0, start)
    return (lineno, offset)

class sanitizer:

    _errorstring = "Can not find element ``%(content)s'' for function "    \
                   "%(func)s at line %(lineno)d offset %(offset)d of file " \
                   "%(file)s"

    def __init__(self, **args):
        """
        Initialize the whole parser : create a list of lua files.
        """
        self.config = args
        self.luaScripts = list()

        if 'datpath' not in self.config.keys():
            self.config['datpath'] = os.path.join(self.config['basepath'],
                                                  'dat/')

        if self.config['use'] == 'xml':
            self.dirtyfiles_from_xml()
        elif self.config['use'] == 'rawfiles':
            self.dirtyfiles_from_directory()

        print('Compiling script files...',end='      ')
        for root, dir, files in os.walk(self.config['basepath']+'/scripts/'):
            self.addLuaFile(files, root)
        print('DONE')

        print('Compiling AI Spawn files...', end='      ')
        for root, dir, files in os.walk(self.config['datpath']+'factions/spawn/'):
            self.addLuaFile(files, root)
        print('DONE')

    def addLuaFile(self, files, root):
        """
        Add a lua file in the bucket. Do nothing if the given file do not end
        with 'lua'
        """
        for filename in files:
            if filename[-3:] == "lua":
                realname = os.path.join(root, filename)
                self.luaScripts.append(realname)

    def dirtyfiles_from_directory(self):
        """
        retrieve a list of files from the directory (the wild viking way)
        """
        for name in "mission", "event":
            print('Compiling ' + name + ' file list like a wild viking ...', end='       ')
            for root, dir, files in os.walk("{0}/{1}s".format(luapath, name)):
                self.addLuaFile(files, root)
            print('DONE')
        return True

    def dirtyfiles_from_xml(self):
        """
        retrieve a list of files from the mission and event XML (the quiet british way)
        """
        import xml.etree.ElementTree as ET
        for name in "mission", "event":
            print('Compiling {0} file list ...'.format(name), end='      ')
            file = os.path.join(self.config['basepath'], "{0}/{1}.xml".format(luapath, name))
            naevxml = ET.parse(file)
            for mission in naevxml.findall(name):
                for lf in mission.findall('lua'):
                    lf = os.path.join("{0}/{1}s".format(luapath, name), lf.text + '.lua')
                    self.luaScripts.append(lf)
            print('DONE')
        return True

    def dah_doctor(self):
        """
        That's the doctor, it detect wrong stuff but can never heal you.

        Pretty big function indeed :
        1/ Will do a call to undiff, then initialize the preprocessing stuff.
        2/ Then, initialize each reader with the appropriate config
        3/ Then, parse each lua file searching for function and content
        """
        from readers import fleet
        from readers import ship
        from readers import outfit
        from readers import unidiff
        from readers.preprocessing import tech

        udata = unidiff(datpath=self.config['datpath'],
                        verbose=self.config['verbose'])

        # This will do some preprocessing check in the xml files
        print('Preprocess valitation :')
        otech = tech(unidiffobj=udata,**self.config)

        # TODO: must be called when needed
        fleetdata = fleet(datpath=self.config['datpath'],
                        verbose=self.config['verbose'])
        shipdata = ship(datpath=self.config['datpath'],
                        verbose=self.config['verbose'],
                        tech=otech, fleetobj=fleetdata)
        outfitdata = outfit(datpath=self.config['datpath'],
                        verbose=self.config['verbose'], tech=otech)

        rawstr = r"""
        (?P<func>
            pilot\.add\(|
            pilot\.addRaw\(|
            player\.addShip\(|
            addOutfit\(|
            diff\.apply\(|
            scom\.addPilot\()\s* (?P<hackery>pilots,|
        )\s*"(?P<content>[^"]+)"""

        search_cobj = re.compile(rawstr, re.VERBOSE| re.UNICODE)

        entry = dict()
        errors = list()

        # XXX This variable will stock all the lua files. This could lead to a
        # massive memory consumption.
        line = dict()

        print("Blind check now ...")
        for file in self.luaScripts:
            if self.config['verbose']:
                print("Processing file {0}...".format(file), end='       ')
            if len(errors) > 0:
                for error in errors:
                    print(error, file=sys.stderr)
                errors = list()

            try:
                line[file] = open(file, 'rU').read()
                haserror=False
                for match in search_cobj.finditer(line[file]):
                    lineno, offset = lineNumber(line[file], match.start())
                    info = dict(
                            lineno=lineno,
                            offset=offset,
                            func=match.group('func')[:-1],
                            content= match.group('content'),
                            file=file
                    )

                    if info['func'] == 'pilot.add':
                        if not fleetdata.find(info['content']):
                            haserror=True
                    if info['func'] == 'scom.addPilot':
                        if not fleetdata.find(info['content']):
                            haserror=True
                    if info['func'] == 'pilot.addRaw':
                        if not shipdata.find(info['content']):
                            haserror=True
                    if info['func'] == 'addOutfit':
                        if not outfitdata.find(info['content']):
                            haserror=True
                    if info['func'] == 'player.addShip':
                        if not shipdata.find(info['content']):
                            haserror=True
                    if info['func'] == 'diff.apply':
                        if not udata.find(info['content']):
                            haserror=True

                    if haserror:
                        errors.append(self._errorstring % info)
                        haserror=False

            except IOError as error:
                print("I/O error: {0}".format(error), file=sys.stderr)
            except:
                raise

            if self.config['verbose']:
                print("DONE")

        print('Verifying ...')
        unused_data = dict()

        # makes sure that only category with unused stuff get listed
        tocheck = ((fleetdata, 'fleet'), (udata,'unidiff'),
                   (shipdata, 'ship'), (outfitdata, 'outfit'))
        for obj, key in tocheck:
            tmp = obj.get_unused()
            if len(tmp) > 0:
                unused_data.update({key: tmp})
        del(tmp)

        if len(unused_data) > 0:
           # Create the regex
            mcobj = dict()
            for (category, data) in unused_data.items():
                regex = r"""
                (?P<{0}>%s)
                """.format(category)
                rname = ''
                uniqList = list()
                for name in data:
                    name = name.replace(' ', "\s")
                    rname = rname + name + '|'
                    if name not in uniqList:
                        uniqList.append(name)
                regex = regex % rname[:-1]
                mcobj.update({category: re.compile(regex, re.VERBOSE| re.UNICODE)})

            # For each file, run each regex
            # If an item is found, that item is set to the unknown status
            for (file, content) in line.items():
                for (category, cobj) in mcobj.items():
                    for match in cobj.finditer(content):
                        groups = match.groupdict()
                        for (obj, key) in tocheck:
                            if key == category:
                                obj.set_unknown(groups[category])

        outfitdata.showMissingTech()
        shipdata.showMissingTech()

        if self.config['show_unused']:
            # XXX there is no real needs to use these two because of
            # ''showMissingTech`` called earlier, unless something is tagged as
            # 'UNKNOWN'
            outfitdata.show_unused()
            shipdata.show_unused()
            fleetdata.show_unused()
            udata.show_unused()

if __name__ == "__main__":

    parser = ArgumentParser(description="""
                Naev sanity check v%s.
                Please, don't go insane.
             """ %  __version__)
    parser.add_argument('--use', '-u',
                        choices=['xml','rawfiles'], default='xml',
                        help="""
                        Use the rawfiles option to run into the directory like
                        a wild viking. Otherwise, the script will use the active
                        mission list to load the file list.
                        """)

    gfleet = parser.add_argument_group('Fleets')
    gfleet.add_argument('--show-unused', action='store_true', default=False,
                        help='Show unused fleets from the xml files')

    parser.add_argument('--version', action='version',
                        version='%(prog)s '+__version__)
    parser.add_argument('--verbose', action='store_true', default=False)
    parser.add_argument('basepath',
                        help='Path to naev/ directory')

    args = parser.parse_args()

    basepath = os.path.abspath(args.basepath)
    luapath = os.path.abspath(basepath + '/dat/')
    insanity = sanitizer(basepath=basepath, luapath=luapath,
                         verbose=args.verbose,use=args.use,
                         show_unused=args.show_unused)

    insanity.dah_doctor()
