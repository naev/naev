local fmt = require "format"
local vn = require "vn"
local vne = require "vnextras"
local tut = require "common.tutorial"
local poi = require "common.poi"

local reward = outfit.get("Bioship Pheromone Emitter")

return function ( mem )
   -- Already done
   if player.outfitNum( reward ) > 0 then
      return
   end

   if mem.sys:jumpDist( "Fertile Crescent", true ) > 7 then
      return
   end

   return {
      type = "function",
      ship = "Soromid Brigand",
      func = function ()
         local sai = tut.vn_shipai()
         vn.appear( sai, tut.shipai.transition )
         sai(_([[Your ship AI appears as you access the system.
"This is strange. What is a dead bioship doing out here? Let me see if I can access the systems."]]))
         sai(_([[After a long pause.
"I'm afraid that it seems like the systems are completely dead. I don't think we'll be able to obtain any information from this ship. You might as well explore it before we leave."]]))
         vn.disappear( sai, tut.shipai.transition )

         vn.na(_([[You start exploring the ship, which seems to be fitted as some sort of science vessel. Eventually, you stumble across what seems to be a dormitory. You find that there seems to be a notebook left on the table. You begin to read it.]]))

         local log = vne.notebookStart()

         -- Game starts at 603:3726.2871
         log(_([[UST 600:4967

I found an interesting rumour today while visiting the bar at Neurri. I was told that, although most bioships are completely domesticated, with little in the ways of autonomy and higher level cognition, it seems like this is not always the case. Some times the bioships can end up too smart for their own good. What happens to them depends on the tribe in this case, but it is not uncommon for them to go missing. This is very interesting and could be related to my research. I am going to try to see if I can find any more information on the topic.]]))
         log(_([[UST 600:4983

After fruitless leads, I have arrived to Silverstone in the Symm system and have been talking with the bioship producers. They seem to reject the notion of "feral" bioships. I was almost about to call it quits and return home, when I managed to talk to a younger employee who sees to confirm my suspicions. Indeed, although rare, some times random mutations in the DNA can lead to not fully controllable bioships. They swear that they are as cunning and intelligent as humans. The future of such ships is uncertain, but they can be taken away or escape. However, only the tribe elders deal with such bioships. It seems like most head towards the Haze, and that is the next place I must travel to. For this trip I will leave behind my trusty Koala and go with a Soromid Brigand. This may arouse less suspicion should I happen upon feral bioships.]]))
         log(_([[UST 601:0005

I have left the Hasselt system and head towards the depth of the Haze following rumours of a nearby colony of feral bioships. Who knows what I shall find? I have not been so giddy with anticipation in ages. Despite having conquered space, there is still much left for humankind to find!]]))
         log(_([[UST 601:0037

There seems to be no signs of life in this haze. I have been reflecting on my task. Perhaps it is a fool's errand after all?]]))
         log(_([[UST 601:0072

My supplies are running a bit thin. It may be hallucinations, but I believe I have seen a few blips on the scanners in the past days. Could these be the ferals?]]))
         log(_([[UST 601:0089

I have been working on a plan. I have been able to localize the different anatomical parts of my bioship. If I can extract pheromones, it may be possible to attract the ferals. This is my last chance.]]))
         log(_([[UST 601:0101

I believe I have completed the pheromone extractor. The smell is… let us say not very pleasant. I will put it to use right away.]]))
         log(_([[The rest of the pages are blank.]]))
         vne.notebookEnd()

         vn.na(fmt.f(_([[You ask {shipai} to perform an in-depth scan for biological signals, and surprisingly enough, find a faint signal that differs from the dessicated biological material of the ship. You go to inspect it and the first thing you find is that it stinks. A quick analysis seems to indicate that this is the pheromone extractor mentioned in the notebook. You carefully take it aboard your ship.]]),
            {shipai=tut.ainame()}))

         vn.na(fmt.reward(reward))

         vn.func( function ()
            player.outfitAdd( reward )
            poi.log(fmt.f(_([[You found a derelict bioship in {sys}. It belonged to a researcher searching for "feral" bioships and you found a {reward}.]]),
               {sys=mem.sys, reward=reward}))
         end )
      end,
   }
end
