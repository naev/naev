--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 5">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 4</done>
 <notes>
  <campaign>Taiomi</campaign>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[
   Taiomi 05

   Help scavenger go look for the curious drones
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
--local equipopt = require "equipopt"
--local pilotai = require "pilotai"
--local luatk = require "luatk"

-- luacheck: globals enter land scene_spawn scene00 scene01 scene02 fight_spawn fight00 hail_scavenger (Hook functions passed by name)

local reward = taiomi.rewards.taiomi05
local title = _("Missing Drones")
local base, basesys = spob.getS("One-Wing Goddard")
local scenesys = system.get("Bastion")
local fightsys = system.get("Gamel")

--[[
   0: mission started
   1: first cutscene
   2: scavenger goes berserk
   3: return time
--]]
mem.state = 0

function create ()
   if not misn.claim({scenesys, fightsys}) then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   misn.accept()

   -- Choose who died
   local opts = rnd.permutation{ taiomi.younga.name, taiomi.youngb.name }
   mem.died = opts[1]
   mem.alive = opts[2]

   -- Mission details
   misn.setTitle( title )
   misn.setDesc(fmt.f(_("You have been asked to help find {name}, who went missing."),
      {name = mem.died} ) )
   misn.setReward( fmt.credits(reward) )
   mem.marker = misn.markerAdd( scenesys )

   misn.osdCreate( title, {
      fmt.f(_("Search for {name} ({sys})"),{name=mem.died, sys=scenesys})
   } )

   hook.enter( "enter" )
   hook.land( "land" )
end

local prevsys = basesys
function enter ()
   if mem.timer then
      hook.rm( mem.timer )
      mem.timer = nil
   end

   local scur = system.cur()
   if mem.state==0 and scur == scenesys and prevsys==basesys then
      mem.timer = hook.timer( 3, "scene_spawn" )

   elseif mem.state==1 and scur == fightsys and prevsys==scenesys then
      mem.timer = hook.timer( 3, "fight_spawn" )

   end

   prevsys = scur
end

local scavenger
local function spawn_scavenger( pos )
   scavenger = pilot.add( "Drone (Hyena)", "Independent", pos, _("Scavenger Drone") )
   scavenger:setFriendly(true)
   hook.pilot( scavenger, "hail", "hail_scavenger" )
   return scavenger
end

function scene_spawn ()
   local jmp = jump.get( scenesys, basesys )
   spawn_scavenger( jmp )
end

function scene00 ()
   hook.timer( 4, "scene01" )
end

function hail_scavenger( p )
   if mem.state < 2 then
      p:comm(_("Our task is of uttermost importance!"))
   else
      p:comm(_("Aaaaaagh!"))
   end
   player.commClose()
end

function scene01 ()
end

function scene02 ()
end

function fight00 ()
   local jmp = jump.get( scenesys, basesys )
   spawn_scavenger( jmp )
end

function land ()
   local c = spob.cur()
   if c ~= base then
      return
   end

   -- TODO
   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   --vn.na(fmt.f(_([[You bring the {resource} aboard the Goddard and find Scavenger waiting for you.]]),
   --   {resource=resource}))
   s(_([["I see you managed to bring all the needed resources. This will be enough enough for us to start working on our project. I will be outside getting things set up. We may still need something else so make sure to check in after you get some rest."
Scavenger backs out of the Goddard and returns to space.]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( reward )
   taiomi.log.main(_("You collected important resources for the inhabitants of Taiomi."))
   misn.finish(true)
end
