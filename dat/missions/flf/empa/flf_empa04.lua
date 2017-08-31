--[[

   The FLF Coup

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

title[1] = _("The Coup")
text[1] = _([[Cheryl wastes no time. "%s, I'm glad I found you. We have a serious situation, and we need your help.
    "See, as I sort of mentioned when you were assigned your last mission, some people are quite upset about our cooperation with the Empire. However, it has come to my attention that it's worse than I thought. A fleet of FLF ships has gone rogue and is trying to initiate a coup. It's not that big of a coup, but I'm having trouble riling support against it.
    "Don't worry. It's not that they support the coup. They don't. But they're just not willing to fight against our former comrades. It just doesn't feel right to them."]])

text[2] = _([[She continues. "I'm so sorry to put you in such a horrible position. I never wanted it to be this way. But if this coup succeeds, all of our operations will be thrown into chaos and our dealings with the Empire will be completely ruined. This coup must be stopped, and since the fleet refuses to respond to our hails, they have to be..." She stops mid-sentence to regain her composure. "They have to be taken out, by any means necessary." Of course, she means that they have to be killed, and this realization makes it clear why she is having so much trouble finding support. After all, FLF soldiers kill Dvaered pilots on a daily basis. But to kill fellow comrades in the FLF? It just feels... wrong.
    Cheryl clearly sees what you are thinking in your eyes. "Again, I'm so sorry to put you in this position, %s, but I need your help. Can you join the defense against this coup?"]])

text[3] = _([["Thank you, %s. I will continue to look around for more people willing to mount a defense. The rogue fleet has probably already entered the system. Good luck." She wanders off, presumably to find some more help. Well, there's no time to lose.]])

text[4] = _([["I understand. I will continue looking for supporters. Hopefully we can mount some kind of defense."]])

pay_text = {}
pay_text[1] = _([[As you return from your dreadful mission, you sense a familiar coldness, mixed in with a great deal of sadness. Cheryl looks at you apologetically, quietly hands you your pay, and leaves.]])

misn_title = _("The Coup")
misn_desc = _("A fleet of FLF soldiers has initiated a coup. For the safety and stability of the FLF, it must be destroyed.")
misn_reward = _("Securing the stability and future of the FLF")

npc_name = _("Cheryl")
npc_desc = _("Cheryl looks unusually distraught as she looks for pilots. Perhaps you should see what is the matter.")


function create ()
   missys = system.get( "Sigur" )
   if not misn.claim( missys ) then misn.finish( false ) end

   level = 3
   ships = 4
   flfships = 0
   flfships = 2

   credits = 100000

   late_arrival = true
   late_arrival_delay = rnd.rnd( 10000, 120000 )

   misn.setNPC( npc_name, "neutral/miner2" )
   misn.setDesc( npc_desc )
end


function accept ()
   tk.msg( title[1], text[1]:format( player.name() ) )
   if tk.yesno( title[1], text[2]:format( player.name() ) ) then
      tk.msg( title[1], text[3]:format( player.name() ) )

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
      tk.msg( title[1], text[4] )
   end
end


function land_flf ()
   leave()
   last_system = nil
   if planet.cur():faction():name() == "FLF" then
      tk.msg( "", pay_text[1] )
      player.pay( credits )
      misn.finish( true )
   end
end

