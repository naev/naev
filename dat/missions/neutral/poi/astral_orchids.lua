local fmt = require "format"
local vn = require "vn"
local vne = require "vnextras"
local tut = require "common.tutorial"
local poi = require "common.poi"
local ao = require "common.astral_orchids"

local misnvar = "poi_orchids"

return function ( mem )
   -- Must be locked
   if not mem.locked then return end

   -- Need to have started the poi data stuff
   if poi.data_get_gained() <= 0 then return end

   -- Already finished
   local mvar = var.peek( misnvar ) or 0
   if mvar >= 1 then
      return
   end

   return {
      type = "function",
      ship = "Llama Voyager",
      func = function ()
         local SPOBS = rnd.permutation( ao.SPOBS )
         local SYSTEMS = {}
         for k,s in ipairs(SPOBS) do
            table.insert( SYSTEMS, s:system() )
         end

         local sai = tut.vn_shipai()
         vn.appear( sai, tut.shipai.transition )
         sai(_([[Your ship AI appears as you access the system.
"I've tried accessing the systems, and it seems like the ship has been non-functional for a while. A physical inspection may be the best course of action."]]))
         vn.disappear( sai, tut.shipai.transition )

         vn.na(_([[You start exploring the vessel, which seems to be covered in nearly fossilized desiccated plants in neat pots. Many seem to have had labels which are now unreadable due to age. You look around the cockpit and manage to find what seems to be a notebook with some pages still intact.]]))

         local log = vne.notebookStart()

         -- Game starts at 603:3726.2871
         log(_([[It's all starting to come together! I've managed to find several planets where the Astral Orchids have been able to thrive, but it hasn't been easy. The necessary biochemical conditions are much stricter than my calculations had originally anticipated. Seeing the orchids bloom in the wild makes it all worth it. I look forward to seeing them bloom, it will be truly a triumph for this species.

And to believe that it was driven to extinction so long ago! It was risky, but extracting the last embryos and flasking them. It may have cost all my savings, but I would do it again given half the chance.]]))
         -- TODO maybe do something better for the locations? Probably player will forget / not notice in seconds, but may just make sense to have the Oracle character or whatever to nudge the player instead
         log(fmt.f(_([[I have currently got some viable populations started at {locations} systems, however, long-term viability is still not clear. I am still going down the list of other potential candidates, however, they are at much riskier locations. I am not certain I will be able to get to them all, I just hope my Llama Voyager's fuel regeneration module doesn't break down.]]),
            {locations="#b"..fmt.list(SYSTEMS).."#0"}))

         vne.notebookEnd()

         local reward = poi.data_str(1)
         vn.na(_([[You look around more and also find {reward} tucked away underneath the captain's chair. At least you won't go back empty-handed, although finding the populations of Astral Orchids could be something interesting to pursue...]]))
         vn.na(fmt.reward(reward))

         vn.func( function ()
            var.push( misnvar, mvar+1 )
            poi.data_give(1)
            poi.log(fmt.f(_([[You found a derelict in the {sys} with notes on Astral Orchids, and you found {reward}. The notes mentioned planting Astral Orchids at the {locations} systems.]]),
               {sys=mem.sys, reward=reward, locations=fmt.list(SYSTEMS)}))
         end )
      end,
   }
end
