local fmt = require "format"
local vn = require "vn"
local tut = require "common.tutorial"
local poi = require "common.poi"

local reward = outfit.get("ZD-5 Guardian Unit")

return function ( mem )
   -- Must be locked
   if not mem.locked then return end

   -- Must be nebula or near nebula
   if not poi.nearNebula( mem ) then
      return
   end

   -- Already done
   if player.outfitNum( reward ) > 0 then
      return
   end

   return {
      type = "function",
      ship = "Za'lek Scout Drone",
      func = function ()
         local sai = tut.vn_shipai()
         vn.appear( sai, tut.shipai.transition )
         sai(_([[Your ship AI appears as you access the drone's systems.
"How rare to see a Za'lek drone out here. It seems to have run out of power. Let me inject a bit and see if I can reactivate the systems fully…"]]))
         vn.na(_([[The light flickers a bit and the drone boots up in maintenance mode.]]))
         sai(_([["I'm afraid this is the best I can do. I was able to find some logs. Let us see them."]]))
         vn.disappear( sai, tut.shipai.transition )

         vn.na(_([[BEGIN TEXT LOG]]))

         vn.na(_([[SCOUT DRONE LOG UST 593:3726.5000

DAMAGE REPORT.
PROPULSION SYSTEMS NON-FUNCTIONAL.
SHIELD GENERATOR UNRESPONSIVE.
SENSORS WORKING AT 70% EFFICIENCY.
NO FURTHER REPAIRS POSSIBLE.]]))
         vn.na(_([[CONTINUING ANALYSIS OF EVENT.
ENERGY EMITTED ESTIMATED TO BE 10e50 JOULES WITH MARGIN OF ERROR OF 10e51 JOULES.
EPICENTER ESTIMATED TO BE WITHIN 50 LIGHT YEARS OF THE POLARIS SYSTEM.
SURVIVABILITY WITHIN 500 LIGHT YEARS OF EPICENTER ESTIMATED TO BE 0.00000000001%.]]))
         vn.na(_([[STRONG SUBSPACE ANOMALITIES DETECTED. SPECIALIZED HARDWARE NECESSARY FOR ANALYSIS.
DELTA WAVES RECEDING, 10 CYCLES EXPECTED FOR DECREASE TO MINIMUM LEVELS NECESSARY FOR HUMAN SURVIVAL.
NO FURTHER ANALYSIS POSSIBLE WITH CURRENT HARDWARE. CONTINUING CURRENT SURVEILLANCE.]]))

         vn.na(_([[END TEXT LOG]]))

         vn.na(_([[The drone flickers once more and everything goes dark. It seems like that was the last of the life that was in it. You run a last routine scan to see if there is anything you can do. While it seems like the drone is sadly out of order permanently, you find what seems to be an experimental piece of hardware of Za'lek make attached to it. Seeing as it is no longer of use to the drone, you take it with you.]]))
         vn.na(fmt.reward(reward))

         vn.func( function ()
            player.outfitAdd( reward )
            poi.log(fmt.f(_([[You found a derelict Za'lek drone in the {sys} system. It contained information about the Incident and you were able to loot a {reward} from it.]]),
               {sys=mem.sys, reward=reward}))
         end )
      end,
   }
end
