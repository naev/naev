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
   nolux_title = "Not Very Luxurious"
   nolux_text  = "Since your ship is not a Luxury Yacht class ship, you will only be paid %s credits. Accept the mission anyway?"

   pay_title = "Mission Completed"
   pay_text    = {}
   pay_text[1] = "The passengers disembark with a new appreciation for the wonders of the universe."
   pay_text[2] = "Going off-world has renewed your passengers sense of adventure."
   pay_text[3] = "The passengers burst into cheers upon returning to the hanger. What a wonderful experience."
   pay_text[4] = "The passengers enjoyed their time aboard your vessel."

   pay_s_lux_title = "Unexpected Bonus"
   pay_s_lux_text    = {}
   pay_s_lux_text[1] = "The passengers appreciate that you took them an a Luxury Yacht class ship after all. You are paid the original fare rather than the reduced fare."
   pay_s_lux_text[2] = "Your passengers were thrilled that they were able to ride in a Luxury Yacht after all. They insist on paying the originally offered fare as a show of appreciation."
   pay_s_lux_text[3] = "As your passengers disembark, one wealthy passenger personally thanks you for taking them on a Luxury Yacht after all and gives you a tip amounting to the difference between the original fare and what your passengers paid."
   pay_s_lux_text[4] = "When it comes time to collect your fare, the passengers collectively announce that they will be paying the original fare offered, since you took them on a Luxury Yacht after all."

   pay_s_nolux_title = "Disappointment"
   pay_s_nolux_text    = {}
   pay_s_nolux_text[1] = "Several passengers are furious that you did not take them on your Luxury Yacht class ship after all. They refuse to pay, leaving you with much less overall payment."
   pay_s_nolux_text[2] = "While your passengers enjoyed the trip, they are not happy that you didn't take them on your Luxury Yacht class ship the entire way. They refuse to pay the full fare."
   pay_s_nolux_text[3] = "Most of the passengers enjoyed your tour, but one particularly loud passenger complains that you tricked them into paying full price even though you did not take them on a Luxury Yacht. To calm this passenger down, you offer to reduce everyone's fare. Some passengers refuse the offer, but you still end up being paid much less than you otherwise would have been."

   -- Mission details
   misn_title  = "Sightseeing in the %s System"
   misn_reward = "%s credits"
   misn_desc   = "Several passengers wish to go off-world and go on a sightseeing tour. Navigate to specified attractions in the %s system."

   -- Messages
   msg    = {}
   msg[1] = "All attractions visited. Return to %s and collect your pay."

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


function create ()
   paying_faction = planet.cur():faction()
   startingplanet = planet.cur()
   startingsystem = system.cur()
   local systems = getsysatdistance( system.cur(), 1, 2 )
   systems[ #systems + 1 ] = startingsystem

   if #systems <= 0 then
      misn.finish( false )
   end

   missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( missys ) then misn.finish( false ) end

   local planets = missys:planets()
   local numpoints = rnd.rnd( 2, #planets )
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

   friend = missys:presence("friendly")
   foe = missys:presence("hostile")
   if friend < foe then
      misn.finish( false )
   end

   credits = missys:jumpDist() * 2500 + attractions * 4000
   credits_nolux = credits + rnd.sigma() * ( credits / 3 )
   credits = credits * rnd.rnd( 2, 6 )
   credits = credits + rnd.sigma() * ( credits / 5 )
   nolux = false
   nolux_known = false

   -- Set mission details
   misn.setTitle( misn_title:format( missys:name() ) )
   misn.setDesc( misn_desc:format( missys:name() ) )
   misn.setReward( misn_reward:format( numstring( credits ) ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   if player.pilot():ship():class() ~= "Luxury Yacht" then
      if tk.yesno( nolux_title, nolux_text:format( numstring(credits_nolux) ) ) then
         nolux_known = true
      else
         misn.finish()
      end
   end

   misn.accept()

   osd_title = osd_title:format( missys:name() )
   osd_msg[1] = osd_msg[1]:format( missys:name() )
   osd_msg[2] = osd_msg_2:format( #points )
   osd_msg[3] = osd_msg[3]:format( startingplanet:name(),startingsystem:name() )
   misn.osdCreate( osd_title, osd_msg )
   civs = misn.cargoAdd( "Civilians", 0 )
   job_done = false

   hook.enter( "enter" )
   hook.jumpout( "jumpout" )
   hook.land( "land" )
end


function enter ()
   if system.cur() == missys and not job_done then
      if player.pilot():ship():class() ~= "Luxury Yacht" then
         nolux = true
      end
      timer()
   end
end


function jumpout ()
   if not job_done and system.cur() == missys then
      misn.osdActive( 1 )
      if timer_hook ~= nil then hook.rm( timer_hook ) end
      if mark ~= nil then
         system.mrkRm( mark )
         mark = nil
      end
   end
end


function land ()
   jumpout()
   if job_done and planet.cur() == startingplanet then
      misn.cargoRm( civs )

      local ttl = pay_title
      local txt = pay_text[ rnd.rnd( 1, #pay_text ) ]
      if nolux ~= nolux_known then
         if nolux then
            ttl = pay_s_nolux_title
            txt = pay_s_nolux_text[ rnd.rnd( 1, #pay_s_nolux_text ) ]
         else
            ttl = pay_s_lux_title
            txt = pay_s_lux_text[ rnd.rnd( 1, #pay_s_lux_text ) ]
         end
      end
      tk.msg( ttl, txt )

      if nolux then
         player.pay( credits_nolux )
      else
         player.pay( credits )
      end

      misn.finish( true )
   end
end


function timer ()
   if timer_hook ~= nil then hook.rm( timer_hook ) end

   local player_pos = player.pilot():pos()

   if #points > 0 then
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
      player.msg( msg[1]:format( startingplanet:name() ) )
      misn.osdActive( 3 )
      if marker ~= nil then
         misn.markerRm( marker )
      end
      misn.markerAdd (startingsystem, "computer")
   end

   if not job_done then
      timer_hook = hook.timer( 50, "timer" )
   end
end
