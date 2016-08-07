--[[

   Patrol
   Copyright 2014-2016 Julie Marchant

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

--

   Generalized replacement for Dvaered patrol mission. Can work with any
   faction.

--]]

include "numstring.lua"
include "jumpdist.lua"

lang = naev.lang()
if lang == "es" then
else -- Default to English
   pay_title = "Mission Completed"
   pay_text    = {}
   pay_text[1] = "After going through some paperwork, an officer hands you your pay and sends you off."
   pay_text[2] = "A tired-looking officer verifies your mission log and hands you your pay."
   pay_text[3] = "The officer you deal with thanks you for your work, hands you your pay, and sends you off."
   pay_text[4] = "An officer goes through the necessary paperwork, looking bored the entire time, and hands you your fee."

   abandon_title = "Mission Abandoned"
   abandon_text    = {}
   abandon_text[1] = "You are sent a message informing you that landing in the middle of a patrol mission is considered to be abandonment. As such, your contract is void and you will not recieve payment."


   -- Mission details
   misn_title  = "Patrol of the %s System"
   misn_reward = "%s credits"
   misn_desc   = "Patrol specified points in the %s system, eliminating any hostiles you encounter."

   -- Messages
   msg    = {}
   msg[1] = "Point secure."
   msg[2] = "Hostiles detected. Engage hostiles."
   msg[3] = "Hostiles eliminated."
   msg[4] = "Patrol complete. You can now collect your pay."
   msg[5] = "MISSION FAILURE! You showed up too late."
   msg[6] = "MISSION FAILURE! You have left the %s system."

   osd_title  = "Patrol of %s"
   osd_msg    = {}
   osd_msg[1] = "Fly to the %s system"
   osd_msg_2  = "Go to indicated point (%d remaining)"
   osd_msg[2] = "(null)"
   osd_msg[3] = "Eliminate hostiles"
   osd_msg[4] = "Land on the nearest %s planet and collect your pay"
   osd_msg["__save"] = true

   mark_name = "Patrol Point"
end


-- Get the number of enemies in a particular system
function get_enemies( sys )
   local enemies = 0
   for i, j in ipairs( paying_faction:enemies() ) do
      local p = sys:presences()[j:name()]
      if p ~= nil then
         enemies = enemies + p
      end
   end
   return enemies
end


function create ()
   paying_faction = planet.cur():faction()

   local systems = getsysatdistance( system.cur(), 1, 2,
      function(s)
         local this_faction = s:presences()[paying_faction:name()]
         return this_faction ~= nil and this_faction > 0 and get_enemies(s) > 0
      end )
   if get_enemies( system.cur() ) then
      systems[ #systems + 1 ] = system.cur()
   end

   if #systems <= 0 then
      misn.finish( false )
   end

   missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( missys ) then misn.finish( false ) end

   local planets = missys:planets()
   local numpoints = math.min( rnd.rnd( 2, 5 ), #planets )
   points = {}
   points["__save"] = true
   while numpoints > 0 and #planets > 0 do
      local p = rnd.rnd( 1, #planets )
      points[ #points + 1 ] = planets[p]
      numpoints = numpoints - 1

      local new_planets = {}
      for i, j in ipairs( planets ) do
         if i ~= p then
            new_planets[ #new_planets + 1 ] = j
         end
      end
      planets = new_planets
   end
   if #points < 2 then
      misn.finish( false )
   end

   jumps_permitted = missys:jumpDist() + 3
   hostiles = {}
   hostiles["__save"] = true
   hostiles_encountered = false

   local n_enemies = get_enemies( missys )
   if n_enemies == 0 then
      misn.finish( false )
   end
   credits = n_enemies * 2000
   credits = credits + rnd.sigma() * (credits / 3)
   reputation = math.floor( n_enemies / 75 )

   -- Set mission details
   misn.setTitle( misn_title:format( missys:name() ) )
   misn.setDesc( misn_desc:format( missys:name() ) )
   misn.setReward( misn_reward:format( numstring( credits ) ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   misn.accept()

   osd_title = osd_title:format( missys:name() )
   osd_msg[1] = osd_msg[1]:format( missys:name() )
   osd_msg[2] = osd_msg_2:format( #points )
   osd_msg[4] = osd_msg[4]:format( paying_faction:name() )
   misn.osdCreate( osd_title, osd_msg )

   job_done = false

   hook.enter( "enter" )
   hook.jumpout( "jumpout" )
   hook.land( "land" )
end


function enter ()
   if system.cur() == missys and not job_done then
      timer()
   end
end


function jumpout ()
   if mark ~= nil then
      system.mrkRm( mark )
      mark = nil
   end

   jumps_permitted = jumps_permitted - 1
   local last_sys = system.cur()
   if not job_done then
      if last_sys == missys then
         fail( msg[6]:format( last_sys:name() ) )
      elseif jumps_permitted < 0 then
         fail( msg[5] )
      end
   end
end


function land ()
   if mark ~= nil then
      system.mrkRm( mark )
      mark = nil
   end

   jumps_permitted = jumps_permitted - 1
   if job_done and planet.cur():faction() == paying_faction then
      local txt = pay_text[ rnd.rnd( 1, #pay_text ) ]
      tk.msg( pay_title, txt )
      player.pay( credits )
      paying_faction:modPlayerSingle( reputation )
      misn.finish( true )
   elseif not job_done and system.cur() == missys then
      local txt = abandon_text[ rnd.rnd( 1, #abandon_text ) ]
      tk.msg( abandon_title, txt )
      misn.finish( false )
   end
end


function pilot_leave ( pilot )
   local new_hostiles = {}
   for i, j in ipairs( hostiles ) do
      if j ~= nil and j ~= pilot and j:exists() then
         new_hostiles[ #new_hostiles + 1 ] = j
      end
   end

   hostiles = new_hostiles
end


function timer ()
   if timer_hook ~= nil then hook.rm( timer_hook ) end

   local player_pos = player.pilot():pos()
   local enemies = pilot.get( paying_faction:enemies() )

   for i, j in ipairs( enemies ) do
      if j ~= nil and j:exists() then
         local already_in = false
         for a, b in ipairs( hostiles ) do
            if j == b then
               already_in = true
            end
         end
         if not already_in then
            if player_pos:dist( j:pos() ) < 1500 then
               j:setVisible( true )
               j:setHilight( true )
               j:setHostile( true )
               hook.pilot( j, "death", "pilot_leave" )
               hook.pilot( j, "jump", "pilot_leave" )
               hook.pilot( j, "land", "pilot_leave" )
               hostiles[ #hostiles + 1 ] = j
            end
         end
      end
   end

   if #hostiles > 0 then
      if not hostiles_encountered then
         player.msg( msg[2] )
         hostiles_encountered = true
      end
      misn.osdActive( 3 )
   elseif #points > 0 then
      if hostiles_encountered then
         player.msg( msg[3] )
         hostiles_encountered = false
      end
      misn.osdActive( 2 )

      local point_pos = points[1]:pos()

      if mark == nil then
         mark = system.mrkAdd( mark_name, point_pos )
      end

      if player_pos:dist( point_pos ) < 500 then
         local new_points = {}
         for i = 2, #points do
            new_points[ #new_points + 1 ] = points[i]
         end
         points = new_points
         points["__save"] = true

         player.msg( msg[1] )
         osd_msg[2] = osd_msg_2:format( #points )
         misn.osdCreate( osd_title, osd_msg )
         misn.osdActive(2)
         if mark ~= nil then
            system.mrkRm( mark )
            mark = nil
         end
      end
   else
      job_done = true
      player.msg( msg[4] )
      misn.osdActive( 4 )
      if marker ~= nil then
         misn.markerRm( marker )
      end
   end

   if not job_done then
      timer_hook = hook.timer( 50, "timer" )
   end
end


-- Fail the mission, showing message to the player.
function fail( message )
   if message ~= nil then
      -- Pre-colourized, do nothing.
      if message:find("\027") then
         player.msg( message )
      -- Colourize in red.
      else
         player.msg( "\027r" .. message .. "\0270" )
      end
   end
   misn.finish( false )
end
