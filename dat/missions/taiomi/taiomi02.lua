--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 2">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 1</done>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</mission>
--]]
--[[
   Taiomi 02

   Player is asked to board convoys
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
local der = require 'common.derelict'
local pilotai = require "pilotai"

local reward = taiomi.rewards.taiomi02
local title = _("Information Hunting")
local base, basesys = spob.getS("One-Wing Goddard")
local N = 2

local convoysys = {
   {
      sys = system.get( "Delta Pavonis" ),
      fct = faction.get("Empire"),
      escorts = {
         "Empire Lancelot",
         "Empire Lancelot",
         "Empire Lancelot",
         "Empire Lancelot",
      },
   },
   {
      sys = system.get( "Father's Pride" ),
      fct = faction.get("Soromid"),
      escorts = {
         "Soromid Reaver",
         "Soromid Reaver",
         "Soromid Reaver",
      },
   },
   {
      sys = system.get( "Doranthex" ),
      fct = faction.get("Dvaered"),
      escorts = {
         "Dvaered Vendetta",
         "Dvaered Vendetta",
         "Dvaered Vendetta",
         "Dvaered Vendetta",
      }
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
      fmt.f(_("Board convoy ships ({n}/{total})"),{n=mem.state, total=N}),
      fmt.f(_("Return to the {spobname} ({spobsys})"), {spobname=base, spobsys=basesys}),
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

   local desc = _("You have agreed to help the robotic citizens of Taiomi to obtain important information regarding the hypergates. The information can be found on convoys that tend to cross the following systems:")
   for k,v in ipairs(convoysys) do
      desc = desc .. "\n" .. fmt.f(_("   {sysname}"),{sysname=v.sys})
   end
   misn.setDesc( desc )
   misn.setReward(reward)

   for k,v in ipairs(convoysys) do
      misn.markerAdd( v.sys )
   end

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

   -- Create the mule and change faction
   table.insert( fleet, pilot.add( "Mule", "Trader", startpos ) )
   for k,v in ipairs(cursys.escorts) do
      local p = pilot.add( v, cursys.fct, startpos )
      p:setLeader( fleet[1] )
      table.insert( fleet, p )
   end

   -- First ship is the convoy ship that has special stuff
   fleet[1]:rename(_("Convoy"))
   fleet[1]:setFaction(cursys.fct)
   fleet[1]:setHilight(true)
   pilotai.hyperspace( fleet[1], endpos )
   local fm = fleet[1]:memory()
   fm.norun = true
   fm.aggressive = false
   hook.pilot( fleet[1], "board", "board_convoy" )
end

function board_convoy( p )
   vn.clear()
   vn.scene()
   vn.sfx( der.sfx.board )
   vn.music( der.sfx.ambient )
   vn.transition()
   if mem.state == 0 then
      vn.na(_([[You board the ship and are able to obtain some hypergate information from the convoy systems.]]))
      vn.func( function ()
         local c = commodity.new( N_("Hypergate Information"), N_("Information relating to the hypergate construction and planning.") )
         mem.cargo = misn.cargoAdd( c, 0 )
      end )
   else
      vn.na(_([[You board the ship and are able to more hypergate information from the convoy systems.]]))
      if mem.state == N then
         vn.na(_([[It seems like you have collected all the necessary data and can return to Scavenger.]]))
      end
   end
   vn.sfx( der.sfx.unboard )
   vn.run()
   player.unboard()

   -- Store faction which will be used for next missions
   var.push( "taiomi_convoy_fct", p:faction():nameRaw() )

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

   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   vn.na(_([[You board the Goddard and and find Scavenger waiting for you.]]))
   s(_([["I can sense you have managed to collect the necessary data. Let me analyze it hastily."]]))
   s(_([[Scavenger takes the data and there is a brief flicker of their lights.]]))
   s(_([["Very interesting. While the documents contain mainly mundane details that aren't particularly of importance to us, there is a lead to one of the experimental locations. I believe it should be possible to find more in-depth construction details there."]]))
   s(_([["I will be outside preparing our next steps."
Scavenger backs out of the Goddard and returns to space.]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( reward )
   taiomi.log.main(_("You helped the robotic inhabitants of Taiomi collect important information regarding the hypergates by obtaining it from convoys."))
   misn.finish(true)
end
