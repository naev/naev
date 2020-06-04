#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''Naev data file tools.'''

# Copyright © 2012 Tim Pederick.
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
import xml.dom.minidom
import sys

# Shortcut function to extract the text content from an element.
nodetext = lambda elem: ''.join(c.data for c in elem.childNodes
								if c.nodeType == c.TEXT_NODE)

class Coords(object):
	'''Represents an x-y coordinate pair.

	Instance attributes:
		x, y -- The coordinate values.
		coords -- A shorthand for both coordinates as a 2-tuple.

	'''
	def __init__(self, x=None, y=None):
		'''Extract x-y coordinates from their XML representation.

		Naev XML data files represent coordinate pairs as a <pos>
		element with <x> and <y> children. This data can be extracted
		by passing the <pos> element. Alternatively, both the x and y
		coordinates may be supplied directly.

		If no arguments are supplied, the coordinates will both be None.

		Positional arguments:
			pos -- An XML element whose <x> and <y> children hold the
				respective coordinate values; OR
			x, y -- The coordinate values given directly.

		'''
		if x is not None and y is None:
			# Just the one argument. Treat it as an XML element with <x> and
			# <y> children whose contents are the respective coordinates.
			self.x = float(nodetext(x.getElementsByTagName('x')[0]))
			self.y = float(nodetext(x.getElementsByTagName('y')[0]))
		else:
			# Both arguments, or neither.
			self.x = x if x is None else float(x)
			self.y = y if y is None else float(y)
	@property
	def coords(self):
		return (self.x, self.y)


class Jump(Coords):
	'''Represents a jump point from one system to another.

	Instance attributes:
		x, y, coords -- The jump point position, inherited from Coords.
		hide -- The "hide" value of this jump point, which controls its
			visibility in-game.
		exit_only -- Whether or not this jump point forbids entry.

	'''
	def __init__(self, pos, hide=1.25, exit_only=False, dest='ignored'):
		'''Construct the jump point.

		Keyword arguments:
			pos -- The location of this jump point, as an x-y coordinate
				pair.
			hide, exit_only -- As the instance attributes. If omitted,
				they default to 1.25 and False, respectively.
			dest -- This argument is not used and may be omitted.

		'''
		super(Jump, self).__init__(*pos)
		self.hide = float(hide)
		self.exit_only = bool(exit_only)


class Nebula(object):
	'''Represents the nebula presence in a star system.

	Instance attributes:
		density -- The density of the nebula in this system.
		volatility -- How damaging the nebula is to ships in the system.

	'''
	def __init__(self, density=0.0, volatility=0.0):
		'''Create the nebula presence data.

		Keyword arguments:
			density, volatility -- As the instance attributes. Both
				will default to 0.0 if omitted.

		'''
		self.density = float(density)
		self.volatility = float(volatility)


class Presence(object):
	'''Represents the faction holding a planet, station, or other asset.

	Instance attributes:
		faction -- The name of the faction whose presence is described.
		value -- A numeric index indicating the magnitude or strength of
			this faction's presence.
		range_ -- How far the faction presence from this system will
			spill out into neighbouring systems.

	'''
	def __init__(self, faction=None, value=100.0, range_=0.0):
		'''Create the faction presence data.

		Keyword arguments:
			faction, value, range_ -- As the instance attributes. All
				arguments are optional, and default to None, 100.0, and
				0.0, respectively.

		'''
		self.faction = faction
		self.value = float(value)
		self.range = int(range_)


class Services(object):
	'''Represents the services available on a planet or station.

	Instance attributes:
		bar -- A string describing the local spaceport bar, or None if
			no bar is present.
		commodities -- A set of commodities available for trade at this
			location, or None if commodity trading is not available.
		land -- A string detailing landing permissions ('any' if landing
			is unrestricted), or None if landing is not possible.
		missions, outfits, refuel, shipyard, blackmarket -- Whether or not these
			services (mission computer, ship outfitting, refuelling, and
			buying and selling of ships, blackmarket) are available at this location.

	'''
	def __init__(self, bar=None, commodity=None, land=None, missions=False,
				 outfits=False, refuel=False, shipyard=False, blackmarket=False):
		'''Create the service availability data.

		Keyword arguments:
			bar, land -- As the instance attributes. Default to None.
			commodity -- As the instance attribute commodities. The
				argument name differs because it is "commodity" in the
				XML files, but "commodities" is a more apt label for
				what it represents. The default is None.
			missions, outfits, refuel, shipyard -- As the instance
				attributes. Default to False.

		'''
		self.bar = bar # None, or a string describing the bar.
		# Note that the argument name is "commodity", to match the XML tag,
		# but the instance attribute is named "commodities".
		self.commodities = None if commodity is None else set(commodity)
		self.land = land # None, or a string detailing who can land.
		self.missions = bool(missions)
		self.outfits = bool(outfits)
		self.refuel = bool(refuel)
		self.shipyard = bool(shipyard)
		self.blackmarket = bool(blackmarket)


class Asset(object):
	'''Represents a planet, moon, station, or virtual holding.

	A "virtual" asset represents a faction's stake in a system without
	being mapped to a physical location. All of the other asset types
	represent a concrete holding of a faction -- or of no faction, in
	the case of an unsettled world or abandoned station.

	Instance attributes:
		description -- A string describing the asset.
		gfx -- A mapping object of graphics pertaining to this asset.
			The values are image filenames, and the keys are the images'
			purposes (e.g. "space" for the asset's appearance from
			space, "exterior" for a panorama of the planet or station
			upon landing).
		hide -- A numeric value controlling the asset's visibility.
		name -- The name of the asset.
		population -- The population of the asset.
		pos -- The location of the asset. An instance of Coords.
		presence -- The faction that holds this asset. An instance of
			Presence.
		services -- The services available at this location. An instance
			of Services.
		virtual -- Whether or not this asset is virtual, as described
			above.
		world_class -- A broad classification of this world's physical
			characteristics. All stations are class 0, while planets
			and moons have letter designations.

	'''
	def __init__(self, filename):
		'''Construct the asset from an XML file.

		Keyword arguments:
			filename -- The filename of the XML asset data. If None, a
				virtual asset without any interesting attributes is
				created.

		'''
		if filename is None:
			# Create an empty, virtual asset.
			self.description = ''
			self.gfx = {}
			self.hide = 0.0
			self.population = 0
			self.pos = Coords()
			self.presence = Presence()
			self.services = Services()
			self.virtual = True
			self.world_class = None
		else:
			# Read the asset from the given file.
			with open(filename) as f:
				# Grab the elements we want.
				doc = xml.dom.minidom.parse(f)
				# Don't try and index into any of these NodeLists yet (they
				# may not exist).
				general = doc.getElementsByTagName('general')
				gfx = doc.getElementsByTagName('GFX')
				pos = doc.getElementsByTagName('pos')
				presence = doc.getElementsByTagName('presence')
				techs = doc.getElementsByTagName('tech')
				virtual = doc.getElementsByTagName('virtual')

				self.name = doc.documentElement.getAttribute('name')

				# Set the asset's position, graphics, and virtual-ness.
				self.pos = (Coords() if not pos else Coords(pos[0]))
				self.gfx = ({} if not gfx else
							dict((child.tagName, nodetext(child))
								  for child in gfx[0].childNodes
								  if child.nodeType == child.ELEMENT_NODE))
				self.virtual = bool(virtual)

				# Extract the faction presence data.
				pres_data = ({} if not presence else
							 dict(('range_' if child.tagName == 'range'
								   else child.tagName, nodetext(child))
								  for child in presence[0].childNodes
								  if child.nodeType == child.ELEMENT_NODE))
				self.presence = Presence(**pres_data)

				# Extract the list of technologies, each of which is an <item>
				# under the <tech> element.
				self.techs = set()
				if techs:
					for tech in techs[0].getElementsByTagName('item'):
						self.techs.add(nodetext(tech))

				# Extract the <general> information. Initialise each one just
				# in case it (or the whole of <general>) is absent.
				bar_desc = None
				commodities = None
				self.description = ''
				self.hide = 0.0
				self.population = 0
				self.services = None
				self.world_class = None

				# Do we even have a <general> element?
				if general:
					for child in general[0].childNodes:
						# We're only interested in child elements.
						if child.nodeType != child.ELEMENT_NODE:
							continue

						if child.tagName == 'bar':
							bar_desc = nodetext(child)
						elif child.tagName == 'description':
							self.description = nodetext(child)
						elif child.tagName == 'commodities':
							# Get the set of child <commodity> nodes' content.
							c_nodes = child.getElementsByTagName('commodity')
							commodities = set(nodetext(c) for c in c_nodes)
						elif child.tagName == 'services':
							# Find which service type elements are present and
							# get their content (if any -- most will be empty).
							services = {}
							for service in child.childNodes:
								if service.nodeType != service.ELEMENT_NODE:
									continue
								services[service.tagName] = nodetext(service)

							# An empty <land> tag means anyone can land.
							try:
								if services['land'] == '':
									services['land'] = 'any'
							except KeyError:
								# An absent <land> tag means no-one can land.
								pass

							# Build the Services object.
							self.services = Services(**services)
						else:
							# Everything else is just defined by its content.
							try:
								child_type = {'population': int,
											  'hide': float}[child.tagName]
							except KeyError:
								child_type = str
							self.__setattr__('world_class'
											 if child.tagName == 'class'
											 else child.tagName,
											 child_type(nodetext(child)))
				# Finalise the list of services.
				if self.services is None:
					self.services = Services()
				else:
					# Put the bar description into the services -- if there is
					# a bar there! If there isn't, the text is discarded.
					if bar_desc is not None and self.services.bar is not None:
						self.services.bar = bar_desc
					# Put the commodities list into the services -- again, only
					# if there are commodities traded here.
					if (commodities is not None and
						self.services.commodities is not None):
						self.services.commodities = commodities


class SSystem(object):
	'''Represents a star system.

	Instance attributes:
		assets -- A set of Asset instances present in this system.
			TODO: Actually, at the moment this is just a set of asset
			names, not instances.
		interference -- The prevailing sensor interference from any
			background radiation or nebula presence in this system.
		jumps -- A mapping object pairing destination system names with
			Jump instances. Note that the jump point coordinates may be
			omitted if the jump is "autopositioned", meaning it is
			located at a point calculated from the two systems' relative
			positions in space.
		name -- The name of this system.
		nebula -- A Nebula instance for nebula presence in this system.
		pos -- A Coords object giving this system's location in space.
		radius -- The size of this system for the purposes of asset
			placement, autopositioning jump points, and the in-game map.
		stars -- The density of stars in this system's background.
			TODO: Check this! It's just my guess as to what this value
			is supposed to represent.

	'''
	def __init__(self, filename=None):
		'''Construct the star system from an XML file.

		Keyword arguments:
			filename -- The filename of the XML system data. If omitted,
				a zero-size system without any interesting attributes is
				created.

		'''
		if filename is None:
			# Create an empty star system.
			self.assets = set()
			self.interference = 0.0
			self.jumps = {}
			self.name = ''
			self.nebula = Nebula()
			self.pos = Coords()
			self.radius = 0.0
			self.stars = 0
			self.hasAsteroids = False
		else:
			# Read the star system from the given file.
			with open(filename) as f:
				# Grab the elements we want.
				doc = xml.dom.minidom.parse(f)
				assets = doc.getElementsByTagName('asset')
				general = doc.getElementsByTagName('general')[0]
				jumps = doc.getElementsByTagName('jump')
				pos = doc.getElementsByTagName('pos')[0]

				self.name = doc.documentElement.getAttribute('name')
				self.hasAsteroids = False

				# Get the system's position, assets (planets and stations and
				# such), and jump points.
				self.pos = Coords(pos)
				self.assets=set()
				self.assetsInstances=set()
				for asset in assets:
					if not nodetext(asset).find("Asteroids Cluster"):
						# BR for Hoshikaze : eliminate Asteroids Clusters from assets list
						self.hasAsteroids=True
						#sys.stderr.write("hasAsteroids : " + self.name + " : " + nodetext(asset) + "\n")
					else:
						self.assets.add(nodetext(asset))
						#sys.stderr.write("hasAssets    : " + self.name + " : " + nodetext(asset) + "\n")

				self.jumps = {}
				for jump in jumps:
					autopos = jump.getElementsByTagName('autopos')
					exit_only = jump.getElementsByTagName('exitonly')
					# We don't index the NodeList of <pos> tags yet because it
					# might be empty, if <autopos/> is present.
					pos = jump.getElementsByTagName('pos')
					jump_pos = ((None, None) if autopos
								else (pos[0].getAttribute('x'),
									  pos[0].getAttribute('y')))
					hide = nodetext(jump.getElementsByTagName('hide')[0])

					self.jumps[jump.getAttribute('target')] = Jump(jump_pos,
																   hide,
																   exit_only)

				# Extract the <general> information. Initialise each one just
				# in case it's missing.
				self.interference = 0.0
				self.nebula = None
				self.radius = 0.0
				self.stars = 0
				for child in general.childNodes:
					# We're only interested in child elements.
					if child.nodeType != child.ELEMENT_NODE:
						continue

					content = nodetext(child)
					if child.tagName == 'nebula':
						# The <nebula> tag has a couple of bits of info.
						self.nebula = Nebula(content,
											 child.getAttribute('volatility'))
					else:
						# Everything else is just a single piece of content.
						# <stars> is an int; the rest are floats.
						child_type = int if child.tagName == 'stars' else float
						self.__setattr__(child.tagName, child_type(content))
				# And just in case <nebula> was absent...
				if self.nebula is None:
					self.nebula = Nebula()
