#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''Naev data file loaders.

The objects in this library abstract away the location of the various
Naev data files, so that if they change in future only this library
needs to be updated to match.

'''

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
import subprocess
import glob
import os

# The main Naev data directory.
DATA_ROOT = 'dat'

# Locations for each type of data. Values in this mapping are 2-tuples, holding
# the directory and a filename pattern.
DATA_LOCS = {'SSystems': ('ssys', '*.xml'),
			 'Assets': ('assets', '*.xml')}

def naev_version(naevroot=None):
	'''Get the currently declared Naev version.'''
	if naevroot is None:
		naevroot = os.curdir
	with open(os.path.join(naevroot, 'VERSION')) as v:
		return v.read().strip()

def naev_revision(naevroot=None):
	'''Get the current Naev version control revision identifier.'''
	if naevroot is not None:
		oldwd = os.getcwd()
		os.chdir(naevroot)
	revision = subprocess.check_output(('git', 'rev-parse', '--short',
										'--verify', 'HEAD')).decode()
	if naevroot is not None:
		os.chdir(oldwd)
	return revision

def datafiles(dataset, naevroot=None):
	'''Provide an iterator to run through data files.

	Keyword arguments:
		dataset -- A string naming the data files to be loaded. Known
			sets are listed in the DATA_LOCS mapping, and include
			'SSystems' (star systems) and 'Assets' (planets, stations,
			and virtual holdings).
		naevroot -- The root of the Naev source tree. If omitted, the
			current directory is used.

	'''
	if naevroot is None:
		naevroot = os.curdir

	# If the dataset argument is not a known or valid data type, a KeyError
	# will result. Let it propagate upwards.
	dat_dir, dat_pattern = DATA_LOCS[dataset]

	fulldir = os.path.join(naevroot, DATA_ROOT, dat_dir)
	if not os.path.exists(fulldir):
		raise IOError("could not find data directory at '{}'".format(fulldir))

	return glob.glob(os.path.join(fulldir, dat_pattern))
