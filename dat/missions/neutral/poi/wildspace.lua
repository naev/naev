local fmt = require "format"
local vn = require "vn"
local tut = require "common.tutorial"
local poi = require "common.poi"

return function ( mem )
   -- Must be locked
   if not mem.locked then return end

   if player.chapter()=="0" then return end

   -- Must know data matrices
   if not poi.data_known() then return end

   -- Must be "nearby"
   local sys1 = system.get("Jommel")
   if sys1:jumpDist() > 7 then
      return
   end

   -- Already done
   local sys2 = system.get("Syndania")
   local jmp = jump.get( sys1, sys2 )
   if jmp:known() then
      return
   end

   return {
      type = "function",
      ship = "Bedivere",
      shipname = _("Irradiated Derelict"),
      weight = 10, -- Much more likely if can appear
      func = function ()

         vn.na(_([[You carefully enter the derelict while watching the radiation levels. They same to be dangerously high, however, if you do not take too long, you do not think it should be too dangerous for your HEV suit. Watching the radiation readings, you warily head towards the command room.]]))
         vn.na(_([[The inside of the ship is in horrible shape. There is junk and dirt everywhere. You can see clear damage on the walls, and most of the airlocks seem to be jammed open. While gliding through the hallway, something grabs your foot, and you instinctively draw your weapon and fire. Once the sound of your ricocheting noise diminishes, you are left with the sound of your accelerated heartbeat.]]))
         vn.na(_([[You catch nothing else move as the ship is silent. With your adrenaline fading, you poke at the object you shot with your foot, revealing a dirty corpse. Seeing the state of mummification, you can only guess it was long dead before you got here. The body is wearing what seems to be a bunch of rags from some old space suit, you take a closer look when you notice the eyes seem to be a faded shade of purple. Strange, maybe some Soromid biotech?]]))
         vn.na(fmt.f(_([[You make your way through the accumulated grime and debris without any further surprises. Eventually you reach the command room. Quickly scanning the room, you find 3 more corpses, all with the same purple eyes but drastically different features. So much for the Soromid theory. You call on {ainame} to perform a more in-depth scan and database lookup.]]),
            {ainame=tut.ainame()}))

         local sai = tut.vn_shipai()
         vn.appear( sai, tut.shipai.transition )
         sai(_([["There seem to be a total of 8 bodies on the ship. The purple eyes are likely some sort of effect from the radiation they have received. Analysing the radiation profile of the ship, it seems to be statistically correlated with the Nebula volatility. I am trying to perform an analysis of the systems, however, the firmware they seem to use is very outdated. It will take a while to interface."]]))
         vn.na(fmt.f(_([[You check the rest of the bodies while {shipai} performs their analysis. Yup, they all have purple eyes.}]]),
            {shipai=tut.ainame()}))
         local reward = poi.data_str(1)
         sai(fmt.f(_([[You receive an alert that the analysis is done.
"I was not able to recover too much other than {reward}, but there was some information of an unknown jump near in the Nebula. I have updated your map with it. It may be worth exploring, although I am not certain that it is worth the risk to organic life."]]),
            {reward=reward}))
         vn.na(fmt.reward(reward))
         vn.disappear( sai, tut.shipai.transition )

         vn.na(_([[It may be worth seeing the hidden to see what caused the fate of the ship. You may want to double-check how hardened your ship is to radiation first though.]]))

         vn.func( function ()
            jmp:setKnown()
            poi.data_give(1)
            poi.log(fmt.f(_([[You found an irradiated derelict ship in the {sys} system containing information about a hidden jump in the Nebula. You also were able to recover {reward} from the ship.]]),
               {sys=mem.sys, reward=reward}))
         end )
      end,
   }
end
