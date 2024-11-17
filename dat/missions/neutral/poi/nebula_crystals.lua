local fmt = require "format"
local vn = require "vn"
local tut = require "common.tutorial"
local poi = require "common.poi"

local cargo = commodity.get("Nebula Crystals")

return function ( mem )
   -- Must be locked
   if not mem.locked then return end

   -- Must be nebula or near nebula
   if not poi.nearNebula( mem ) then
      return
   end

   local function add_crystals( q )
      q = q or rnd.rnd(20,50)
      local added
      vn.func( function ()
         added = player.fleetCargoAdd( cargo, q )
         var.push( "poi_nebula_crystals", (var.peek("poi_nebula_crystals") or 0)+1 )
         poi.log(fmt.f(_([[You found large amounts of {cargo} on an unusual derelict in the {sys} system.]]),
            {cargo=cargo, sys=mem.sys}))
      end )
      vn.na( function ()
         return fmt.f(_("You have received #g{amount} of {cargo}#0."), {amount=fmt.tonnes(added), cargo=cargo})
      end )
   end

   return {
      type = "function",
      ship = "Rhino",
      func = function ()

         if var.peek( "poi_nebula_crystals" ) then
            vn.na(fmt.f(_("Once you gain access to the ship, a quick scan finds large amounts of nebula radiation. You approach the source and find a large cache of {cargo}. Given that there seems to be nothing else of interest on the ship, you fit as much as you can take and leave."),
               {cargo=cargo}))
         else
            local sai = tut.vn_shipai()
            vn.appear( sai, tut.shipai.transition )
            sai(_([[Your ship AI materializes once you access the system.
"I am detecting a large amount of unknown radiation. It seems to be coming from the inside of this derelict. I am sending you a rough location."]]))

            vn.na(fmt.f(_([[You scour the ship heading towards the location sent to you by {shipai}. Eventually you reach the cargo hold which seems to be glowing faintly. You carefully open the hatch and find what seem to be a small mountain of some sort of crystal.]]),
               {shipai=tut.ainame()}))

            sai(_([["BEGINNING SCANNING… … … … …SCAN COMPLETE."
"It seems like these crystals have a very similar signal to the Nebula. I would hypothesize that they are of Nebula origin. I would guess many individuals would want to get their hands on them. It should not be difficult to sell them if you are so inclined. I would recommend taking as many as possible"]]))

            vn.na(fmt.f(_([[Heeding {shipai}'s advice, you load as many {cargo} as you can. You also perform a quick search of the rest of the ship and find nothing useful, other than the {cargo}.]]),
               {shipai=tut.ainame(), cargo=cargo}))
         end

         add_crystals()
      end,
   }
end
