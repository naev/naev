--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Lucas 01">
 <unique />
 <chance>20</chance>
 <location>Bar</location>
 <cond>
   -- Required to bribe Maanen's Moon
   if spob.get("Maanen's Moon"):reputation() &lt; 0 then
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

   vn.na(_([[You approach the stressed-out individual gnawing at their nails at the bar.]]))
   lucas(_([["Any chance you could help me find my family? I'll pay, I promise!"]]))
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
   lucas(_([["My name is Lucas. I'm a Nebula refugee. When I was very young, my family and I escaped the Incident, but I don't know what happened-we got separated."]]))
   lucas(_([["I never really thought about it; I just did what I was told and went with the flow, until one day I couldn't anymore. You understand?"]]))
   lucas(_([[They pause and clench their fists.
"There was always something missing in me, like something fundamental the Incident took away, but I didn't realize it, didn't understand."]]))
   lucas(_([["Then something snapped. I had a dream that brought back what I'd buried—my family. I guess I'd repressed it for so long. Survival instincts maybe?"]]))
   lucas(_([["Since then I've been trying to find them with the credits I've managed to scrape together. I don't think they were as lucky; they're probably stuck on some refugee world. The governments don't care for us at all. They just toss refugees onto barely habitable planets and look away. We're human too!"]]))
   lucas(fmt.f(_([["I've narrowed it down a bit: I think they're on {spb} in the {sys} system. Please try to find them! I'll give you all the information I have. Here, take this locket—it's the only thing I have from them."]]),
      {spb=mem.first_spob, sys=mem.first_sys}))
   vn.na(_([[They hand you an old locket that looks like it's missing half. It's fairly simple and made of some rugged metal alloy, but bears signs of heavy use.]]))
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

      vn.na(_([[You land and begin to ask around to see if there are any traces of Lucas' family.]]))
      vn.na(fmt.f(_([[As you converse with many of the refugees at {spb}, you begin to appreciate the calamity of the Incident. Many refugees are missing or searching for family members, carrying deep psychological scars that won't heal.]]),
         {spb=spb}))
      vn.na(fmt.f(_([[Feeling like you're searching for a needle in a haystack, you're almost ready to give up when you find an older one-armed refugee. They take a close look at the locket and mention they used to share a cell with someone who had the same locket on {spb} in the {sys} system.]]),
         {spb=mem.last_spob, sys=mem.last_sys}))
      vn.na(fmt.f(_([[From their story, {spb} sounds horribly traumatic, a refugee limbo where atrocities are commonplace. The planet is locked down; you'll likely have to bribe the authorities to access it.]]),
         {spb=mem.last_spob}))
      vn.na(_([[They give you the location of the cell where they were and wish you luck. Looks like you finally have a lead.]]))

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

      vn.na(fmt.f(_([[You manage to land on {spb}. Although things are quite quiet and clean around the spaceport, it all takes a turn for the worse once you pass the checkpoints and head into the slums toward the location you were given.]]),
         {spb=spb}))
      vn.na(_([[The atmosphere is dark, almost oppressive, and you feel the locals' stares. They clearly don't get many visitors.]]))
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
      vn.na(_([[You open the cell to find a very weakened old man. At first they are terrified, thinking you've come to take their meagre possessions or their life, but when you mention Lucas sent you they ease for a moment before collapsing from shock.]]))
      vn.na(_([[You quickly look around to see if there is anyone else there. Once you confirm the man is alone, you pick him up and make your way back to the spaceport. Likely due to malnourishment, the old man is a much lighter load than you had expected. On the way back, surprisingly enough, people seem to take less notice of you than when you came in.]]))
      vn.na(_([[Eventually you reach the spaceport checkpoint. The guards raise an eyebrow at the old man you're carrying, but once you show them the receipt of your spaceship, they let you through.]]))
      vn.na(_([[The old man is in rough shape and falls into a deep slumber when you set him aboard the ship. It would be best to head back to Lucas as soon as possible.]]))
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
      lucas(_([[When Lucas sees the old man, his eyes well up and he kneels to take a closer look.
"Dear old man, what did they do to you?"]]))
      lucas(_([[Still kneeling, Lucas shuffles forward and hugs the old man tightly. Probably not the reunion he was looking for, but better than nothing nonetheless.]]))
      lucas(_([[After a solemn while, Lucas kisses the old man on the cheek and turns to you.
"Thank you for everything. I can take it from here. I have to see what happened to the others."]]))
      vn.na(_([[You help Lucas take the old man off the ship, and also return the locket. Lucas thanks you fervently for all you've done and hands you a credit chip. He then heads off with his father towards the nearest medical centre.]]))

      vn.sfxVictory()
      vn.func( function ()
         player.pay( reward )
         var.push( "lucas01_done", time.cur() )
      end )
      vn.na(fmt.reward(reward))

      vn.done( lcs.lucas.transition )
      vn.run()

      misn.finish(true)
   end
end
