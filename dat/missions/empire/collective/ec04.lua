--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Collective Extraction">
 <unique />
 <priority>2</priority>
 <cond>faction.playerStanding("Empire") &gt; 5 and var.peek("emp_commando") ~= nil and time.get() &gt; time.fromnumber( var.peek("emp_commando") )</cond>
 <done>Collective Distraction</done>
 <chance>100</chance>
 <location>Land</location>
 <spob>Omega Enclave</spob>
 <notes>
  <campaign>Collective</campaign>
 </notes>
</mission>
--]]
--[[

   Collective Extraction

   Author: bobbens
      minor edits by Infiltrator

   Fifth mission in the collective mini campaign.

   You must go rescue a team of commandos after being attacked by collective
    forces on a surveillance mission.

]]--
local fleet = require "fleet"
local lmisn = require "lmisn"
require "proximity"
local fmt = require "format"
local emp = require "common.empire"

-- Mission constants
local misn_target, misn_target_sys = spob.getS("Eiroik")
local misn_base, misn_base_sys = spob.getS("Omega Enclave")
local credits = emp.rewards.ec04

local fleet1, fleet2, swarm1, swarm2, swarm3 -- Non-persistent state
local empireAttack, empireRetreat -- Forward-declared functions

function create ()
   local missys = {misn_target}
   if not misn.claim(missys) then
      misn.finish(false)
   end

   -- Intro text
   if tk.yesno( _("Collective Extraction"), _([[As soon as you exit the landing pad, you see Lt. Commander Dimitri waiting for you. He seems a bit more nervous then usual.
    "The commando team has sent us an SOS. They were discovered by the Collective, and now they're under heavy fire. We need you to go and get them out of there. Would you be willing to embark on another dangerous mission?"]]) ) then
      misn.accept()

      mem.misn_stage = 0
      mem.misn_marker = misn.markerAdd( misn_target, "low" )

      -- Mission details
      misn.setTitle(_("Collective Extraction"))
      misn.setReward( fmt.credits( credits ) )
      misn.setDesc( fmt.f(_("Check for survivors on {pnt} in {sys}"), {pnt=misn_target, sys=misn_target_sys} ))
      tk.msg( _("Collective Extraction"), fmt.f(_([["We'll send extra forces to {sys} to try to give you a chance to break through the blockade. You'll have to land on {pnt} and extract our team. Be very careful. This is going to be no walk in the park."]]), {sys=misn_target_sys, pnt=misn_target}) )
      misn.osdCreate(_("Collective Extraction"), {
         fmt.f(_("Fly to {sys}"), {sys=misn_target_sys}),
         fmt.f(_("Land on {pnt}"), {pnt=misn_target}),
         fmt.f(_("Return to {pnt}"), {pnt=misn_base}),
      })

      hook.enter("enter")
      hook.land("land")
   end
end

-- Handles the Collective encounters.
function enter()
   player.allowSave() -- This mission disables saving, which is dangerous. Should be turned back on ASAP.

   local empire_flanking_fleet = {"Empire Pacifier", "Empire Admonisher", "Empire Admonisher", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot"}

   if system.cur() == misn_target_sys and mem.misn_stage == 0 then
      -- Case jumped in before landing
      pilot.clear()
      pilot.toggleSpawn(false)

      local fleetpos1 = vec2.new(20500, 2300)
      local fleetpos2 = vec2.new(20500, 1700)
      local waypoint1 = vec2.new(7500, 9500)
      local waypoint2 = vec2.new(7500, -5500)
      local waypoint12 = vec2.new(1500, 3000)
      local waypoint22 = vec2.new(1500, -500)

      fleet1 = fleet.add( 1,  empire_flanking_fleet, "Empire", fleetpos1, nil, {ai="empire_norun"} )
      fleet2 = fleet.add( 1,  empire_flanking_fleet, "Empire", fleetpos2, nil, {ai="empire_norun"} )
      empireAttack(fleet1)
      empireAttack(fleet2)

      fleet1[1]:comm(fmt.f(_("Okay, {player}, we'll flank the Collective force around the planet and try to draw their fire. You punch right through and land on that planet!"), {player=player.name()}))
      fleet1[1]:taskClear()
      fleet1[1]:moveto(waypoint1, false, false)
      fleet1[1]:moveto(waypoint12, false, false)
      fleet2[1]:taskClear()
      fleet2[1]:moveto(waypoint2, false, false)
      fleet2[1]:moveto(waypoint22, false, false)
      hook.pilot(fleet1[1], "idle", "idle")
      hook.pilot(fleet2[1], "idle", "idle")

      system.markerAdd(waypoint1, _("Empire flanking maneuver"))
      system.markerAdd(waypoint2, _("Empire flanking maneuver"))

      swarm1 = {}
      for i=1,24 do
         swarm1[i] = pilot.add("Drone", "Collective", misn_target, _("Collective Drone") )
      end
      for i=1,16 do
         swarm1[i+24] = pilot.add("Heavy Drone", "Collective", misn_target, _("Collective Heavy Drone") )
      end

      for _,j in ipairs(swarm1) do
         if j:exists() then
            j:control()
            j:setVisplayer()
         end
      end

      hook.timer(0.5, "proximity", {location = misn_target:pos(), radius = 3000, funcname = "idle"})

      mem.misn_stage = 1
      misn.osdActive(2)
   elseif system.cur() == misn_target_sys and mem.misn_stage == 2 then
      -- Case taken off from the planet
      pilot.clear()
      pilot.toggleSpawn(false)

      local pv = player.pos()

      fleet1 = fleet.add( 1,  empire_flanking_fleet, "Empire", pv + vec2.new(-150, 500) )
      fleet2 = fleet.add( 1,  empire_flanking_fleet, "Empire", pv + vec2.new(-150, -500) )
      empireRetreat(fleet1)
      empireRetreat(fleet2)
      fleet1[1]:comm(_("There's too many of them! Fall back! Everyone to the jump point!"))

      -- TODO: Use heavier Collective ships here
      swarm1 = {}
      swarm2 = {}
      swarm3 = {}
      for i = 1,12 do
         swarm1[i] = pilot.add("Drone", "Collective", pv + vec2.new(-3000, 500), _("Collective Drone") )
         swarm2[i] = pilot.add("Drone", "Collective", pv + vec2.new(-3000, 0), _("Collective Drone") )
         swarm3[i] = pilot.add("Drone", "Collective", pv + vec2.new(-3000, -500), _("Collective Drone") )
      end
      for i = 1,8 do
         swarm1[i+12] = pilot.add("Heavy Drone", "Collective", pv + vec2.new(-3000, 500), _("Collective Heavy Drone") )
         swarm2[i+12] = pilot.add("Heavy Drone", "Collective", pv + vec2.new(-3000, 0), _("Collective Heavy Drone") )
         swarm3[i+12] = pilot.add("Heavy Drone", "Collective", pv + vec2.new(-3000, -500), _("Collective Heavy Drone") )
      end

      for i, _ in ipairs(swarm1) do -- Let's be lazy.
         swarm1[i]:setVisplayer()
         swarm2[i]:setVisplayer()
         swarm3[i]:setVisplayer()
      end

      player.allowLand(false, _("You can't land now! Get to the jump point!"))
      misn.osdActive(3)
   elseif mem.misn_stage == 1 then
      -- Case jumped back out without landing
      mem.misn_stage = 0
      misn.osdActive(1)
   elseif mem.misn_stage == 2 then
      -- Case jumped out after landing
      mem.misn_stage = 3
   end
end

-- Preps the Empire ships for attack.
function empireAttack(flt)
   for _, j in ipairs(flt) do
      if j:exists() then
         j:control()
         j:setVisplayer()
         j:follow(flt[1])
      end
   end
end

-- Makes the Empire ships run away.
function empireRetreat(flt)
   for _, j in ipairs(flt) do
      if j:exists() then
         j:control()
         j:setVisplayer()
         j:hyperspace(lmisn.getNextSystem(system.cur(), misn_base_sys))
      end
   end
end

-- Triggered when either Empire fleet is in attack range.
function idle()
   for _,j in ipairs(swarm1) do
      if j:exists() then
         j:control(false)
      end
   end
   for _,j in ipairs(fleet1) do
      if j:exists() then
         j:control(false)
      end
   end
   for _,j in ipairs(fleet2) do
      if j:exists() then
         j:control(false)
      end
   end
end

-- Handles arrival back to base
function land ()
   -- Just landing
   if mem.misn_stage == 1 and spob.cur() == misn_target then
      player.allowSave(false) -- This prevents the player from starting on Eiroik if he dies after taking off.
      player.takeoff()

      -- Some flavour text
      local title = fmt.f(_("Planet {pnt}"), {pnt=misn_target})
      tk.msg( title, _([[The atmosphere once again starts giving your shields a workout as you land. You spend a while flying low until your sensors pick up a reading of possible life forms. The silhouette of the transport ship is barely visible. As you fly closer, it becomes apparent that you arrived too late. Everyone is already dead. You see if you can salvage the readings from their equipment, but it seems like it's completely toasted.]]) )

      -- Add fuel if needed
      if player.jumps() < 2 then
         local _fuel, consumption = player.fuel()
         player.refuel(2 * consumption)
         tk.msg( title, _([[You notice you won't have enough fuel to get back so you salvage some from the wrecked transport ship. Stealing from the dead isn't pleasant business, but if it gets you out alive, you figure it's good enough.]]) )
      end

      tk.msg( title, _([[You spend a while searching until you find a datapad on one of the corpses. Ignoring the stench of burnt flesh you grab it, just as you hear the sirens go off in your ship. Enemy reinforcements! Time to hit the afterburner.
   You've got one, right?]]) )

      -- Add goods
      local c = commodity.new( N_("Datapad"), N_("A dead soldier's datapad.") )
      mem.misn_cargo = misn.cargoAdd( c, 0 )
      mem.misn_stage = 2

   elseif mem.misn_stage == 3 and spob.cur() == misn_base then

      tk.msg( _("Mission Accomplished"), _([[Lt. Commander Dimitri's face cannot hide his sadness as he sees you approach with no commando members.
    "No survivors, eh? I had that gut feeling. At least you were able to salvage something? Good, at least it'll mean they didn't die in vain. Meet me in the bar in a while. We're going to try to process this datapad. It'll hopefully have the final results."]]) )
      misn.cargoRm( mem.misn_cargo )
      var.pop("emp_commando")

      -- Rewards
      player.pay(credits)
      faction.modPlayerSingle("Empire",5)

      emp.addCollectiveLog( _([[You attempted to rescue the commando team on Eiroik, but despite your best efforts, they were already dead by the time you got there. However, you managed to retrieve a datapad from the team's wrecked ship. Lt. Commander Dimitri has asked you to meet him in the bar again in a while.]]) )

      misn.finish(true)
   end
end
