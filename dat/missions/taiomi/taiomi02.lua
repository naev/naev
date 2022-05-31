--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 2">
 <flags>
  <unique />
 </flags>
 <avail>
  <chance>0</chance>
  <location>None</location>
  <done>Taiomi 1</done>
 </avail>
 <notes>
  <campaign>Taiomi</campaign>
  <tier>2</tier>
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

-- luacheck: globals enter heartbeat land spawn_fleet board_convoy (Hook functions passed by name)

local reward = taiomi.rewards.taiomi02
local title = _("Escaping Taiomi")
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
   misn.accept()

   -- Mission details
   misn.setTitle( title )
   misn.setDesc( _("You have agreed to help the robotic citizens of Taiomi to obtain important information regarding the hypergates.") )
   misn.setReward( fmt.credits(reward) )

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

   heartbeat()
end

function spawn_fleet ()
   fleet = {}

   local j = system.cur():jumps()
   j = rnd.permutation( j )
   local startpos = j[1]
   local endpos = j[2]

   local function padd( shipname, boss )
      local p = pilot.add( cursys.fct, shipname, startpos, endpos )
      if boss then
         p:setLeader( boss )
      else
         p:control()
         p:jump( endpos )
      end
      return p
   end

   table.insert( fleet, padd( "Mule" ) )
   for k,v in ipairs(cursys.escorts) do
      table.insert( fleet, padd( v, fleet[1] ) )
   end

   -- First ship is the convoy ship that has special stuff
   hook.pilot( fleet[1], "board", "board_convoy" )
end

function board_convoy ()
   -- TODO do boarding stuff
   player.unboard()

   mem.state = mem.state+1
   osd() -- Update OSD

   if mem.state >= N then
      misn.markerRm()
      misn.markerAdd( base )
   end
end

function heartbeat ()
   if not fleet then
      hook.timer( 10+rnd.rnd()*10, "spawn_fleet" )
   end

   hook.timer( 1, "heartbeat" )
end

function land ()
   if mem.state < N or not spob.cur()==base then
      return
   end

   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   vn.na(_([[You board the Goddard and by the time you get off the ship Scavenger is waiting for you.]]))
   s(_([[""]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.na(_([[Scavenger once again elegantly exits the ship, leaving you to yourself.]]))
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( reward )
   taiomi.log.main(_("TODO"))
   misn.finish(true)
end
