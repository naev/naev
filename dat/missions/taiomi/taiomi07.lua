--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 7">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 6</done>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</mission>
--]]
--[[
   Taiomi 07

   Player has to destroy a large fleet in Gamel
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
local fleet = require "fleet"
local pilotai = require "pilotai"
local lmisn = require "lmisn"

local reward = taiomi.rewards.taiomi07
local title = _("Patrol Elimination")
local base, basesys = spob.getS("One-Wing Goddard")
local fightsys = system.get("Gamel")
local entersys = system.get("Dune")
local exitsys = system.get("Bastion")

--[[
   0: mission started
   1: destroyed patrol
   2: calmed down Scavenger
--]]
mem.state = 0

function create ()
   if not misn.claim( {fightsys}, true) then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   misn.accept()

   -- Mission details
   misn.setTitle( title )
   misn.setDesc(fmt.f(_([[Destroy a patrol in the {sys} system.]]),
      {sys = fightsys} ))
   misn.setReward( fmt.credits(reward) )
   mem.marker = misn.markerAdd( fightsys )

   misn.osdCreate( title, {
      fmt.f(_("Destroy the patrol in {sys}"),{sys=fightsys}),
      fmt.f(_("Return to {base} ({basesys})"),{base=base, basesys=basesys}),
   } )

   hook.enter( "enter" )
   hook.land( "land" )

   hook.custom( "taiomi_philosopher", "taiomi_philosopher" )
end

function taiomi_philosopher ()
   misn.osdCreate( title, {
      fmt.f(_("Destroy the patrol in {sys}"),{sys=fightsys}),
      fmt.f(_("Return to {base} ({basesys})"),{base=base, basesys=basesys}),
      _("Save Scavenger if possible"),
   } )
end

local plts = {}
function enter ()
   if system.cur() ~= fightsys or mem.state ~= 0 then
      return
   end

   -- We'll make a fancy caravan
   local fct = var.peek( "taiomi_convoy_fct" ) or "Empire"
   local flt
   if fct== "Soromid" then
      flt = {
         "Soromid Ira",
         "Soromid Odium",
         "Soromid Odium",
         "Soromid Reaver",
         "Soromid Reaver",
         "Soromid Reaver",
         "Soromid Reaver",
      }
   else
      flt = {
         "Empire Hawking",
         "Empire Admonisher",
         "Empire Admonisher",
         "Empire Lancelot",
         "Empire Lancelot",
         "Empire Lancelot",
         "Empire Lancelot",
      }
   end

   local enterjmp = jump.get( fightsys, entersys )
   local exitjmp = jump.get( fightsys, exitsys )

   -- Spawn the patrol
   plts = fleet.add( 1, flt, faction.get(fct), enterjmp )
   plts[1]:setHilight(true)
   for k,p in ipairs(plts) do
      local m = p:memory()
      m.norun = true
      pilotai.hyperspace( p, exitjmp )

      hook.pilot( p, "death", "patrol_death" )
      hook.pilot( p, "jump", "patrol_jump" )
   end
end

function patrol_jump ()
   lmisn.fail(_("some of the patrol got away!"))
end

function patrol_death ()
   local nplts = {}
   local hashilight = false
   for k,p in ipairs(plts) do
      if p:exists() then
         table.insert( nplts, p )
         if p:flags("hilight") then
            hashilight = true
         end
      end
   end
   plts = nplts

   if #plts <= 0 then
      mem.state = 1 -- next state
      misn.osdActive(2)
      misn.markerMove( base )
   else
      -- Rehighlight as necessary
      if not hashilight then
         plts[1]:setHilight(true)
      end
   end
end

function land ()
   if mem.state ~= 2 then
      return -- Not done yet
   end

   vn.clear()
   vn.scene()
   local e = vn.newCharacter( taiomi.vn_elder() )
   vn.transition()
   vn.na(_([[You find Elder waiting for you in the Goddard hangar bay.]]))
   e(fmt.f(_([["How was the fighting? Cleaning {sys} is an important first step for our security."]]),
      {sys=fightsys}))
   vn.menu{
      {_([["It was a cakewalk."]]), "01_cakewalk"},
      {_([["Is there no other way?"]]), "01_other"},
      {_([[…]]), "01_cont"},
   }

   vn.label("01_cakewalk")
   e(_([["It seems that the best way to deal with humans is another human."]]))
   vn.jump("01_cont")

   vn.label("01_other")
   e(_([["There is no other option. Our numbers dwindle, picked off by stray ships. Only establishing a secure zone will allow us to survive."]]))
   vn.jump("01_cont")

   vn.label("01_cont")
   e(_([["I would hope that you have bought us enough time for a while. However, usually space is not kind to us. We may need to move again soon."]]))
   e(_([["I am not used to dealing with humans, but Philosopher told me it was customary in capitalistic societies to provide rewards in exchange for services. Seems like a waste of resources, but I shall comply. Please take some credits we scrounged up from derelict ships."]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   e(_([["I shall be outside planning our next steps."]]))
   vn.na(_([[Elder scratches their way out of the Goddard.]]))
   vn.run()

   player.pay( reward )
   taiomi.log.main(fmt.f(_("You destroyed {num} ships in the {sys} as ordered by Elder."),{num=37, sys=fightsys}))
   misn.finish(true)
end
