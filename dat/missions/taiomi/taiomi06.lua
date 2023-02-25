--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 6">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 5</done>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</mission>
--]]
--[[
   Taiomi 06

   Player has to destroy a number ships in Bastion
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"

local reward = taiomi.rewards.taiomi06
local title = _("Human Extermination")
local base, basesys = spob.getS("One-Wing Goddard")
local fightsys = system.get("Bastion")

local NUMBER_SHIPS = 23 -- Number of ships to kill, prime on purpose

local pilots = {} -- stores damage done to ships, reset on enter
mem.killed = 0 -- number of ships killed

local function osd ()
   local left = NUMBER_SHIPS - mem.killed
   misn.osdCreate( title, {
      fmt.f(_("Destroy {left} ships in {sys}"),{sys=fightsys, left=left}),
      fmt.f(_("Return to {base} ({basesys})"),{base=base, basesys=basesys}),
   } )
   if left <= 0 then
      misn.osdActive(2)
   end
end

function create ()
   if not misn.claim( {fightsys}, true ) then
      warn(_("Unable to claim system that should be claimable!"))
      misn.finish(false)
   end

   misn.accept()

   -- Mission details
   misn.setTitle( title )
   misn.setDesc(fmt.f(_([[Destroy {num} ships in the {sys} system.

Only ships to which you or your fleet deal over 50% damage will count towards the number of ships destroyed.]]),
      {num = NUMBER_SHIPS, sys = fightsys} ))
   misn.setReward( fmt.credits(reward) )
   mem.marker = misn.markerAdd( fightsys )

   osd()

   hook.enter( "enter" )
   hook.land( "land" )
end

-- Only count as player kill if player did >50% damage
function pilot_death( p, _attacker )
   if p:withPlayer() then
      return
   end

   local id = p:id()
   local pt = pilots[id] or { player=0, nonplayer=0}

   if pt.player >= pt.nonplayer then
      mem.killed = mem.killed + 1
      osd()
      if (mem.killed >= NUMBER_SHIPS) then
         mem.marker = misn.markerAdd( base )
      end
   end
end

-- Compare damage of player vs non-player
function pilot_attacked( p, attacker, dmg )
   if not attacker or not attacker:exists() then
      return
   end
   if p:withPlayer() then
      return
   end

   local id = p:id()
   local pt = pilots[id] or { player=0, nonplayer=0}
   if attacker:withPlayer() then
      pt.player = pt.player + dmg
   else
      pt.nonplayer = pt.nonplayer + dmg
   end
   pilots[id] = pt
end

function enter ()
   pilots = {}

   -- Only set up hooks when necessary
   if system.cur() == fightsys then
      mem.hook_death = hook.pilot( nil, "death", "pilot_death" )
      mem.hook_attacked = hook.pilot( nil, "attacked", "pilot_attacked" )
   elseif mem.hook_death then
      hook.rm( mem.hook_death )
      hook.rm( mem.hook_attacked )
      mem.hook_death = nil
      mem.hook_attacked = nil
   end
end

function land ()
   if mem.killed < NUMBER_SHIPS then
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
   taiomi.log.main(fmt.f(_("You destroyed {num} ships in the {sys} system as ordered by Elder."),{num=NUMBER_SHIPS, sys=fightsys}))
   misn.finish(true)
end
