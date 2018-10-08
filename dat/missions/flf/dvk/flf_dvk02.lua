--[[

   Pirate Alliance
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

-- Localization
title = {}
text = {}

title[1] = _("The enemy of my enemy...")
text[1] = _([[Benito motions for you to sit. She doesn't seem quite as calm and relaxed as she usually is.
    "Hello again, %s. Look: we have a pretty bad situation here. As you may have guessed, we rely on... unconventional sources for supplies. Unfortunately, we seem to have hit a bit of a snag. See, one of our important sources has stopped supplying us, and I fear we may be cut off and no longer able to carry out our operations before long if we don't do something.
    "But that being said, I think I may have found a solution. See, we have reason to believe that we are actually neighboring a pirate stronghold. We're not entirely sure, but we have detected some evidence of occasional pirate activity in the nearby %s system."]])

text[2] = _([[You raise an eyebrow. It seems rather odd that pirates would be in such a remote system. Perhaps it could be a gateway of some sort?
    "You must be thinking the same thing," Benito pipes up. "Yes, that is a very strange system to see pirates in, even occasionally. That's why we think there is a secret pirate stronghold nearby. It may even be the one associated with piracy in the Frontier.
    "We must establish trading relations with that stronghold at once. This could give us just the edge we need against the Dvaereds. I honestly don't know how you can go about doing it, but my recommendation would be to go to the %s system and see if you find any pirates. Tell them you're on official FLF business, and that we're seeking to become trade partners with them. Are you in?"]])

title[3] = _("...is my friend.")
text[3] = _([["Excellent! I knew you would do it." Benito becomes visibly more relaxed, almost her usual self. "Now, %s, I'm sure you're well aware of this, but please remember that pirates are extremely dangerous. They will probably attack you, and they may have demands. I'm counting on you to overcome any... obstacles you may encounter and secure a deal." You nod in understanding. "Good," she says. "Report back here with your results." Benito then excuses herself, presumably to take care of other things.]])

title[4] = _("...is still my enemy.")
text[4] = _([["That's too bad. I understand where you're coming from, though. Please feel free to return if you are willing to take on this mission at a later date."]])

misn_title = _("Pirate Alliance")
misn_desc = _("You are to seek out pirates in the %s system and try to convince them to become trading partners with the FLF.")
misn_reward = _("Supplies for the FLF")

npc_name = _("Benito")
npc_desc = _("It seems Benito wants something from you again. Something about her looks a little off this time around.")

osd_title   = _("Pirate Alliance")
osd_desc    = {}
osd_desc[1] = _("Fly to the %s system and look for pirates")
osd_desc[2] = _("Convince pirates to begin trading with the FLF")


function create ()
   missys = system.get( "Tormulex" )
   if not misn.claim( missys ) then misn.finish( false ) end

   misn.setNPC( npc_name, "flf/unique/benito" )
   misn.setDesc( npc_desc )
end


function accept ()
   tk.msg( title[1], text[1]:format( player.name(), missys:name() ) )
   if tk.yesno( title[1], text[2]:format( missys:name() ) ) then
      tk.msg( title[3], text[3]:format( player.name() ) )

      misn.accept()

      osd_desc[1] = os._desc[1]:format( missys:name() )
      misn.osdCreate( osd_title, osd_desc )
      misn.setTitle( misn_title )
      misn.setDesc( misn_desc:format( missys:name() ) )
      marker = misn.markerAdd( missys, "high" )
      misn.setReward( misn_reward )

      job_done = false

      hook.enter( "enter" )
   else
      tk.msg( title[4], text[4] )
   end
end


function enter ()
   if not job_done then
      if system.cur() == missys then
         -- TODO: Spawn pirates
      else
         misn.osdActive( 1 )
      end
   end
end

