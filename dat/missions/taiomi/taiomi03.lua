--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 3">
 <unique />

  <chance>0</chance>
  <location>None</location>
  <done>Taiomi 2</done>

 <notes>
  <campaign>Taiomi</campaign>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[
   Taiomi 03

   Player has to infiltrate a laboratory.
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
local mg = require "minigames.stringguess"

-- luacheck: globals enter land (Hook functions passed by name)

local reward = taiomi.rewards.taiomi03
local title = _("Escaping Taiomi")
local base, basesys = spob.getS("One-Wing Goddard")

--[[
   0: mission started
   1: visited laboratory
--]]
mem.state = 0

function create ()
   mem.lab, mem.labsys = taiomi.laboratory()

   misn.accept()

   -- Mission details
   misn.setTitle( title )
   misn.setDesc(fmt.f(_("You have been asked to fly to {lab} in the {labsys} system to recover important documents regarding the technical details of the hypergates.."),{lab=mem.lab,labsys=mem.labsys}))
   misn.setReward( fmt.credits(reward) )
   misn.markerAdd( mem.lab )

   misn.osdCreate( title, {
      fmt.f(_("Infiltrate the laboratory at {spobname} ({spobsys})"),{spobname=mem.lab, spobsys=mem.labsys}),
      fmt.f(_("Return to the {spobname} ({spobsys})"), {spobname=base, spobsys=basesys}),
   } )

   hook.enter( "enter" )
   hook.land( "land" )
end

function enter ()
   if mem.state > 0 or system.cur() ~= mem.labsys then
      return
   end
   -- Allow the player to land always, Soromid spob is actually not landable usually
   mem.lab:landOverride( true )
end

local function land_lab ()
   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_([[]]))
   mg.vn()
   vn.func( function ()
   --[[
      if mg.completed() then
      else
      end
   --]]
   end )
   vn.run()
end

local function land_done ()
   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   vn.na(_([[You board the Goddard and and find Scavenger waiting for you.]]))
   s(_([["TODO"]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( reward )
   taiomi.log.main(_("You stole some important documents detailing the inner workings of the hypergates for the inhabitants of Taiomi."))
   misn.finish(true)
end

function land ()
   local c = spob.cur()
   if mem.state == 0 and c == mem.lab then
      land_lab()
   elseif mem.state >= 1 and c == base then
      land_done()
   end
end
