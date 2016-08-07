--[[

   FLF diversion mission.
   Copyright (C) 2014, 2015 Julie Marchant <onpon4@riseup.net>

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
   osd_desc    = {}
   osd_desc[1] = "Fly to the %s system"
   osd_desc[2] = "Engage and destroy Dvaered ships to get their attention"
   osd_desc[3] = "Return to FLF base"
   osd_desc["__save"] = true
end


function create ()
   missys = flf_getTargetSystem()
   if not misn.claim( missys ) then misn.finish( false ) end

   local num_dvaereds = missys:presences()["Dvaered"]
   local num_empire = missys:presences()["Empire"]
   local num_flf = missys:presences()["FLF"]
   if num_dvaereds == nil then num_dvaereds = 0 end
   if num_empire == nil then num_empire = 0 end
   if num_flf == nil then num_flf = 0 end
   dv_attention_target = num_dvaereds / 50
   credits = 200 * (num_dvaereds + num_empire - num_flf) * system.cur():jumpDist( missys ) / 3
   credits = credits + rnd.sigma() * 10000
   reputation = math.max( (num_dvaereds + num_empire - num_flf) / 25, 1 )
   if credits < 10000 then misn.finish( false ) end

   -- Set mission details
   misn.setTitle( misn_title:format( missys:name() ) )
   misn.setDesc( misn_desc:format( missys:name() ) )
   misn.setReward( misn_reward:format( numstring( credits ) ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   misn.accept()

   osd_desc[1] = osd_desc[1]:format( missys:name() )
   misn.osdCreate( osd_title, osd_desc )

   dv_attention = 0
   dv_coming = false
   job_done = false

   hook.enter( "enter" )
   hook.jumpout( "leave" )
   hook.land( "leave" )
end


function enter ()
   if not job_done then
      if system.cur() == missys then
         misn.osdActive( 2 )
         update_dv()
      else
         misn.osdActive( 1 )
         dv_attention = 0
      end
   end
end


function leave ()
   if update_dv_hook ~= nil then hook.rm( update_dv_hook ) end
end


function update_dv ()
   for i, j in ipairs( pilot.get( { faction.get("Dvaered") } ) ) do
      hook.pilot( j, "attacked", "pilot_attacked_dv" )
      hook.pilot( j, "death", "pilot_death_dv" )
   end
   update_dv_hook = hook.timer( 3000, "update_dv" )
end


function add_attention( p )
   p:setHilight( true )

   if not job_done then
      dv_attention = dv_attention + 1
      if dv_attention >= dv_attention_target and dv_attention - 1 < dv_attention_target then
         if success_hook ~= nil then hook.rm( success_hook ) end
         success_hook = hook.timer( 30000, "timer_mission_success" )
      end

      hook.pilot( p, "jump", "rm_attention" )
      hook.pilot( p, "land", "rm_attention" )
   end
end


function rm_attention ()
   dv_attention = math.max( dv_attention - 1, 0 )
   if dv_attention < dv_attention_target then
      if success_hook ~= nil then hook.rm( success_hook ) end
   end
end


function pilot_attacked_dv( p, attacker )
   if attacker == player.pilot() and not dv_coming and rnd.rnd() < 0.01 then
      dv_coming = true
      hook.timer( 10000, "timer_spawn_dv" )
   end
end


function pilot_death_dv( p, attacker )
   if attacker == player.pilot() and not dv_coming and rnd.rnd() < 0.25 then
      dv_coming = true
      hook.timer( 10000, "timer_spawn_dv" )
   end
end


function timer_spawn_dv ()
   dv_coming = false
   if not job_done then
      local shipnames = { "Dvaered Vendetta", "Dvaered Ancestor", "Dvaered Phalanx", "Dvaered Vigilance", "Dvaered Goddard", "Dvaered Small Patrol", "Dvaered Big Patrol" }
      local shipname = shipnames[ rnd.rnd( 1, #shipnames ) ]
      for i, j in ipairs( pilot.add( shipname ) ) do
         add_attention( j )
      end
   end
end


function timer_mission_success ()
   if dv_attention >= dv_attention_target then
      job_done = true
      misn.osdActive( 3 )
      if marker ~= nil then misn.markerRm( marker ) end
      if update_dv_hook ~= nil then hook.rm( update_dv_hook ) end
      hook.land( "land" )
      tk.msg( "", success_text[ rnd.rnd( 1, #success_text ) ] )
   end
end


function land ()
   if planet.cur():faction():name() == "FLF" then
      tk.msg( "", pay_text[ rnd.rnd( 1, #pay_text ) ] )
      player.pay( credits )
      faction.get("FLF"):modPlayer( reputation )
      misn.finish( true )
   end
end
