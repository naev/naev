--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Black Cat's Gift">
 <unique/>
 <location>enter</location>
 <chance>5</chance>
 <cond>
   if not player.misnDone("Black Cat") then
      return false
   end
   local pir = require "common.pirate"
   return pir.systemPresence() > 50
 </cond>
</event>
--]]
local th = require "common.treasure_hunt"
local vn = require "vn"
local lmisn = require "lmisn"
local fmt = require "format"
local pilotai = require "pilotai"
local audio = require 'love.audio'

local meow = audio.newSource( "snd/sounds/meow.ogg" )

local REWARD = outfit.get("Corsair Engine")

function create ()
   -- Player already has reward, so no need for this event
   if player.outfitNum(REWARD) > 0 then
      return evt.finish(true)
   end

   -- Only works when can inclusively claim the system
   if not evt.claim( system.cur(), true ) then
      return evt.finish(false)
   end

   hook.timer( 15+30*rnd.rnd(), "spawn")
   hook.enter("finish")
end

local plt
function spawn ()
   local fct = faction.dynAdd("Wild Ones", "black_cat", _("Wild Ones"), {clear_allies=true, clear_enemies=true} )
   local pos = player.pos()
   plt = pilot.add("Pirate Hyena", fct, pos, _("Hairball 5"), { stealth = true } )
   plt:outfitAddIntrinsic("Escape Pod")
   plt:setFriendly()
   plt:hailPlayer()
   plt:control(true)
   plt:follow( player.pilot() )

   -- Move closer to the player, so they get discovered
   local dis = plt:detectedDistance()
   plt:setPos( pos + vec2.newP( dis*0.5, rnd.angle() ) )

   hook.pilot( plt, "hail", "hail" )
   hook.pilot( plt, "attacked", "goaway" )
   hook.timer( 30, "goaway" )
end

local goingaway = false
function goaway ()
   if not goingaway and plt:exists() then
      plt:control(false)
      pilotai.clear(plt)
      goingaway = true
   end
end

function hail ()
   local accepted = false
   player.commClose()
   if goingaway then
      plt:comm( player.pilot(), _("No response.") )
      return
   end

   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(fmt.f(_([[You open a comm channel with {pilot}, however, nothing seems to appear onscreen despite the video stream being open.]]),
      {pilot = plt:name()}))
   vn.sfx( meow )
   vn.na(_([[What was that? It seems like you are receiving a data package. Accept?]]))
   vn.menu{
      {_([[Accept.]]), "accept"},
      {_([[Decline.]]), "decline"},
   }

   vn.label("decline")
   vn.sfx( meow )
   vn.na(_([[The communication seems to cut abruptly off. What was that all about?]]))
   vn.done()

   vn.label("accept")
   vn.func( function () accepted = true end )
   vn.sfx( meow )
   vn.na(_([[You accept the data packet. It seems to be some sort of map that leads to something.]]))
   vn.sfxBingo()
   vn.na(fmt.reward(_("Treasure Map")))
   vn.sfx( meow )
   vn.na(_([[With the transmission completed, the ship closes communication and seems to go on its way. Looks like it's time to hunt some treasure!]]))

   vn.run()

   goaway()

   if not accepted then return end

   -- Give map
   local sys = system.cur()
   local goallst = lmisn.getSysAtDistance( sys, 8, 11, th.good_sys )
   local goal = goallst[ rnd.rnd(1,#goallst) ]
   local spb = {}
   for k,p in ipairs(goal:spobs()) do
      if th.good_spob(p) then
         table.insert(spb,p)
      end
   end
   spb = spb[rnd.rnd(1,#spb)] -- Should exist as we checked when getting goal
   local name = _("Map from Meow?")
   mem.gave_map = true
   th.give_map_from{
      spb   = spb,
      goal  = goal,
      start = sys,
      name  = name,
      seed  = rnd.rnd(1,2^30),
      trigger = "black_cat_gift_found",
   }
   hook.custom( "black_cat_gift_found", "black_cat_gift_found" )
   evt.save(true)
end

function finish ()
   if not mem.gave_map then
      evt.finish(false)
   end
end

function black_cat_gift_found( aborted )
   if aborted then
      mem.gave_map = false
      evt.finish(false)
      return
   end

   vn.reset()
   vn.scene()
   vn.transition()
   vn.na(_([[You reach the location indicated by the map and seem to pick up something in low orbit on your scanners.]]))
   vn.na(_([[You approach the object, which seems to be some sort of cache, and clamp on to it with your ship. Donning your space suit, you venture out to see what you can find.]]))
   vn.na(fmt.f(_([[Inside, there seems to be a large #g{reward}#0. What, it seems to be covered in something, something... fuzzy?

You carefully get closer to have a look at what it is. Wait, is it cat hair? Black cat hair?]]),
      {reward=REWARD}))
   vn.na(_([[Stifling the trauma of how long it took you to clean your ship after your last rendezvous with a black cat, you haul the outfit back to your ship. Hope you have enough lint rollers to clean it up.]]))
   vn.sfxVictory()
   vn.func( function ()
      player.outfitAdd(REWARD)
   end )
   vn.na(fmt.reward(REWARD))
   vn.run()

   th.log(fmt.f(_([[You were given a treasure map by a weird ship and followed it to {goal} to found a nice reward of {reward}.]]),
      {goal=system.cur(), reward=REWARD}))

   -- And done here :D
   evt.finish(true)
end
