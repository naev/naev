--[[

   Assault on Raelid
   Copyright (C) 2018 Julie Marchant <onpon4@riseup.net>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

--]]

include "numstring.lua"
include "dat/missions/flf/flf_common.lua"

-- Localization stuff
title = {}
text = {}

misn_title = _("Assault on Raelid")
misn_desc = _("Join with the other FLF pilots for the assault on Raelid Outpost.")
misn_reward = _("A great victory against the Dvaereds")

npc_name = _("Benito")
npc_desc = _("Benito is seated at a table with several other FLF soldiers. She motions for you to come over.")

osd_title   = _("Assault on Raelid")
osd_desc    = {}
osd_desc[1] = _("Fly to the %s system")
osd_desc[2] = _("Protect the Pirate Kestrel while it launches its attack on %s")
osd_desc[3] = _("Return to FLF base")
