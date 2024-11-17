local fmt = require "format"
local vn = require "vn"
local vne = require "vnextras"
local tut = require "common.tutorial"
local poi = require "common.poi"

local misnvar = "poi_caretaker"

return function ( mem )
   -- Must be locked
   if not mem.locked then return end

   -- Need to have started the poi data stuff
   if poi.data_get_gained() <= 0 then
      return
   end

   -- Already finished
   local mvar = var.peek( misnvar )  or 0
   if mvar >= 3 then
      return
   end

   return {
      type = "function",
      ship = "Koala",
      func = function ()

         local sai = tut.vn_shipai()
         vn.appear( sai, tut.shipai.transition )
         sai(_([[Your ship AI appears as you access the system.
"This derelict seems to be quite a mess. I've completed a scan, and it is full of biological waste. Be careful."]]))
         vn.disappear( sai, tut.shipai.transition )
         vn.na(_([[The airlock opens and filth oozes out. Looks like you're going to have to get dirty. You spend a while searching and find some half-digested pages of notes. Could be a diary?]]))

         local log = vne.notebookStart()
         if mvar == 0 then
            log(_([[The damn thing bit me again today, little blood this time. I think it has it out for me. I thought the Pet Iguana Caretaker was supposed to be a noble position, and yet we spend all day knees deep in iguana filth! I never expected them to excrete so much. It's an endless torrent! I did a PhD thesis in biology for this?]]))
            log(_([[Doctors say I may recover feeling in my reattached finger, and still insist on me going to some weird Soromid clinic for a proper adjustment, however, I still haven't figured out the paperwork I have to do to even start considering that option. The Chief Caretaker told me that in the 30 cycles since he started working here that they haven't figured out any of the forms they have to do. I feel like it's all just a ruse, so we can't claim healthcare, but it could just be all the accumulated bureaucratic cruft.]]))
            log(_([[I've started to get used to the Iguana Armour, which is apparently the closest I'll get to healthcare. It stinks horrible from many cycles of usage, but apparently it can't be washed, and I have no choice but to get used to it. I've stopped eating before work to minimize it all coming back up with the horrible combination of stench and filth. Can I really get used to this?]]))
         elseif mvar == 1 then
            log(_([[I am not sure if I have completely lost my sense of smell or the Imperial Canteen has gone overindulged in blandness. Either way, I don't think I will be missing the flavour of the Royal Nutritional Pudding. They can not even feed that stuff to beasts.]]))
            log(_([[We lost another intern this morning when the Iguana grabbed them. That is the 3rd one this month. It almost seems like their diet is more intern than anything else. As if losing the intern itself wasn't bad enough, the follow-up paperwork is horrible. The Chief Caretaker is now making me do it all before their retirement. Am I going to have to do this forever? I might end up being jealous of the interns…]]))
            log(_([[Although I am busy with the Iguana most of the time, and it has for me to notice, things seem to be changing throughout the Imperial Palace. You see people huddled and whispering with constant new proclamations. Work is also being started on some very large space structures in the solar system. It is supposed to be a new sort of revolutionary device, however, there are worries it is going to bankrupt the Empire. I try not to engage in such thoughts, The last thing I need is to be accused of being a traitor.]]))
         else
            log(_([[The duties of the Chief Caretaker wear me down day by day. I haven't felt this dead inside since I was in the middle of my PhD thesis. Each day is a slog, and wading through the Iguana filth seems like an allegory of my mental state. At least the Iguana has learned to respect me. It only took most of my left arm. When I retire I should see if I can finish the paperwork for the Soromid treatment to recover full mobility. Retirement! What a beautiful word!]]))
            log(_([[You would think that an economic crisis would encourage more interns, however, it seems like the mortality statistics do not play in our favour. We have the fewest amount of applications I have ever seen. This makes the Iguana hungry and more irritable, which in turn scares off the few interns that actually show up. I wonder if it would be possible to look into some other alternative food source. I worry about the budget though.]]))
            log(_([[I hope to retire and leave the system before the large constructions are finished. There are too many Peacemakers and Hawkings around the star system. All seems to indicate that something is going to happen. Most likely not good. I must work harder to find a suitable replacement for me, or I will end up caught in the middle of the mess.]]))
         end
         vne.notebookEnd()

         local reward = poi.data_str(1)
         vn.na(fmt.f(_([[You keep on exploring the filthy vessel and eventually find {reward} stashed away in the ship systems, before heading back to your ship. The smell isn't going to go away easily, is it?

{rewardmsg}]]),
            {reward=reward, rewardmsg=fmt.reward(reward)}))

         vn.func( function ()
            var.push( misnvar, mvar+1 )
            poi.data_give(1)
            poi.log(fmt.f(_([[You found a filthy derelict ship in the {sys} system and were able to recover {reward}.]]),
               {sys=mem.sys, reward=reward}))
         end )
      end,
   }
end
