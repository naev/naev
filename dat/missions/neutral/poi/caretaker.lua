local fmt = require "format"
local vn = require "vn"
local vne = require "vn.extras"
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
   if mvar >= 1 then
      return
   end

   return {
      type = "function",
      ship = "Koala",
      func = function ()

         local sai = tut.vn_shipai()
         vn.appear( sai, tut.shipai.transition )
         sai(_([[Your ship AI appears as you access the system.
"This derelict seems to be quite a mess. I've completed a scan and it is full of biological waste. Be careful."]]))
         vn.disappear( sai, tut.shipai.transition )
         vn.na(_([[The airlock opens and filth oozes out. Looks like you're going to have to get dirty. You spend a while searching and find some half-digested pages of notes. Could be a diary?]]))

         local log = vne.notebookStart()
         if mvar == 0 then
            log(_([[The damn thing bit me again today, little blood this time. I think it has it out for me. I thought the Pet Iguana Caretaker was supposed to be a noble position, and yet we spend all day knees deep in iguana filth! I never expected them to excrete so much. It's an endless torrent! I did a PhD thesis in biology for this?]]))
            log(_([[Doctors say I may recover feeling in my reattached finger, and still insist on me going to some weird Soromid clinic for a proper adjustment, however, I still haven't figured out the paperwork I have to do to even start considering that option. The Chief Caretaker told me that in the 30 cycles since he started working here that they haven't figured out any of the forms they have to do. I feel like it's all just a ruse so we can't claim healthcare, but it could just be all the accumulated bureaucratic cruft.]]))
            log(_([[I've started to get used to the Iguana Armour, which is apparently the closest I'll get to healthcare. It stinks horrible from many cycles of usage, but apparently it can't be washed and I have no choice but to get used to it. I've stopped eating before work to minimize it all coming back up with the horrible combination of stench and filth. Can I really get used to this?]]))
         --elseif mvar == 1 then
         --else
         end
         vne.notebookEnd()

         local reward = poi.data_str(1)
         vn.na(fmt.f(_([[You keep on exploring the filthy vessel and eventually find {reward} stashed away in the ship systems, before heading back to your ship. The smell isn't going to go away easily is it?

{rewardmsg}]]),
            {reward=reward, rewardmsg=fmt.reward(reward)}))

         vn.func( function ()
            var.push( misnvar, mvar+1 )
            poi.data_give(1)
            poi.log(fmt.f(_([[You found a derelict ship in the {sys} system and were able to recover {reward}.]]),
               {sys=mem.sys, reward=reward}))
         end )
      end,
   }
end
