--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Edergast Pirate Ambush">
 <location>land</location>
 <chance>100</chance>
 <spob>Edergast</spob>
 <unique />
</event>
--]]
local vn = require "vn"
local fmt = require "format"
local fleet = require "fleet"
local pilotai = require "pilotai"
local tut = require "common.tutorial"
local lmisn = require "lmisn"
local th = require "common.treasure_hunt"
local poi = require "common.poi"

local SPB, SYS = spob.getS("Edergast")

function create ()
   evt.finish(false) -- disabled for now
   if not evt.claim( SYS ) then return evt.finish() end

   local ambush = false

   vn.reset()
   vn.scene()
   vn.transition()
   vn.na(_([[You land and do your routine check, when you seem to pick up what seems to be some construction on the surface. Do you wish to go investigate?]]))
   vn.menu{
      {_([[Investigate.]]), "01_investigate"},
      {_([[Let it be.]]), "01_leave"},
   }

   vn.label("01_leave")
   vn.done()

   vn.label("01_investigate")
   vn.na(fmt.f(_([[You get out your gear and go to check out the site. Eventually you find what seems to be some sort of small construction made of an industrial shipping container. You approach when suddenly you are interrupted by {shipai}.]]),
      {shipai=tut.ainame()}))

   local sai = tut.vn_shipai()
   vn.appear( sai, tut.shipai.transition )
   sai(_([["I hate to bother you on your nature excursion, however, it seems like a signal was emitted from the planet and the ships sensors pick up incoming hostile ships. You may want to consider returning."]]))
   vn.disappear( sai, tut.shipai.transition )
   vn.na(_([[You haul your ass back to your ship, and quickly ascend your way outside the gravitational pull. As you catch your breath, you notice hostiles coming in hot. Time to take evasive manoeuvres.]]))
   vn.done( tut.shipai.transition )
   vn.run()

   if not ambush then
      return evt.finish(false)
   end

   mem.hook_enter = hook.enter("enter")
   hook.land("land")
   player.takeoff()
end

local pirates
function enter ()
   if system.cur() ~= SYS then evt.finish(false) end

   pilot.clear()
   pilot.toggleSpawn(false)
   local ships = {
      "Pirate Starbridge",
      "Pirate Admonisher",
      "Pirate Ancestor",
      "Pirate Shark",
      "Pirate Shark",
   }
   pirates = fleet.add( 1, ships, "Marauder", SPB:pos() + vec2.newP( 4000, rnd.angle() ) )
   for k,p in ipairs(pirates) do
      p:setHostile(true)
      hook.pilot( p, "disable", "pir_check" )
      hook.pilot( p, "death", "pir_check" )
   end
   pilotai.guard( pirates, SPB:pos() + vec2.newP( 200, rnd.angle() ) )

   -- Don't want to allow landing again
   SPB:landDeny(true,_("You can not land while being ambushed."))

   hook.timer( 5, "" )
end

function pir_check ()
   for k,p in ipairs(pirates) do
      if p:exists() and (not p:disabled()) then
         return
      end
   end

   -- Re-allow landing
   SPB:landDeny(false)
   player.msg(fmt.f(_("You have cleared the ambush at {spb}. It should be safe to land again."),
      {spb=SPB}))
   hook.rm( mem.hook_enter )
   evt.save() -- We'll track it saved
   pilot.toggleSpawn(true) -- Spawn again
end

function land ()
   if spob.cur()~=SPB then return end
   if player.misnActive("Treasure Hunt") and mem.gave_map then return end

   local takemap = false

   vn.reset()
   vn.scene()
   if mem.gave_map_once then
      vn.na(_([[You return to the pirate lair.]]))
   else
      vn.na(_([[You go back to the industrial shipping container now that the pirates are no longer trouble and after a few tries are able to break down the door.]]))
      vn.na(_([[It looks like a makeshift pirate lair, although it's hard to tell with all the junk and half-eaten space rations lying around. Don't they know this will attract all sorts of pests?]]))
      vn.na(_([[Happy that you are in your atmospheric suit, you wait through the rubbish and shift through the room. Other than a dubious generic "PIRATE MANIFESTO" about becoming the pirate king, which is something you never heard of, you find some data matrices, and something that looks like a treasure map.]]))
      local MATRIX_AMOUNT = 2
      local poi_reward = poi.data_str(MATRIX_AMOUNT)
      vn.na(fmt.reward(poi_reward))
      vn.func( function ()
         poi.data_give(MATRIX_AMOUNT)
      end )
   end
   vn.na(_([[Do you wish to make a copy of the treasure map again?]]))
   vn.menu{
      {_("Make a copy."), "copy"},
      {_("Maybe later."), "nocopy"},
   }

   vn.label("nocopy")
   vn.na(_([[You decide to not make a copy. Better to be safe than sorry. You can always come back if you change your mind.]]))
   vn.done()

   vn.label("copy")
   vn.func( function() takemap=true end )
   vn.sfxBingo()
   vn.na(_([[You make a copy of the treasure map. Now to find it.]]))

   vn.run()

   -- Didn't want to take the map.
   if not takemap then
      return
   end

   -- Set up hook
   if not mem.cust_hook then
      mem.cust_hook  = hook.custom( "edergast_pirate_ambush_found" )
   end

   -- Give map
   local goallst = lmisn.getSysAtDistance( SYS, 10, 15, th.good_sys )
   local goal = goallst[ rnd.rnd(1,#goallst) ]
   local spb = {}
   for k,p in ipairs(goal:spobs()) do
      if th.good_spob(p) then
         table.insert(spb,p)
      end
   end
   spb = spb[rnd.rnd(1,#spb)] -- Should exist as we checked when getting goal
   local name = fmt.f(_("Map from {spb}"), {spb=SPB})
   mem.gave_map = true
   mem.gave_map_once = true
   return {
      spb   = spb,
      goal  = goal,
      start = SYS,
      name  = name,
      seed  = rnd.rnd(1,2^30),
      trigger = "edergast_pirate_ambush_found",
   }
end

function edergast_pirate_ambush_found( aborted )
   if aborted then
      mem.gave_map = false
      return
   end

   -- TODO better reward
   local REWARD = outfit.get("Berserk Chip")

   vn.reset()
   vn.scene()
   vn.transition()
   vn.na(_([[As you approach what you believe to be the location marked in the treasure map, you scanners pick up something in low orbit.]]))
   vn.na(fmt.f(_([[You slowly approach the object while hoping it's not another ambush. Luckily enough, you pick up nothing strange as you near what seems to be some sort of capsule orbiting {spob}.]]),
      {spob=spob.cur()}))
   vn.na(fmt.f(_([[You manage to pry open the capsule, and inside you find some weird pirate king fanfics, but more importantly, you see a {reward}. You flip through some of the fanfic, it doesn't seem to be too original or interesting, so you leave that, the {reward} on the other hand seems like something you should come with you.]]),
      {reward=REWARD}))
   vn.na(fmt.f(_([[You take the {reward}, but leave the fanfic for the next person to find the capsule. You can imagine they'll be very happy.]]),
      {reward=REWARD}))
   vn.sfxVictory()
   vn.func( function ()
      player.outfitAdd(REWARD)
   end )
   vn.na(fmt.reward(REWARD))
   vn.run()

   th.log(fmt.f(_([[You followed a treasure map you found on {start} to {goal}, and found a nice reward of {reward}.]]),
      {start=SPB, goal=system.cur(), reward=REWARD}))

   -- And done here :D
   evt.finish(true)
end
