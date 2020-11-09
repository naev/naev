#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''Universe mapping tool for Naev.

Run this script from the root directory of your Naev source tree. It
reads the XML files in dat/ssys/ and outputs an SVG map to standard
output. Example usage:
	user@home:~/naev/$ jumpmap > map.svg

'''

# Copyright © 2012 Tim Pederick.
#
# Version 0.008
#   Modified by Benoît 'Mutos' ROBIN, 01/04/2020
#    - Python indentation using tabulations
#    - SVG indentation using tabulations
#    - Graphics attributes for better map readability
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Python 2 compatibility
from __future__ import division, print_function, unicode_literals

# Standard library imports.
from datetime import date
import sys

# Local imports.
from naevdata import SSystem, Asset
from dataloader import datafiles

# Debugger
import logging

def mapdata(ssystems, assets):
	'''Extract mappable data from a list of star systems.

	Keyword arguments:
		ssystems -- A sequence object containing the star systems to be
			mapped (instances of naevdata.SSystem).
		assets -- A sequence object containing the assets to be
			mapped (instances of naevdata.Asset).
	Returns:
		A 5-tuple containing:
		* the map boundaries (a 4-tuple of x-minimum, x-maximum,
		  y-minimum and y-maximum)
		* the system locations (a mapping object of system names to
		  coordinates, given as 2-tuple x-y pairs)
		* the system inhabited flags (a mapping object of system names to
		  coordinates, given as boolean)
		* the system asteroids flags - Hoshikaze-specific, for vanilla NAEV, will be empty
		  (a mapping object of system names to coordinates, given as boolean)
		* the two-way jumps between systems (a sequence object of
		  2-tuples holding the coordinates of the two ends, themselves
		  given as 2-tuple x-y pairs)
		* the one-way jumps between systems (as above, but note that
		  the two ends are ordered as origin then destination)

	'''
	syslocs = {}
	sysassets = {}
	sysinhabited = {}
	sysasteroids = {}
	jumps_by_name = {}
	jumps = []
	jumps_oneway = []
	xmin = xmax = ymin = ymax = 0

	# Extract the data.
	for ssys in ssystems:
		sys.stderr.write("\t\t" + ssys.name + "\n")
		# Note down the system name and location.
		syslocs[ssys.name] = ssys.pos

		# Note down if the system is inhabited
		#assets = ssys.assets
		sysinhabited[ssys.name] = False
		for asset in ssys.assets:
			result = set(assetInstance for assetInstance in assets if assetInstance.name==asset)
			#sys.stderr.write("\t\t\t" + asset + " : " + str(len(result)) + "\n")
			for assetInstance in result:
				pass
			#sys.stderr.write("\t\t\t\t" + assetInstance.name + "\n")
			sysinhabited[ssys.name] = sysinhabited[ssys.name] or (assetInstance.services.land!=None)
		sys.stderr.write("\t\t\tInhabited : " + str(sysinhabited[ssys.name]) + "\n")

		# Note down if the system has asteroids
		sysasteroids[ssys.name] = True
		sysasteroids[ssys.name] = ( ssys.hasAsteroids )
		sys.stderr.write("\t\t\tAsteroids : " + str(sysasteroids[ssys.name]) + "\n")

		# Note down if the system has non-asteroids assets
		sysassets[ssys.name]    = True
		sysassets[ssys.name]    = (len(ssys.assets) != 0)
		sys.stderr.write("\t\t\tAssets    : " + str(sysassets[ssys.name]) + "\n")

		# Note down any jumps it has. Ignore any that can't be entered from
		# here; they'll be recorded in the system at the other end.
		jumps_by_name[ssys.name] = list(dest for dest in ssys.jumps
										if not ssys.jumps[dest].exit_only)
		# Track the outermost systems.
		xmin = min(xmin, ssys.pos.x)
		xmax = max(xmax, ssys.pos.x)
		ymin = min(ymin, ssys.pos.y)
		ymax = max(ymax, ssys.pos.y)

	# Convert the jump data to a series of coordinates.
	for origin in jumps_by_name:
		for dest in jumps_by_name[origin]:
			if dest in jumps_by_name and origin in jumps_by_name[dest]:
				# Two-way jump.
				jumps.append((syslocs[origin], syslocs[dest]))
				# Don't duplicate jumps.
				jumps_by_name[dest].remove(origin)
			else:
				# One-way jump.
				jumps_oneway.append((syslocs[origin], syslocs[dest]))

	return ((xmin, xmax, ymin, ymax), syslocs, sysinhabited, sysasteroids, sysassets, jumps, jumps_oneway)

def makemap(ssystems, assets, margin=100, sys_size=5, inhabited_sys_size=15, ssystem_colour="white", asteroids_ssystem_colour="grey", assets_ssystem_colour="black", inhabited_ssystem_colour="green",
			jump_colour="grey", label_colour="black", label_font="serif",
			file=sys.stdout):
	'''Create an SVG map from a list of star systems.

	Keyword arguments:
		ssystems -- A sequence object containing the star systems to be
			mapped (instances of naevdata.SSystem).
		margin -- The margin width (in pixels) to put around the edges
			of the map. The default value is 10.
		sys_size -- The radius of the dot representing each star system.
			The default is 5.
		inhabited_sys_size -- The radius of the dot representing each star system.
			The default is 10.
		ssystem_colour, inhabited_ssystem_colour, asteroids_ssystem_colour, jump_colour, label_colour, label_font -- Control
			the appearance of the SVG output. The default appearance has
			grey empty systems, black asteroids-only systems, orange inhabited star systems, grey jumps, and labels in black serif.
		file -- A file-like object to output the SVG to. Defaults to
			standard output.

	'''
	(xmin, xmax, ymin, ymax), systems, sysinhabited, sysasteroids, sysassets, jumps, jumps_oneway = mapdata(ssystems, assets)
	# Pad the bounds of the map and convert to SVG viewBox specs.
	LABEL_SPACE = 200
	svg_bounds = (xmin - margin,
				  -ymax - margin,
				   xmax - xmin + 2 * margin + LABEL_SPACE,
				   ymax - ymin + 2 * margin)

	# Output the SVG file.
	print('<?xml version="1.0"?>')
	print('<svg xmlns="http://www.w3.org/2000/svg" version="1.2" '
		  'baseProfile="tiny" width="{2}px" height="{3}px" '
		  'viewBox="{0} {1} {2} {3}">'.format(*svg_bounds), file=file)
	print('  <title>NAEV Hyperspace Map - {}</title>'.format(date.today()),
		  file=file)
##	print('<!-- {} -->'.format((xmin, xmax, ymin, ymax)))

	# Style the map.
	print('	<defs>', file=file)
	print('		<marker id="arrow" orient="auto" viewBox="-1 -2 4 4"', file=file)
	print('		markerWidth="8" markerHeight="8">', file=file)
	print('			<path d="M 0,0 -1,-2 3,0 -1,2 Z" '
		  'fill="{}"/>'.format(jump_colour), file=file)
	print('		</marker>', file=file)
	print('		<style type="text/css">', file=file)
	print('			<![CDATA[', file=file)
	print('				g#jumps > line {{stroke: {}; '
		  'stroke-width: 1}}'.format(jump_colour), file=file)
	print('				g#jumps > line.oneway {stroke-dasharray: 2,1;', file=file)
	print('						   marker-mid: url(#arrow)}', file=file)
	print('				g#systems > g > circle {{stroke: black; '
		  'fill: {}}}'.format(ssystem_colour), file=file)
	print('				g#systems > g > circle.inhabited {{stroke: none; '
		  'fill: {}}}'.format(inhabited_ssystem_colour), file=file)
	print('				g#systems > g > circle.asteroids {{stroke: none; '
		  'fill: {}}}'.format(asteroids_ssystem_colour), file=file)
	print('				g#systems > g > circle.assets {{stroke: none; '
		  'fill: {}}}'.format(assets_ssystem_colour), file=file)
	print('				g#names > text {{stroke: none; '
		  'fill: {}; font-family: {}}}'.format(label_colour, label_font),
		  file=file)
	print('			]]>', file=file)
	print('		</style>', file=file)
	print('	</defs>', file=file)
	print(file=file)

	# Output the jumps first, so they're underneath the system markers.
	print('	<g id="jumps">', file=file)
	for jump in jumps:
		print('		<line x1="{}" y1="{}" x2="{}" y2= "{}" />'.format(jump[0].x, -jump[0].y,
													jump[1].x, -jump[1].y),
			  file=file)

	for jump in jumps_oneway:
		print('		<line class="oneway"', file=file)
		print('		  x1="{}" y1="{}" x2="{}" y2="{}"'
			  '/>'.format(jump[0].x,
						  -jump[0].y,
						  (jump[1].x - jump[0].x) // 2,
						  -(jump[1].y - jump[0].y) // 2),
			  file=file)
	print('	</g>', file=file)
	print(file=file)

	# Output the system markers in a middle layer.
	print('	<g id="systems">', file=file)
	for name in systems:
		x, y = systems[name].coords
		if sysinhabited[name]:
			# System is inhabited
			print('		<g>',file=file)
			print('			<circle class="inhabited" cx="{}" cy="{}" r="{}"/>'.format(x, -y, inhabited_sys_size),
				  file=file)
			print('		</g>',file=file)
		else:
			if sysassets[name]:
				# Uninhabited system has non-asteroids assets
				print('		<g>',file=file)
				print('			<circle class="assets" cx="{}" cy="{}" r="{}"/>'.format(x, -y, (sys_size+inhabited_sys_size)/2),
					  file=file)
				print('		</g>',file=file)
			else:
				if sysasteroids[name]:
					# Uninhabited system has only asteroids assets
					print('		<g>',file=file)
					print('			<circle class="asteroids" cx="{}" cy="{}" r="{}"/>'.format(x, -y, (sys_size+inhabited_sys_size)/2),
						  file=file)
					print('		</g>',file=file)
				else:
					# System has no assets at all
					print('		<g>',file=file)
					print('			<circle cx="{}" cy="{}" r="{}"/>'.format(x, -y, sys_size),
						  file=file)
					print('		</g>',file=file)
	print('	</g>', file=file)
	print(file=file)

	# Output the system names in a third layer on top of everything else.
	print('	<g id="names">', file=file)
	for name in systems:
		x, y = systems[name].coords
		if sysinhabited[name]:
			# System is inhabited
			print('		<g>',file=file)
			print('			<text x="{}" y="{}" font-size="{}">{}</text>'.format(x + 2 * inhabited_sys_size,
																-y + inhabited_sys_size,
																2 * inhabited_sys_size,
															  name),
				file=file)
			print('		</g>',file=file)
		else:
			if (sysassets[name] or sysasteroids[name]):
				# Uninhabited system has assets
				print('		<g>',file=file)
				print('			<text x="{}" y="{}" font-size="{}">{}</text>'.format(x + 2 * sys_size,
																  -y + sys_size,
																  4 * sys_size,
																  name),
					file=file)
				print('		</g>',file=file)
			else:
				# System has no assets at all
				print('		<g>',file=file)
				print('			<text x="{}" y="{}" font-size="{}">{}</text>'.format(x + 2 * sys_size,
																  -y + sys_size,
																  3 * sys_size,
																  name),
					file=file)
				print('		</g>',file=file)
	print('	</g>', file=file)
	print(file=file)

	# And we're done!
	print('</svg>', file=file)

def main():
	'''Generate an SVG map and print it to standard output.

	The data files are assumed to be in ./dat/ssys/, relative to the
	current path, so this should be run from the root of the Naev
	source directory.
		BR change : runs from naevroot/utils/starmap
		so data is in ../../dat/ssys
	'''
	sys.stderr.write("Begin\n")
	# Local variables
	naevRoot = '../..'
	logging.basicConfig(level=logging.DEBUG)
	ssystems = []
	assets   = []

	sys.stderr.write("\tLoading stellar systems\n")
	for ssysfile in datafiles('SSystems', naevRoot):
		# Parse each XML file into a SSystem object.
		try:
			ssystems.append(SSystem(ssysfile))
		except:
			print("Choked on '{}'".format(ssysfile), file=sys.stderr)
			raise
	sys.stderr.write("\t\t" + str(len(ssystems)) + " systems loaded\n")

	sys.stderr.write("\tLoading assets\n")
	for assetfile in datafiles('Assets', naevRoot):
		# Parse each XML file into a Asset object.
		try:
			assets.append(Asset(assetfile))
		except:
			print("Choked on '{}'".format(assetfile), file=sys.stderr)
			raise
	sys.stderr.write("\t\t" + str(len(assets)) + " assets loaded\n")

	sys.stderr.write("\tBuilding map\n")
	makemap(ssystems, assets)
	sys.stderr.write("Normal end\n")

if __name__ == '__main__':
	main()
