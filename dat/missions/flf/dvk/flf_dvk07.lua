--[[

   The FLF Split

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

include "dat/missions/flf/flf_rogue.lua"

title = {}
text = {}

title[1] = _("The Split")
text[1] = _([[As you approach, you notice that Benito has an unusually annoyed expression. But when she seems you, she calms down somewhat. "Ah, %s." She sighs. "No one is willing to take up this mission, and while I can understand it's a tough mission, it really has to be taken care of.
    "See, for some reason, a group of FLF pilots has decided to turn traitor on us. They're hanging around outside of Sindbad and shooting us down. They need to be stopped, but no one wants to get their hands dirty killing fellow FLF pilots. But they're not FLF pilots anymore! They betrayed us! Can't anyone see that?" She takes a deep breath. "Will you do it, please? You'll be paid for the service, of course."]])

text[2] = _([["Yes, finally!" It's as if a massive weight has been lifted off of Benito's chest. "Everyone trusts you a lot, so I'm sure this will convince them that,  yes, killing traitors is the right thing to do. They're no better than Dvaereds, or those Empire scum who started shooting at us recently! Thank you for accepting the mission. Now I should at least be able to get a couple more pilots to join in and help you defend our interests against the traitors. Good luck!"]])

text[3] = _([["Ugh, this is so annoying... I understand, though. Just let me know if you change your mind, okay?"]])

pay_text = {}
pay_text[1] = _([[Upon your return to the station, you are greeted by Benito. "Thanks once again for a job well done. I really do appreciate it. Not only have those traitors been taken care of, the others have become much more open to the idea that, hay, traitors are traitors and must be eliminated." She hands you a credit chip. "Here is your pay. Thank you again."]])

misn_title = _("The Split")
misn_desc = _("A fleet of FLF soldiers has betrayed the FLF. Destroy this fleet.")
misn_reward = _("Getting rid of traiterous scum")

npc_name = _("Benito")
npc_desc = _("Benito seems to be frantically searching for a pilot.")


function create ()
   missys = system.get( "Sigur" )
   if not misn.claim( missys ) then misn.finish( false ) end

   level = 3
   ships = 4
   flfships = 2

   credits = 100000

   late_arrival = true
   late_arrival_delay = rnd.rnd( 10000, 120000 )

   misn.setNPC( npc_name, "flf/unique/benito" )
   misn.setDesc( npc_desc )
end


function accept ()
   if tk.yesno( title[1], text[1]:format( player.name() ) ) then
      tk.msg( title[1], text[2]:format( player.name() ) )

      misn.accept()

      misn.setTitle( misn_title )
      misn.setDesc( misn_desc )
      misn.setReward( misn_reward )
      marker = misn.markerAdd( missys, "high" )

      osd_desc[1] = osd_desc[1]:format( missys:name() )
      osd_desc[2] = osd_desc[2]:format( misn_level[level] )
      misn.osdCreate( osd_title, osd_desc )

      rogue_ships_left = 0
      job_done = false
      last_system = planet.cur()

      hook.enter( "enter" )
      hook.jumpout( "leave" )
      hook.land( "leave" )
   else
      tk.msg( title[1], text[3] )
   end
end


function land_flf ()
   leave()
   last_system = nil
   if planet.cur():faction():name() == "FLF" then
      tk.msg( "", pay_text[1] )
      player.pay( credits )
      flf_setReputation( 95 )
      misn.finish( true )
   end
end

