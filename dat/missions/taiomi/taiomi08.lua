--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 8">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 7</done>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</mission>
--]]
--[[
   Taiomi 08

   Player has to raid pirate convoys
   * bonus points for looting materials
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
local der = require 'common.derelict'
local pilotai = require "pilotai"

local reward = taiomi.rewards.taiomi08
local title = _("Just Pirate Business")
local base, basesys = spob.getS("One-Wing Goddard")
local N = 2

local pir_escorts = {
   "Mule",
   "Pirate Admonisher",
   "Pirate Shark",
   "Pirate Shark",
   "Pirate Hyena",
}
local convoysys = {
   {
      sys = system.get("Haven"),
      escorts = pir_escorts,
   },
   {
      sys = system.get("Titus"),
      escorts = pir_escorts,
   },
}

--[[
   0: mission started
   1: first convoy scanned
   2: second convoy scanned
--]]
mem.state = 0

local function osd ()
   misn.osdCreate( title, {
      fmt.f(_("Raid pirate transports ({n}/{total})"),{n=mem.state, total=N}),
      fmt.f(_("Return to {spobname} ({spobsys})"), {spobname=base, spobsys=basesys}),
      fmt.f(_("[Optional] Steal {cargo}"), {cargo=mem.cargo}),
   } )

   if mem.state >= N then
      misn.osdActive(2)
   end
end

function create ()
   local claimsys = {}
   for k,v in ipairs(convoysys) do
      table.insert( claimsys, v.sys )
   end
   if not misn.claim( claimsys ) then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   misn.accept()

   -- Mission details
   misn.setTitle( title )

   local desc = _("You have agreed to help Scavenger search for a dealer to obtain supplies for a large hypergate-like construction. Raiding pirate convoys in the following systems may lead to hints and useful resources:")
   for k,v in ipairs(convoysys) do
      desc = desc .. "\n" .. fmt.f(_("   {sysname}"),{sysname=v.sys})
   end
   misn.setDesc( desc )
   misn.setReward(reward)

   for k,v in ipairs(convoysys) do
      misn.markerAdd( v.sys )
   end

   -- Special cargo
   mem.cargo = commodity.new(N_("High-Tech Contraband"),N_("An assortment of various high-tech equipment and gadgets that seem to be sourced through all sorts of illegal mechanisms."))
   mem.cargo:illegalto{ "Empire", "Soromid", "Dvaered", "Za'lek", "Sirius" }

   osd()

   hook.enter( "enter" )
   hook.land( "land" )
end

local heartbeat_hook, fleet, cursys
function enter ()
   if heartbeat_hook then
      hook.rm( heartbeat_hook )
      heartbeat_hook = nil
      fleet = nil
   end

   -- Only interested at first
   if mem.state >= N then
      return
   end

   -- Determine what system we are in
   local csys = system.cur()
   cursys = nil
   for k,v in ipairs(convoysys) do
      if v.sys == csys then
         cursys = v
         break
      end
   end
   if not cursys then
      return
   end

   fleet = {}
   heartbeat()
end

function spawn_fleet ()
   fleet = {}

   local j = system.cur():jumps()
   j = rnd.permutation( j )
   local startpos = j[1]
   local endpos = j[2]

   -- Create the pirates
   local fct = faction.get("Pirate")
   for k,v in ipairs(cursys.escorts) do
      local p = pilot.add( v, fct, startpos )
      table.insert( fleet, p )
   end
   for i=2,#fleet do
      fleet[i]:setLeader( fleet[1] )
   end

   -- First ship is the convoy ship that has special stuff
   fleet[1]:rename(_("Pirate Convoy"))
   fleet[1]:setHilight(true)
   fleet[1]:cargoRm( "all" )
   fleet[1]:cargoAdd( mem.cargo, fleet[1]:cargoFree() )
   pilotai.hyperspace( fleet[1], endpos )
   local fm = fleet[1]:memory()
   fm.norun = true
   fm.aggressive = false
   hook.pilot( fleet[1], "board", "board_convoy" )
end

function board_convoy( _p )
   vn.clear()
   vn.scene()
   vn.sfx( der.sfx.board )
   vn.music( der.sfx.ambient )
   vn.transition()
   if mem.state == 0 then
      vn.na(_([[You board the ship and first quickly download the system log information. It seems to have quite a lot of details of pirate convoy operations done in the vicinity.]]))
      vn.func( function ()
         local c = commodity.new( N_("Pirate Convoy Logs"), N_("Logging information containing lots of details about pirate convoy operations near Taiomi.") )
         mem.loot = misn.cargoAdd( c, 0 )
      end )
   else
      vn.na(_([[You board the ship and download even more system log information.]]))
      if mem.state == N then
         vn.na(_([[It seems like you have collected all the necessary data and can return to Scavenger.]]))
      end
   end
   vn.run()

   mem.state = mem.state+1
   osd() -- Update OSD

   if mem.state >= N then
      misn.markerRm()
      misn.markerAdd( base )

      if heartbeat_hook then
         hook.rm( heartbeat_hook )
         heartbeat_hook = nil
         fleet = nil
      end
   end
end

function heartbeat ()
   local nfleet = {}
   for k,v in ipairs(fleet) do
      if v:exists() then
         table.insert( nfleet, v )
      end
   end
   fleet = nfleet

   -- Spawn new fleet
   if #fleet <= 0 then
      local t = 10+rnd.rnd()*10
      hook.timer( t, "spawn_fleet" )
      heartbeat_hook = hook.timer( t+1, "heartbeat" )
   else
      heartbeat_hook = hook.timer( 1, "heartbeat" )
   end
end

function land ()
   if mem.state < N or spob.cur()~=base then
      return
   end

   local cargo_owned = player.fleetCargoOwned( mem.cargo )
   local full_reward = reward + cargo_owned*1000

   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   vn.na(_([[You board the Goddard and and find eagerly Scavenger waiting for you.]]))
   s(_([["Were the pirate convoys accessible?"]]))
   if cargo_owned > 0 then
      vn.na(_("You explain how you were able to raid the convoys and obtain parts of the ship logs detailing smuggling operations, and recover large amounts of cargo that may be useful for Scavenger's plan."))
      s(_([["Excellent. That will be most useful for the construction."]]))
   else
      vn.na(_("You explain how you were able to raid the convoys and obtain parts of the ship logs detailing smuggling operations."))
   end
   s(_([["The information looks promising. I will have to do an in-depth analysis afterwards. Meet me outside, I will be planning our next steps."]]))
   if cargo_owned > 0 then
      s(_([["Let me provide you with a reward for your services. I should be able to make an extra allowance and provide a bonus for the cargo you were able to recover."]]))
   else
      s(_([["Let me provide you with a reward for your services."]]))
   end
   vn.sfxVictory()

   vn.na( fmt.reward(full_reward) )
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( full_reward )
   local log = _("You raided pirate convoys near Taiomi for information on smuggling operations.")
   if cargo_owned > 0 then
      player.fleetCargoRm( mem.cargo, cargo_owned )
   else
      log = log .. fmt.f(_(" You were also able to recover large amounts of {cargo} to help accelerate the construction of the hypergate."),{cargo=mem.cargo})
   end
   taiomi.log.main( log )
   misn.finish(true)
end
