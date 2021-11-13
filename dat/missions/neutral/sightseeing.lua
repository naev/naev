--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Sightseeing">
 <avail>
  <priority>4</priority>
  <cond>planet.cur():class() ~= "1" and planet.cur():class() ~= "2" and planet.cur():class() ~= "3" and system.cur():presences()["Independent"] ~= nil and system.cur():presences()["Independent"] &gt; 0</cond>
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

   Based on patrol mission, this mission ferries sightseers to various points.

--]]
local pir = require "common.pirate"
local fmt = require "format"
local vntk = require "vntk"
local lmisn = require "lmisn"

local marks -- Non-persistent state
local set_marks -- Forward-declared functions

local pay_text    = {}
pay_text[1] = _("The passengers disembark with a new appreciation for the wonders of the universe.")
pay_text[2] = _("Going off-world has renewed your passengers sense of adventure.")
pay_text[3] = _("The passengers burst into cheers upon returning to the hanger. What a wonderful experience.")
pay_text[4] = _("The passengers enjoyed their time aboard your vessel.")

local pay_s_lux_text    = {}
pay_s_lux_text[1] = _("The passengers appreciate that you took them an a Luxury Yacht class ship after all. You are paid the original fare rather than the reduced fare.")
pay_s_lux_text[2] = _("Your passengers were thrilled that they were able to ride in a Luxury Yacht after all. They insist on paying the originally offered fare as a show of appreciation.")
pay_s_lux_text[3] = _("As your passengers disembark, one wealthy passenger personally thanks you for taking them on a Luxury Yacht after all and gives you a tip amounting to the difference between the original fare and what your passengers paid.")
pay_s_lux_text[4] = _("When it comes time to collect your fare, the passengers collectively announce that they will be paying the original fare offered, since you took them on a Luxury Yacht after all.")

local pay_s_nolux_text    = {}
pay_s_nolux_text[1] = _("Several passengers are furious that you did not take them on your Luxury Yacht class ship after all. They refuse to pay, leaving you with much less overall payment.")
pay_s_nolux_text[2] = _("While your passengers enjoyed the trip, they are not happy that you didn't take them on your Luxury Yacht class ship the entire way. They refuse to pay the full fare.")
pay_s_nolux_text[3] = _("Most of the passengers enjoyed your tour, but one particularly loud passenger complains that you tricked them into paying full price even though you did not take them on a Luxury Yacht. To calm this passenger down, you offer to reduce everyone's fare. Some passengers refuse the offer, but you still end up being paid much less than you otherwise would have been.")

--Sightseeing Messages
local ssmsg = {}
ssmsg[1] = _("The passengers are loving it.")
ssmsg[2] = _("The wide-eyed passengers mutter with astonishment.")
ssmsg[3] = _("The passengers faces are pressed up against the windows of your ship.")
ssmsg[4] = _("Everyone seems like they're having a good time.")
ssmsg[5] = _("A collective gasp of wonder travels through the cabin.")
ssmsg[6] = _("A sense of terror and mystery engulfs the passengers as they contemplate their existence above the skies.")
ssmsg[7] = _("Truly a sight to behold for the passengers.")

function create ()
   mem.paying_faction = planet.cur():faction()
   mem.startingplanet = planet.cur()
   mem.startingsystem = system.cur()
   local systems = lmisn.getSysAtDistance( system.cur(), 1, 2 )
   systems[ #systems + 1 ] = mem.startingsystem

   if #systems <= 0 then
      misn.finish( false )
   end

   mem.missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( mem.missys ) then misn.finish( false ) end

   local planets = mem.missys:planets()
   local numpoints = rnd.rnd( 2, #planets )
   mem.attractions = numpoints
   mem.points = {}
   while numpoints > 0 and #planets > 0 do
      local p = rnd.rnd( 1, #planets )
      mem.points[ #mem.points + 1 ] = planets[p]
      numpoints = numpoints - 1

      local new_planets = {}
      for i, j in ipairs( planets ) do
         if i ~= p then
            new_planets[ #new_planets + 1 ] = j
         end
      end
      planets = new_planets
   end
   if #mem.points < 2 then
      misn.finish( false )
   end

   local friend = mem.missys:presence("friendly")
   local foe = mem.missys:presence("hostile")
   if friend < foe then
      misn.finish( false )
   end

   mem.credits = system.cur():jumpDist(mem.missys) * 2500 + mem.attractions * 4000
   mem.credits_nolux = mem.credits + rnd.sigma() * ( mem.credits / 3 )
   mem.credits = mem.credits * rnd.rnd( 2, 6 )
   mem.credits = mem.credits + rnd.sigma() * ( mem.credits / 5 )
   mem.nolux = false
   mem.nolux_known = false

   -- Set mission details
   misn.setTitle( fmt.f( _("Sightseeing in the {sys} System"), {sys=mem.missys} ) )
   misn.setDesc( fmt.f(_("Several passengers wish to go off-world and go on a sightseeing tour. Navigate to specified attractions in the {sys} system."), {sys=mem.missys} ) )
   misn.setReward( fmt.credits( mem.credits ) )
   mem.marker = misn.markerAdd( mem.missys, "computer" )
end


function accept ()
   if player.pilot():ship():classDisplay() ~= "Luxury Yacht" then
      if tk.yesno( _("Not Very Luxurious"), fmt.f( _("Since your ship is not a Luxury Yacht class ship, you will only be paid {credits}. Accept the mission anyway?"), {credits=fmt.credits(mem.credits_nolux)} ) ) then
         mem.nolux_known = true
         misn.setReward( fmt.credits( mem.credits_nolux ) )
      else
         misn.finish()
      end
   end

   misn.accept()

   misn.osdCreate( _("Sightseeing"), {
      fmt.f( _("Fly to the {sys} system"), {sys=mem.missys} ),
      _("Go to all indicated points"),
      fmt.f(_("Return to {pnt} in the {sys} system and collect your pay"), {pnt=mem.startingplanet, sys=mem.startingsystem} ),
   } )
   local c = misn.cargoNew( N_("Sightseers"), N_("A bunch of sightseeing civilians.") )
   mem.civs = misn.cargoAdd( c, 0 )
   mem.job_done = false

   hook.enter( "enter" )
   hook.jumpout( "jumpout" )
   hook.land( "land" )
end


function enter ()
   if system.cur() == mem.missys and not mem.job_done then
      if player.pilot():ship():classDisplay() ~= "Luxury Yacht" then
         mem.nolux = true
      end
      set_marks()
      timer()
   end
end


function jumpout ()
   if not mem.job_done and system.cur() == mem.missys then
      misn.osdActive( 1 )
      hook.rm( mem.timer_hook )
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
   if mem.job_done and planet.cur() == mem.startingplanet then
      misn.cargoRm( mem.civs )

      local ttl = _("Mission Completed")
      local txt = pay_text[ rnd.rnd( 1, #pay_text ) ]
      if mem.nolux ~= mem.nolux_known then
         if mem.nolux then
            ttl = _("Disappointment")
            txt = pay_s_nolux_text[ rnd.rnd( 1, #pay_s_nolux_text ) ]
         else
            ttl = _("Unexpected Bonus")
            txt = pay_s_lux_text[ rnd.rnd( 1, #pay_s_lux_text ) ]
         end
      end
      lmisn.sfxMoney()
      vntk.msg( ttl, txt )

      pir.reputationNormalMission(rnd.rnd(2,3))
      if mem.nolux then
         player.pay( mem.credits_nolux )
      else
         player.pay( mem.credits )
      end

      misn.finish( true )
   end
end


function timer ()
   hook.rm( mem.timer_hook )

   local player_pos = player.pos()

   if #mem.points > 0 then
      misn.osdActive( 2 )

      local updated = false
      local new_points = {}

      for i, p in ipairs(mem.points) do
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
         mem.points = new_points
         set_marks()
      end
   end

   -- Another check since the previous block could change the result
   if #mem.points <= 0 then
      mem.job_done = true
      player.msg( fmt.f(_("All attractions visited. Return to {pnt} and collect your pay."), {pnt=mem.startingplanet} ) )
      misn.osdActive( 3 )
      misn.markerMove( mem.marker, mem.startingsystem )
   end

   if not mem.job_done then
      mem.timer_hook = hook.timer( 0.05, "timer" )
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
   for i, p in ipairs(mem.points) do
      marks[i] = system.mrkAdd( p:pos(), _("Attraction") )
   end
end
