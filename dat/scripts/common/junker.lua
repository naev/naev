local equipopt = require "equipopt"
local eparams = require 'equipopt.params'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'

local PLATES = outfit.get("Junker Plates")

local lib = {}

-- Spawn The Junker
function lib.spawn_pilot( pos )
   local fct = faction.dynAdd( "Mercenary", "junker", _("Junker") )
   local junker = pilot.add( "Koala Armoured", fct, pos, _("The Junker"), {naked=true, ai="pers"} )
   local cores = ecores.get( junker, { all="standard" } )
   cores["hull"] = PLATES
   local outfits = eoutfits.merge{
      eoutfits[ "standard" ].set
      { "Junker Ion Shotter" },
   }
   local params = eparams.choose( junker )
   params.prefer = {
      ["Junker Ion Shotter"] = 100,
   }
   equipopt.optimize( junker, cores, outfits, params )

   local m = junker:memory()
   if player.outfitNum( PLATES ) <= 0 then
      m.lootable_outfit = PLATES
   end
   m.capturable = true
   junker:outfitAddIntrinsic("Escape Pod")
   return junker
end

-- The Junker doesn't like people
function lib.good_sys( sys )
   sys = sys or system.cur()
   for k,spb in ipairs(sys:spobs()) do
      if spb:services()["inhabited"] then
         return false
      end
   end
   return true
end

return lib
