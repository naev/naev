--[[

   FLF diversion mission.
   Copyright (C) 2014, 2015 Julian Marchant <onpon4@riseup.net>

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

-- localization stuff
lang = naev.lang()
if lang == "notreal" then
else -- default English
   misn_title  = "FLF: Diversion in %s"
   misn_reward = "%s credits"

   success_text = {}
   success_text[1] = "You receive a transmission from an FLF officer saying that the operation has completed, and you can now return to the base."

   pay_text = {}
   pay_text[1] = "The FLF officer in charge of the primary operation thanks you for your contribution and hands you your pay."
   pay_text[2] = "You greet the FLF officer in charge of the primary operation, who seems happy that the mission was a success. You congratulate each other, and the officer hands you your pay."

   misn_desc = "A fleet of FLF ships will be conducting an operation against the Dvaered forces. Create a diversion from this operation by wreaking havoc in the nearby %s system."

   osd_title   = "FLF Diversion"
   osd_esc     = {}
   osd_desc[1] = "Fly to the %s system"
   osd_desc[2] = "Destroy Dvaered ships to get them to get the others' attention"
   osd_desc[3] = "Return to FLF base"
   osd_desc["__save"] = true
end


function create ()
   missys = flf_getTargetSystem()
   if not misn.claim( missys ) then misn.finish( false ) end

   local num_dvaereds = missys:presences()["Dvaered"]
   dv_attention_target = num_dvaereds / 20
   credits = 75 * num_dvaereds * system.cur():jumpDist( missys ) / 3
   credits = credits + rnd.sigma() * 10000
   reputation = math.max( num_dvaereds / 100, 1 )

   -- Set mission details
   misn.setTitle( misn_title:format( missys:name() ) )
   misn.setDesc( misn_desc:format( missys:name() ) )
   misn_setReward( misn_reward:format( numstring( credits ) ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   misn.accept()

   osd_desc[1] = osd_desc[1]:format( missys:name() )
   misn.osdCreate( osd_title, osd_desc )

   dv_attention = 0
   job_done = false

   hook.enter( "enter" )
   hook.jumpout( "leave" )
   hook.land( "leave" )
end


function enter ()
   if not job_done then
      if system.cur() == missys then
         misn.osdActive( 2 )
         update_dv_hook = hook.safe( update_dv )
         player.pilot():setVisible( true )
      else
         misn.osdActive( 1 )
         dv_attention = 0
         player.pilot():setVisible( false )
      end
   end
end


function leave ()
   hook.rm( update_dv_hook )
end


function update_dv ()
   for i, j in ipairs( pilot.get( { faction.get("Dvaered") } ) ) do
      hook.pilot( j, "death", "pilot_death_dv", j )
   end
end


function pilot_death_dv( arg )
   if not job_done and dv_attention < dv_attention_target then
      if not arg:memoryCheck( "flf_diversion_diverted" ) then
         dv_attention = dv_attention + 1
      end
      for i, j in ipairs( pilot.get( { faction.get("Dvaered") } ) ) do
         if not j:memoryCheck( "flf_diversion_diverted" ) then
            j:changeAI( "dvaered_norun" )
            j:setHostile()
            j:memory( "aggressive", true )
            j:setVisplayer( true )
            j:setHilight( true )
            j:memory( "flf_diversion_diverted", true )
            dv_attention = dv_attention + 1
         end
      end

      if dv_attention >= dv_attention_target then
         hook.timer( 10000, "timer_mission_success" )
      end
   end
end


function timer_mission_success ()
   job_done = true
   misn.osdActive( 3 )
   misn.markerRm( marker )
   hook.rm( update_dv_hook )
   hook.land( "land" )
   tk.msg( "", success_text[ rnd.rnd( 1, #success_text ) ] )
end


function land ()
   if planet.cur():faction():name() == "FLF" then
      tk.msg( "", pay_text[ rnd.rnd( 1, #pay_text ) ] )
      player.pay( credits )
      faction.get("FLF"):modPlayer( reputation )
      misn.finish( true )
   end
end
