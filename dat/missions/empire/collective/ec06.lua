--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Operation Cold Metal">
 <unique />
 <priority>2</priority>
 <cond>faction.playerStanding("Empire") &gt; 5 and var.peek("collective_fail") ~= true</cond>
 <done>Operation Black Trinity</done>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Omega Station</spob>
 <tags>
  <tag>emp_cap_ch01_lrg</tag>
 </tags>
 <notes>
  <provides name="The Collective is dead and no one will miss them"/>
  <campaign>Collective</campaign>
 </notes>
</mission>
--]]
--[[

   Operation Cold Metal

   Author: bobbens
      minor edits by Infiltrator

   Seventh and final mission in the Collective Campaign

]]--
require "proximity"
local emp = require "common.empire"
local fmt = require "format"
local pilotai = require "pilotai"
local equipopt = require "equipopt"

-- Mission constants
local misn_base = spob.get("Omega Station")
local misn_target_sys1 = system.get("C-59")
local misn_target_sys2 = system.get("C-28")
local misn_final_sys = system.get("C-00")

local droneC, fleetC, fleetE, refesc, refship -- Non-persistent state

function create ()
   local missys = {misn_target_sys1, misn_target_sys2, misn_final_sys}
   if not misn.claim(missys) then
      misn.finish( false )
   end

   misn.setNPC( _("Keer"), "empire/unique/keer.webp", _("You see Commodore Keer at a table with a couple of other pilots. She motions for you to sit down with them.") )
end


-- Creates the mission
function accept ()

   -- Intro text
   if not tk.yesno( _("Bar"), _([[You join Commodore Keer at her table.
    She begins, "We're going to finally attack the Collective. We've gotten the Emperor himself to bless the mission and send some of his better pilots. Would you be interested in aiding in the destruction of the Collective?"]]) ) then
      misn.finish()
   end

   -- Accept the mission
   misn.accept()

   -- Mission data
   mem.misn_stage = 0
   mem.misn_marker = misn.markerAdd( misn_target_sys2, "low" )
   mem.misn_marker = misn.markerAdd( misn_final_sys, "high" )

   -- Mission details
   misn.setTitle(_("Operation Cold Metal"))
   misn.setReward( _("Fame and Glory") )
   misn.setDesc( fmt.f(_("Neutralize enemy forces in {sys}"), {sys=misn_target_sys1} ))
   local osd_msg = {
      fmt.f(_("Fly to {final_sys} via {sys} and meet up with the Imperial fleet"), {final_sys=misn_final_sys, sys=misn_target_sys2}),
      _("Defeat the Starfire"),
      _("Report back"),
   }
   if var.peek("trinity") then
      osd_msg[2] = _("Defeat the Starfire and the Trinity")
   end
   misn.osdCreate(_("Operation Cold Metal"), osd_msg)

   tk.msg( _("Operation Cold Metal"), fmt.f( _([["The Operation has been dubbed 'Cold Metal'. We're going to mount an all-out offensive in C-00. The systems up to {sys} are already secure and under our control, all we need to do now is to take the final stronghold. Should we encounter the Starfire at any stage our goal will be to destroy it and head back. The Imperial fleet will join you when you get there. See you in combat, pilot."]]),
   {sys=misn_target_sys1} ) )

   hook.jumpout("jumpout")
   hook.enter("jumpin")
   hook.land("land")
end


function jumpout ()
   mem.last_sys = system.cur()
end


-- Handles jumping to target system
function jumpin ()
   if system.cur() == misn_target_sys1 or system.cur() == misn_target_sys2 then
      -- Remove collective pilots in most dangerous systems
      pilot.clear()
      pilot.toggleSpawn(false)
   end
   if mem.misn_stage == 0 then
      -- Entering target system?
      if system.cur() == misn_final_sys then
         pilot.clear()
         pilot.toggleSpawn(false)

         local function setup_pilot( p, pos )
            pilotai.guard( p, pos )
            local m = p:memory()
            m.guarddodist = math.huge
            mem.guardreturndist = math.huge
         end

         -- Create big battle
         fleetE = {}
         fleetC = {}
         droneC = {}
         local fleetCpos = vec2.new(0, 0)
         local fleetEpos = jump.get( system.cur(), mem.last_sys ):pos()
         mem.deathsC = 0

         local emp_boss = pilot.add( "Empire Peacemaker", "Empire", mem.last_sys )
         setup_pilot( emp_boss, fleetCpos )
         table.insert( fleetE, emp_boss )
         local h1 = pilot.add( "Empire Hawking", "Empire", mem.last_sys )
         h1:setLeader( emp_boss )
         setup_pilot( h1, fleetCpos )
         table.insert( fleetE, h1 )
         local h2 = pilot.add( "Empire Hawking", "Empire", mem.last_sys )
         h2:setLeader( emp_boss )
         setup_pilot( h2, fleetCpos )
         table.insert( fleetE, h2 )
         for i=1,6 do
            local boss = pilot.add( "Empire Pacifier", "Empire", mem.last_sys )
            for j=1,3 do
               local f = pilot.add( "Empire Lancelot", "Empire", mem.last_sys )
               f:setLeader( boss )
               table.insert( fleetE, f )
            end
            setup_pilot( boss, fleetCpos )
            table.insert( fleetE, boss )
         end

         local starfire = pilot.add( "Goddard", "Collective", fleetCpos, _("Starfire"), {naked=true, ai="collective_norun"} )
         equipopt.empire( starfire )
         starfire:setNoDisable()
         starfire:setVisible()
         starfire:setHilight()
         setup_pilot( starfire, fleetEpos )
         hook.pilot(starfire, "death", "col_dead")
         table.insert( fleetC, starfire )
         if var.peek("trinity") then
            local trinity = pilot.add( "Empire Hawking", "Collective", fleetCpos + vec2.new(300, 0), _("ESS Trinity"), {naked=true, ai="collective_norun"} )
            equipopt.empire( trinity )
            trinity:setNoDisable()
            trinity:setHilight()
            setup_pilot( trinity, fleetEpos )
            hook.pilot(trinity, "death", "col_dead")
            table.insert( fleetC, trinity )
         end
         droneC = {}
         for i=1,6 do
            local pos = fleetCpos + vec2.newP( 10e3*rnd.rnd(), rnd.angle() )
            local bossdrone = pilot.add( "Heavy Drone", "Collective", pos, _("Collective Heavy Drone"), {ai="collective_norun"} )
            for j=1,5 do
               local d = pilot.add( "Drone", "Collective", pos+vec2.newP(100+100*rnd.rnd(), rnd.angle()), _("Collective Drone"), {ai="collective_norun"} )
               d:setLeader( bossdrone )
               setup_pilot( d, fleetEpos )
               table.insert( droneC, d )
            end
            setup_pilot( bossdrone, fleetEpos )
            table.insert( droneC, bossdrone )
         end
         for i=1,5 do
            local pos = fleetCpos + vec2.newP( 300+200*rnd.rnd(), rnd.angle() )
            local d = pilot.add( "Heavy Drone", "Collective", pos, _("Collective Heavy Drone"), {ai="collective_norun"} )
            d:setLeader( starfire )
         end
         for i=1,25 do
            local pos = fleetCpos + vec2.newP( 300+200*rnd.rnd(), rnd.angle() )
            local d = pilot.add( "Drone", "Collective", pos, _("Collective Drone"), {ai="collective_norun"} )
            d:setLeader( starfire )
         end

         for _, j in ipairs(fleetE) do
            j:setFriendly(true)
         end

         emp_boss:broadcast(_("To all pilots, this is mission control! We are ready to begin our attack! Engage at will!"))
         misn.osdActive(2)
         mem.misn_stage = 1
      else
         misn.osdActive(1)
         mem.misn_stage = 0
      end
   elseif mem.misn_stage == 1 and system.cur() ~= misn_final_sys then
      pilot.clear()
      pilot.toggleSpawn(false)
      mem.misn_stage = 0
      diff.apply("collective_dead")
      hook.timer( 4.0, "fail_timer" )
   end
end


function fail_timer ()
   tk.msg( _("Cowardly Behavior"), _([[You receive a message signed by Commodore Keer:
"There is no room for cowards in the Empire's fleet."
The signature does seem valid.]]) )
   emp.addCollectiveLog( _([[You abandoned your mission to help the Empire destroy the Collective. Commander Keer transmitted a message: "There is no room for cowards in the Empire's fleet."]]) )
   misn.finish( true )
end


function refuelBroadcast ()
   if refship:exists() then
      refship:broadcast(_("Tanker in system, contact if in need of fuel."))
      hook.timer(30.0, "refuelBroadcast")
   end
end


local function addRefuelShip ()
   -- Create the pilot
   refship = pilot.add( "Empire Rainmaker", "Empire", mem.last_sys, _("Fuel Tanker"), {ai="empire_refuel", naked=true} )
   equipopt.empire( refship, {fuel=1000} ) -- max fuel!
   refship:setFriendly()
   refship:setVisplayer()
   refship:setHilight()
   refship:setNoJump()

   -- Add some escorts
   refesc = {}
   refesc[1] = pilot.add( "Empire Lancelot", "Empire", mem.last_sys, nil, {ai="empire_idle"} )
   refesc[2] = pilot.add( "Empire Lancelot", "Empire", mem.last_sys, nil, {ai="empire_idle"} )
   for k,v in ipairs(refesc) do
      v:setFriendly()
   end

   -- Broadcast spam
   hook.timer(15.0, "refuelBroadcast")
end


-- Handles collective death
function col_dead( _victim )
   mem.deathsC = mem.deathsC + 1
   if var.peek("trinity") and mem.deathsC < 2 then
      return
   end
   misn.osdActive(3)
   addRefuelShip()
   mem.misn_stage = 4

   -- Change back to normal AI
   for k,v in ipairs(fleetE) do
      if v:exists() then
         v:changeAI( "empire" )
      end
   end
end


-- Handles arrival back to base
function land ()
   -- Final landing stage
   if mem.misn_stage == 4 and spob.cur() == misn_base then

      tk.msg( _("Mission Success"), fmt.f(_([[As you approach to land on {pnt} you notice big banners placed on the exterior of the station. They seem to be in celebration of the final defeat of the Collective. Upon landing, you are saluted by the welcoming committee in charge of honoring all the returning pilots.
    You notice Commodore Keer. Upon greeting her, she says, "You did a good job out there. No need to worry about the Collective anymore. Without Welsh, the Collective won't stand a chance, since they aren't truly autonomous. Right now we have some ships cleaning up the last of the Collective; shouldn't take too long to be back to normal."]]), {pnt=misn_base}) )

      diff.apply("collective_dead")

      -- Rewards
      -- This was the last mission in the minor campaign, so bump the reputation cap.
      faction.modPlayerSingle("Empire",5)
      player.pay( emp.rewards.ec06 )

      tk.msg( _("Mission Success"), _([[She continues. "As a symbol of appreciation, you should find a deposit of 5,000,000 credits in your account. There will be a celebration later today in the officer's room if you want to join in."
    And so ends the Collective threat...
    You don't remember much of the after party, but you wake up groggily in your ship clutching an Empire officer's boot.]]) )
      player.outfitAdd("Left Boot")

      emp.addCollectiveLog( _([[You helped the Empire to finally destroy the Collective once and for all. The Collective is now no more.]]) )

      misn.finish(true) -- Run last
   end
end
