--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Animal transport">
 <unique />
 <priority>4</priority>
 <chance>10</chance>
 <location>Bar</location>
 <faction>Sirius</faction>
 <cond>
   if spob.cur():tags().station then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[

   MISSION: Animal transport
   DESCRIPTION: A man asks you to transport a crate of specially bred creatures for
   his in-law's exotic pet store on another planet. It's a standard fare A-to-B mission,
   but doing this mission infests the player's current ship with the creatures.

--]]
local neu = require "common.neutral"
local lmisn = require "lmisn"
local fmt = require "format"
local portrait = require "portrait"
local vn = require "vn"

local money_reward = 200e3

local npc_name = _("A Fyrra civilian")
local npc_portrait = "sirius/unique/rodentman.webp"
local npc_image = portrait.getFullPath( "sirius/unique/rodentman.webp" )

function create ()
    -- Get an M-class Sirius planet at least 2 and at most 4 jumps away. If not found, don't spawn the mission.
   local planets = {}
   lmisn.getSysAtDistance( system.cur(), 2, 4,
      function(s)
         for i, v in ipairs(s:spobs()) do
            if v:faction() == faction.get("Sirius") and v:class() == "M" and v:canLand() then
               planets[#planets + 1] = {v, s}
            end
         end
         return false
      end )

   if #planets == 0 then
      misn.finish(false)
   end

   local index = rnd.rnd(1, #planets)
   mem.destplanet = planets[index][1]
   mem.destsys = planets[index][2]

   misn.setNPC( npc_name, npc_portrait, _("There's a civilian here, from the Fyrra echelon by the looks of him. He's got some kind of crate with him."))
end


function accept ()
   local accepted = false
   vn.clear()
   vn.scene()
   local rod = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()
   rod(fmt.f(_([["Good day to you, captain. I'm looking for someone with a ship who can take this crate here to planet {pnt} in the {sys} system. The crate contains a colony of rodents I've bred myself. My in-law has a pet shop on {pnt} where I hope to sell them. Upon delivery, you will be paid {credits}. Are you interested in the job?"]]),
      {credits=fmt.credits(money_reward), pnt=mem.destplanet, sys=mem.destsys}))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   vn.done()

   vn.label("accept")
   rod(_([["Excellent! My in-law will send someone to meet you at the spaceport to take the crate off your hands, and you'll be paid immediately on delivery. Thanks again!"]]))
   vn.func( function () accepted = true end )

   vn.run()

   if not accepted then
      return
   end

   misn.accept()
   misn.setDesc(fmt.f(_("You've been hired to transport a crate of specially engineered rodents to {pnt} ({sys} system)."), {pnt=mem.destplanet, sys=mem.destsys}))
   misn.setReward(fmt.f(_("You will be paid {credits} on arrival."),{credits=fmt.credits(money_reward)}))
   misn.osdCreate(_("Animal transport"), {fmt.f(_("Fly to the {sys} system and land on planet {pnt}"), {sys=mem.destsys, pnt=mem.destplanet})})
   misn.markerAdd( mem.destplanet, "high" )
   hook.land("land")
end

function land()
   if spob.cur() ~= mem.destplanet then
      return
   end

   vn.clear()
   vn.scene()
   vn.na(fmt.f(_([[As promised, there's someone at the spaceport who accepts the crate. In return, you receive a number of credit chips worth {credits}, as per the arrangement. You go back into your ship to put the chips away before heading off to check in with the local authorities. But did you just hear something squeak…?]]),{credits=fmt.credits(money_reward)}))
   vn.func( function ()
      player.pay(money_reward)
   end )
   vn.sfxVictory()
   vn.na( fmt.reward(money_reward) )
   vn.run()

   var.push("shipinfested", true)
   neu.addMiscLog( _([[You successfully transported a crate of rodents for a Fyrra civilian. You could have sworn you heard something squeak.]]) )
   misn.finish(true)
end
