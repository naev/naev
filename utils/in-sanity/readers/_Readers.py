# -*- coding: utf-8 -*-
# vim:set shiftwidth=4 tabstop=4 expandtab textwidth=80:

# Trying to get an etree.
try:
    from lxml import etree as ET
except ImportError:
    try:
        import xml.etree.ElementTree as ET
    except ImportError:
        print("Failed to import ElementTree from any known place")
        exit()


class readers:
    """
    Master object with verbosity support
    """

    _verbose=None
    xmlData=None

    def __init__(self, xmlFiles, verbose=False):
        """
        Set verbosity level and load xml file.
        The file must be valid.
        """
        self._verbose=verbose
        self.nameList=list()

        if self.xmlData is None:
            if type(xmlFiles) is not type(list()):
                self.xmlData = ET.parse(xmlFiles)
                return
            self.xmlData = list()
            for xfile in xmlFiles:
                self.xmlData.append(ET.parse( xfile ))

    def get_unused(self):
        """
        this method return a list containing all the unused stuff.
        use it wisely, it'll regenerate all the list. Could be quite time
        consuming with a lot of data.
        """
        tmp = self.nameList
        try:
            for name in self.used:
                # XXX TODO WARNING : this is a 'fix'. But I need to understand
                # why that name could not be in nameList
                if name in tmp:
                    tmp.remove(name)
            for name in self.unknown:
                if name in tmp:
                    tmp.remove(name)
        except ValueError:
            print('ValueError: %s not in %s used list' %
                        (name, self._componentName))
            print('Debug info:')
            print("\ntmp: %s" % (tmp))
            print("\nNameList: %s" % (self.nameList))
            print("\nUsed list: %s" % (self.used))

            raise
        return tmp

    def show_unused(self):
        if len(self.unknown) > 0:
            print('\nProbably not used %s name:' % self._componentName)
            print('\n'.join(self.unknown))
        tmp = self.get_unused()
        if len(tmp) > 0:
            print('\nUnused %s name:' % self._componentName)
            print('\n'.join(tmp))

    def set_unknown(self, name):
        """
        Set the name in an unknown status.
        Meaning it is probably used by a lua script, but this tool can't be
        certain (i.e. name used in a variable).
        """
        if name in self.nameList and name not in self.used:
            if name not in self.unknown:
                self.v("SET ''%s`` as UNKNOWN" % name)
                self.unknown.append(name)

    def v(self, msg):
        if self._verbose:
            print(msg)
        else:
            pass


