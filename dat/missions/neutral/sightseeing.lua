--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Sightseeing">
 <avail>
  <priority>4</priority>
  <cond>planet.cur():class() ~= "1" and planet.cur():class() ~= "2" and planet.cur():class() ~= "3" and system.cur():presences()["Civilian"] ~= nil and system.cur():presences()["Civilian"] &gt; 0</cond>
  <chance>460</chance>
  <location>Computer</location>
  <faction>Dvaered</faction>
  <faction>Empire</faction>
  <faction>Frontier</faction>
  <faction>Goddard</faction>
  <faction>Independent</faction>
  <faction>Sirius</faction>
  <faction>Soromid</faction>
  <faction>Za'lek</faction>
 </avail>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
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

require "numstring.lua"
require "jumpdist.lua"

nolux_title = _("Not Very Luxurious")
nolux_text  = _("Since your ship is not a Luxury Yacht class ship, you will only be paid %s. Accept the mission anyway?")

pay_title = _("Mission Completed")
pay_text    = {}
pay_text[1] = _("The passengers disembark with a new appreciation for the wonders of the universe.")
pay_text[2] = _("Going off-world has renewed your passengers sense of adventure.")
pay_text[3] = _("The passengers burst into cheers upon returning to the hanger. What a wonderful experience.")
pay_text[4] = _("The passengers enjoyed their time aboard your vessel.")

pay_s_lux_title = _("Unexpected Bonus")
pay_s_lux_text    = {}
pay_s_lux_text[1] = _("The passengers appreciate that you took them an a Luxury Yacht class ship after all. You are paid the original fare rather than the reduced fare.")
pay_s_lux_text[2] = _("Your passengers were thrilled that they were able to ride in a Luxury Yacht after all. They insist on paying the originally offered fare as a show of appreciation.")
pay_s_lux_text[3] = _("As your passengers disembark, one wealthy passenger personally thanks you for taking them on a Luxury Yacht after all and gives you a tip amounting to the difference between the original fare and what your passengers paid.")
pay_s_lux_text[4] = _("When it comes time to collect your fare, the passengers collectively announce that they will be paying the original fare offered, since you took them on a Luxury Yacht after all.")

pay_s_nolux_title = _("Disappointment")
pay_s_nolux_text    = {}
pay_s_nolux_text[1] = _("Several passengers are furious that you did not take them on your Luxury Yacht class ship after all. They refuse to pay, leaving you with much less overall payment.")
pay_s_nolux_text[2] = _("While your passengers enjoyed the trip, they are not happy that you didn't take them on your Luxury Yacht class ship the entire way. They refuse to pay the full fare.")
pay_s_nolux_text[3] = _("Most of the passengers enjoyed your tour, but one particularly loud passenger complains that you tricked them into paying full price even though you did not take them on a Luxury Yacht. To calm this passenger down, you offer to reduce everyone's fare. Some passengers refuse the offer, but you still end up being paid much less than you otherwise would have been.")

-- Mission details
misn_title  = _("Sightseeing in the %s System")
misn_desc   = _("Several passengers wish to go off-world and go on a sightseeing tour. Navigate to specified attractions in the %s system.")

-- Messages
msg    = {}
msg[1] = _("All attractions visited. Return to %s and collect your pay.")

--Sightseeing Messages
ssmsg = {}
ssmsg[1] = _("The passengers are loving it.")
ssmsg[2] = _("The wide-eyed passengers mutter with astonishment.")
ssmsg[3] = _("The passengers faces are pressed up against the windows of your ship.")
ssmsg[4] = _("Everyone seems like they're having a good time.")
ssmsg[5] = _("A collective gasp of wonder travels through the cabin.")
ssmsg[6] = _("A sense of terror and mystery engulfs the passengers as they contemplate their existence above the skies.")
ssmsg[7] = _("Truly a sight to behold for the passengers.")

osd_title  = _("Sightseeing")
osd_msg    = {}
osd_msg[1] = _("Fly to the %s system")
osd_msg[2]  = _("Go to all indicated points")
osd_msg[3] = _("Return to %s in the %s system and collect your pay")
osd_msg["__save"] = true

mark_name = _("Attraction")


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

   credits = system.cur():jumpDist(missys) * 2500 + attractions * 4000
   credits_nolux = credits + rnd.sigma() * ( credits / 3 )
   credits = credits * rnd.rnd( 2, 6 )
   credits = credits + rnd.sigma() * ( credits / 5 )
   nolux = false
   nolux_known = false

   -- Set mission details
   misn.setTitle( misn_title:format( missys:name() ) )
   misn.setDesc( misn_desc:format( missys:name() ) )
   misn.setReward( creditstring( credits ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   if player.pilot():ship():class() ~= "Luxury Yacht" then
      if tk.yesno( nolux_title, nolux_text:format( creditstring(credits_nolux) ) ) then
         nolux_known = true
         misn.setReward( creditstring( credits_nolux ) )
      else
         misn.finish()
      end
   end

   misn.accept()

   osd_msg[1] = osd_msg[1]:format( missys:name() )
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
      set_marks()
      timer()
   end
end


function jumpout ()
   if not job_done and system.cur() == missys then
      misn.osdActive( 1 )
      if timer_hook ~= nil then hook.rm( timer_hook ) end
      if marks ~= nil then
         for i, m in ipairs(marks) do
            if m ~= nil then
               system.mrkRm(m)
            end
         end
         marks = nil
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

      local updated = false
      local new_points = {}
      new_points["__save"] = true

      for i, p in ipairs(points) do
         local point_pos = p:pos()

         if player_pos:dist( point_pos ) < 500 then
            local sstxt = ssmsg[ rnd.rnd( 1, #ssmsg ) ]
            player.msg( sstxt )
            updated = true
         else
            new_points[#new_points + 1] = p
         end
      end

      if updated then
         points = new_points
         set_marks()
      end
   end

   -- Another check since the previous block could change the result
   if #points <= 0 then
      job_done = true
      player.msg( msg[1]:format( startingplanet:name() ) )
      misn.osdActive( 3 )
   end

   if not job_done then
      timer_hook = hook.timer( 50, "timer" )
   end
end


function set_marks ()
   -- Remove existing marks
   if marks ~= nil then
      for i, m in ipairs(marks) do
         if m ~= nil then
            system.mrkRm(m)
         end
      end
   end

   -- Add new marks
   marks = {}
   for i, p in ipairs(points) do
      marks[i] = system.mrkAdd(mark_name, p:pos())
   end
end
