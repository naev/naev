--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Lucas 01">
 <unique />
 <chance>20</chance>
 <location>Bar</location>
 <cond>
   -- Required to bribe Maanen's Moon
   if faction.get("Empire"):playerStanding() &lt; 0 then
      return false
   end

   -- Should start at a normal planet
   local t = spob.cur():tags()
   if t.refugee or t.station or t.restricted then
      return false
   end

   -- Within 6 jumps of a refugee spob
   local dist = math.huge
   for k,v in ipairs(spob.getAll()) do
      if v:tags().refugee then
         if v:system():jumpDist() &lt; 6 then
            return require("misn_test").reweight_active()
         end
      end
   end
   return false
 </cond>
 <notes>
  <tier>1</tier>
  <campaign>Lucas</campaign>
 </notes>
</mission>
--]]
--[[
   Refugee Family

   A person who escaped the Incident is trying to get in touch with their family.
--]]
local fmt = require "format"
local vn = require "vn"
local lmisn = require "lmisn"
local mg = require "minigames.flip"
local lcs = require "common.lucas"

local title = _([[Refugee Family]])
local reward = 300e3

-- Mission stages
-- 0: just started
-- 1: visited first spob
-- 2: visited last spob and got family
mem.stage = 0

function create ()
   -- Get closest refugee planet
   local candidates = lmisn.getSpobAtDistance( nil, 0, math.huge, nil, false, function( s )
      return s:tags().refugee
   end )
   if #candidates <= 0 then
      misn.finish(false)
      return
   end
   table.sort( candidates, function ( a, b )
      return a:system():jumpDist() < b:system():jumpDist()
   end )

   -- First spob is closest one, second one is fixed
   mem.first_spob, mem.first_sys = candidates[1], candidates[1]:system()
   mem.last_spob, mem.last_sys = spob.getS("Maanen's Moon")
   mem.return_spob, mem.return_sys = spob.cur()

   misn.setNPC( lcs.lucas.name, lcs.lucas.portrait, _([[The person looks stressed and worn out.]]) )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local lucas = vn.newCharacter( lcs.vn_lucas() )
   vn.transition( lcs.lucas.transition )

   vn.na(_([[You approach the stressed out individual gnawing at their nails at the bar.]]))
   lucas(_([["Say, you wouldn't be able to help me find my family? I'll pay, I promise!"]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Refuse]]), "refuse"},
   }

   vn.label("refuse")
   lucas(_([[They go back to chewing their nails.]]))
   vn.done( lcs.lucas.transition )

   vn.label("accept")
   vn.func( function ()
      accepted = true
   end )
   lucas(_([["My name is Lucas, I was a Nebula refugee. When I was very young, me and my family escaped the Incident, but I don't know what happened and I got separated."]]))
   lucas(_([["I guess I never really thought about it too much, just did what I was told and went with the flow, but then one day I couldn't. You understand?"]]))
   lucas(_([[The pause and tighten their fists.
"I guess there was always something missing in me, like something fundamental was taken away by the Incident. However, I just didn't realize it, didn't understand."]]))
   lucas(_([["One day it snapped, and I had a dream remembering stuff I had forgotten about: my family. I guess I had repressed it for so long. Survival instincts maybe?"]]))
   lucas(_([["Since then I've been trying to find my family with the credits I was able to scrounge up working. I don't think they had as much luck as I had, they are probably stuck on some refugee world. The governments don't care for us at all. They just toss refugees into barely habitable planets and turn their eyes away. We are humans too!"]]))
   lucas(fmt.f(_([["I've narrowed it down a bit, I think they should be on {spb} in the {sys} system. Please try to find them! I'll give you all the information I have. Here take this locket, it is the only thing I have from them."]]),
      {spb=mem.first_spob, sys=mem.first_sys}))
   vn.na(_([[They hand you an old locket that looks like is missing half of it. It is fairly simple and made of some resistant metal alloy, but bears signs of heavy use.]]))
   lucas(_([["I'll be waiting here."]]))
   vn.done( lcs.lucas.transition )
   vn.run()

   if not accepted then return end

   misn.accept()
   misn.setTitle( title )
   misn.setDesc(_([[You promised to help Lucas, the ex-Nebula refugee, find his family who may not have made it far from the Nebula.]]))
   misn.setReward(reward)

   local c = commodity.new( N_("Old Pendant"), N_("An old locket belonging to Lucas.") )
   mem.cargo = misn.cargoAdd( c, 0 )

   misn.osdCreate(_(title), {
      fmt.f(_([[Search for the family at {pnt} ({sys} system)]]),
         {pnt=mem.first_spob, sys=mem.first_sys}),
      fmt.f(_([[Return to {pnt} ({sys} system)]]),
         {pnt=mem.return_spob, sys=mem.return_sys}),
   })
   mem.mrk = misn.markerAdd( mem.first_spob )

   hook.land("land")
end

function land ()
   local spb = spob.cur()
   if mem.stage==0 and spb==mem.first_spob then
      vn.clear()
      vn.scene()
      vn.transition()

      vn.na(_([[You land and begin to ask around to see if there are any traces of Lucas family.]]))
      vn.na(fmt.f(_([[As you converse with many of the refugees at {spb}, you begin to appreciate the magnitude of the Incident calamity. Many refugees are missing or searching for family members, with deep psychological scars that are unable to heal.]]),
         {spb=spb}))
      vn.na(fmt.f(_([[Feeling like searching for a needle in a haystack, you are almost about to give up when you find an older one-armed refugee. They take a close look at the locket and mention that they used to share a cell with someone using the same locket on {spb} in the {sys} system.]]),
         {spb=mem.last_spob, sys=mem.last_sys}))
      vn.na(fmt.f(_([[From their story, it seems like {spb} is a horrible traumatic place, a refugee limbo where atrocities are commonplace. It also seems like the planet is locked down, you'll likely have to bribe the authorities to access it.]]),
         {spb=mem.last_spob}))
      vn.na(_([[They give you the location of the cell where they were and wish you luck. Looks like you have a lead.]]))

      vn.run()

      misn.osdCreate(_(title), {
         fmt.f(_([[Search for the family at {pnt} ({sys} system, bribe if necessary)]]),
            {pnt=mem.last_spob, sys=mem.last_sys}),
         fmt.f(_([[Return to {pnt} ({sys} system)]]),
            {pnt=mem.return_spob, sys=mem.return_sys}),
      })
      misn.markerMove( mem.mrk, mem.last_spob )
      mem.stage = 1

   elseif mem.stage==1 and spb==mem.last_spob then
      vn.clear()
      vn.scene()
      vn.transition()

      vn.na(fmt.f(_([[You manage to land on {spb}. Although things are quite quiet and clean around the spaceport, it all takes a turn for the worse once you pass the checkpoints and head into the slums towards the location you were given.]]),
         {spb=spb}))
      vn.na(_([[The atmosphere is dark, almost oppressive, and you feel like you are given many stares by the locals. It seems likely that they do not get many visitors.]]))
      vn.na(_([[Eventually you get to the cell you were told about. It seems to be locked. You take a look to see if you can override it.]]))
      mg.vn()
      vn.func( function ()
         if mg.completed() then
            misn.osdActive(2)
            misn.markerMove( mem.mrk, mem.return_spob )
            mem.stage=2
            local c = commodity.new( N_("Lucas' Father"), N_("A very weakened old man.") )
            mem.cargo_person = misn.cargoAdd( c, 0 )
         else
            vn.jump("failed")
            return
         end
      end )
      vn.na(_([[You open the cell and find yourself with a very weakened old man. At first they are scared to death, believing you have come to take their meager possessions or their life, however, you are able to calm them down when you mention that Lucas sent you, and pass out.]]))
      vn.na(_([[You quickly look around to see if there is anyone else there. Once you confirm that the man is the only person you pick them up and make your way back to the spaceport. Likely due to malnourishment, the old man is a much lighter load than you had expected. On the way back, surprisingly enough, people seem to take less notice of you than when you came in.]]))
      vn.na(_([[Eventually you reach the spaceport checkpoint. The guards raise an eyebrow at the old man you're carrying, but once you show them the receipt of your spaceship, they let you through.]]))
      vn.na(_([[The old man is in not very good shape and seems to fall into a deep slumber when you set them aboard the ship, would be best to try to head back to Lucas as soon as possible.]]))
      vn.done()

      vn.label("failed")
      vn.na(_([[You fail to override the cell. As you are attracting too much suspicion, you decide to head back to the spaceport area. You'll have to come back if you want to try again.]]))
      vn.done()

      vn.run()

   elseif mem.stage==2 and spb==mem.return_spob then
      vn.clear()
      vn.scene()
      local lucas = vn.newCharacter( lcs.vn_lucas() )
      vn.transition( lcs.lucas. transition )

      vn.na(_([[You land and quickly go find Lucas, who then follows you back to your ship.]]))
      lucas(_([[When Lucas sees the old man, his eyes tear up and he kneels to take a closer look.
"Dear old man, what have they done to you?"]]))
      lucas(_([[Still kneeling, Lucas shuffles forward and emotively hugs the old man. Probably not the reunion they were looking for, but better than nothing nonetheless.]]))
      lucas(_([[After a solemn while, Lucas kisses the old man on the cheek and turns to you.
"Thank you for what you've done. I think I can take care of it from now on. I have to see what happened to the others."]]))
      vn.na(_([[You help Lucas take the old man off the ship, and also return the locket. Lucas thanks you fervently for all you've done and hands you a credit chip. He then heads off with his father towards the nearest medical center.]]))

      vn.sfxVictory()
      vn.func( function ()
         player.pay( reward )
         var.push( "lucas01_done", time.get() )
      end )
      vn.na(fmt.reward(reward))

      vn.done( lcs.lucas.transition )
      vn.run()

      misn.finish(true)
   end
end
