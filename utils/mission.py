#!/usr/bin/env python

import os
import xml.etree.ElementTree as ET
import tempfile
from xml.sax.saxutils import escape


version=0.1


def indent(elem, level=0):
    i = "\n" + level*"  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indent(elem, level+1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i


print("Mission XML generator, version", version)

name = input("Mission name?: ")
lua = input("Lua file. e. g.  \"empire/collective/ec01\": ")
unique = input("unique? If yes, type anything, if no, leave empty: ")
done = input("done? If this mission requires the player to successfully finish another mission, then please write the name of that mission. If there are no requirements, then leave it empty.\n")
chance = input("The last two digits of the number in a <chance> tag determine the likelihood a mission will appear when a player lands on a planet. If the chance number is three digits long, the first digit determines how many times that probability is calculated on each landing. A number larger than 100 means the mission may appear more than once simultaneously.\n")
location = input("location? Can be either None, Computer, Bar, Outfit, Shipyard, Land or Commodity: ")
planets = []
while True:
    planet = input("planet? The name of a start planet. Leave empty if no more planets should be listed: ")
    if planet:
        planets.append(planet)
    else:
        break
factions = []
while True:
    faction = input("faction? The name of required faction of the planet. Leave empty if no more factions should be listed: ")
    if faction:
        factions.append(faction)
    else:
        break

cond=None
if input("Do you want to edit the <cond> in your $EDITOR? If yes, please type anything, if no then please leave empty: "):
    f = tempfile.NamedTemporaryFile()
    os.system("$EDITOR "+f.name)
    cond = f.read()
    f.close()

root = ET.Element("mission")
root.set("name", name)
e_lua = ET.SubElement(root, "lua").text=escape(lua)
if unique:
    ET.SubElement( ET.SubElement(root, "flags") , "unique")
avail = ET.SubElement(root, "avail")
if done:
    ET.SubElement(avail, "done").text=escape(done)
ET.SubElement(avail, "chance").text=escape(chance)
ET.SubElement(avail, "location").text=escape(location)
for faction in factions:
    ET.SubElement(avail, "faction").text=escape(faction)
for planet in planets:
    ET.SubElement(avail, "planet").text=escape(planet)
if cond:
    ET.SubElement(avail, "cond").text=escape(cond)

print("""The mission xml. Insert it right before '<missions />'.
===== THE XML =====""")
indent(root)
print(ET.tostring(root))
