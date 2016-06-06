--[[

   Sightseeing
   author:micahmumper
   
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

   Based on patrol mission, this mission ferries sightseers to various points.
   
--]]

include "numstring.lua"
include "jumpdist.lua"

lang = naev.lang()
if lang == "es" then
else -- Default to English
   pay_title = "Mission Completed"
   pay_text    = {}
   pay_text[1] = "The passengers disembark with a new appreciation for the wonders of the universe."
   pay_text[2] = "Going off-world has renewed your passengers sense of adventure."
   pay_text[3] = "The passengers burst into cheers upon returning to the hanger. What a wonderful experience."
   pay_text[4] = "The passengers enjoyed their time aboard your vessel."

   -- Mission details
   misn_title  = "Sightseeing in the %s System"
   misn_reward = "%s credits"
   misn_desc   = "Several passengers wish to go off-world and go on a sightseeing tour. Navigate to specified attractions in the %s system."

   -- Messages
   msg    = {}
   msg[2] = "Hostiles detected. Protect the passengers at all costs."
   msg[3] = "Passengers safe."
   msg[4] = "All attractions visited. Return to %s and collect your pay."

   --Sightseeing Messages
   ssmsg = {}
   ssmsg[1] = "The passengers are loving it."
   ssmsg[2] = "The wide-eyed passengers mutter with astonishment."
   ssmsg[3] = "The passengers faces are pressed up against the windows of your ship."
   ssmsg[4] = "Everyone seems like they're having a good time."
   ssmsg[5] = "A collective gasp of wonder travels through the cabin."
   ssmsg[6] = "A sense of terror and mystery engulfs the passengers as they contemplate their existance above the skies."
   ssmsg[7] = "Truly a sight to behold for the passengers."
   
   osd_title  = "Sightseeing tour in %s"
   osd_msg    = {}
   osd_msg[1] = "Fly to the %s system"
   osd_msg_2  = "Go to indicated point (%d remaining)"
   osd_msg[2] = "(null)"
   osd_msg[3] = "Return to %s in the %s system and collect your pay"
   osd_msg["__save"] = true

   mark_name = "Attraction"
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
   startingplanet = planet.cur()
   startingsystem = system.cur()
   local systems = getsysatdistance( system.cur(), 1, 2,
      function(s)
         local this_faction = s:presences()[paying_faction:name()]
         return this_faction ~= nil and this_faction > 0
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
   attractions = numpoints
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

   hostiles = {}
   hostiles["__save"] = true
   hostiles_encountered = false

   friend = missys:presence("friendly")
   foe = missys:presence("hostile")
   if friend < foe then
      misn.finish( false )
   end
   if player.pilot():ship():class() == "Luxury Yacht" then
   credits = (missys:jumpDist()*2500 + attractions*4000)*rnd.rnd(2,6)
   credits = credits + rnd.sigma() * (credits/5)
   else
   credits = missys:jumpDist()*2500 + attractions*4000
   credits = credits + rnd.sigma() * (credits/3)
   end

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
   osd_msg[3] = osd_msg[3]:format( startingplanet:name(),startingsystem:name() )
   misn.osdCreate( osd_title, osd_msg )
   civs = misn.cargoAdd( "Civilians", 0 )
   job_done = false

   hook.enter( "enter" )
   hook.land( "land" )
end


function enter ()
   if system.cur() == missys and not job_done then
      timer()
   end
end


function land ()
   if job_done and planet.cur() == startingplanet then
      misn.cargoRm( civs )
      local txt = pay_text[ rnd.rnd( 1, #pay_text ) ]
      tk.msg( pay_title, txt )
      player.pay( credits )
      misn.finish( true )
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
      misn.osdActive( 2 )
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

         local sstxt = ssmsg[ rnd.rnd( 1, #ssmsg ) ]
         player.msg( sstxt )
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
      player.msg( msg[4]:format( startingplanet:name() ) )
      misn.osdActive( 3 )
      if marker ~= nil then
         misn.markerRm( marker )
      end
      misn.markerAdd (startingsystem, "computer")
   end

   if not job_done then
      hook.timer( 50, "timer" )
   end
end