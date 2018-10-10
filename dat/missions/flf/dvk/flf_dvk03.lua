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

title[1] = _("The Next Level")
text[1] = _([["Hello there, %s! You're just in time. We were just discussing our next operation against the Dvaered oppressors! Do you want in?"]])

title[2] = _("Not This Time")
text[2] = _([["Okay. Just let me know if you change your mind."]])

title[3] = _("A Decisive Strike")
text[3] = _([[You take a seat among the group. "Fantastic!" says Benito. "Let me get you caught up, then. Do you remember that mission in Raelid you helped with a while back?" You nod. You were wondering what you were actually creating a diversion from. "Yes, well, I never got around to telling you what we actually did there. See, we've been wanting to destroy Raelid Outpost for some time, mostly because it's often used as a front for trying to scout us out. So while you were getting the attention of those idiot Dvaered oppressors, we rigged a special bomb and covertly installed it onto the outpost!
    "Now, the bomb is not perfect. The bombs can only be detonated by mechanical means, so we will need to attack the station directly. Unfortunately, we didn't have guns quite powerful enough to deal the kind of damage necessary. But thanks to your efforts, we now do. After much negotiation, we have succeeded in convincing a few pilots with Kestrels to help us out!"]])

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


function create ()
   missys = system.get( "Raelid" )
   if not misn.claim( missys ) then
      misn.finish( false )
   end

   misn.setNPC( npc_name, "flf/unique/benito" )
   misn.setDesc( npc_desc )
end


function accept ()
   
end

