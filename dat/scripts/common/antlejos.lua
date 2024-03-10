--[[

   Antlejos Terraforming Common Functions

--]]
local vn = require "vn"

local antlejos = {}

antlejos.verner = {
   portrait = "verner.webp",
   image = "verner.webp",
   name = _("Verner"),
   colour = nil,
   description = _("Verner seems to be taking a break from all the terraforming and relaxing at the new spaceport bar."),
   transition = nil, -- Use default
}

function antlejos.vn_verner( params )
   return vn.Character.new( antlejos.verner.name,
         tmerge( {
            image=antlejos.verner.image,
            colour=antlejos.verner.colour,
         }, params) )
end

-- Function for adding log entries for miscellaneous one-off missions.
function antlejos.log( text )
   shiplog.create( "antlejos", _("Antlejos V"), _("Neutral") )
   shiplog.append( "antlejos", text )
end

antlejos.unidiff_list = {
   "antlejosv_1",
   "antlejosv_2",
   "antlejosv_3",
   "antlejosv_4",
   "antlejosv_5",
   "antlejosv_6",
   "antlejosv_7",
   "antlejosv_8",
   "antlejosv_9",
}

function antlejos.unidiff( diffname )
   for _k,d in ipairs(antlejos.unidiff_list) do
      if diff.isApplied(d) then
         diff.remove(d)
      end
   end
   diff.apply( diffname )
end

function antlejos.unidiffLevel ()
   for k,d in ipairs(antlejos.unidiff_list) do
      if diff.isApplied(d) then
         return k
      end
   end
   return 0
end

function antlejos.dateupdate ()
   var.push( "antlejos_date", time.get() )
end
function antlejos.datecheck ()
   local d = var.peek("antlejos_date")
   return d and d==time.get()
end

--[[
   Gets the Pilots United Against Atmosphere Anthropocentrism (PUAAA) faction or creates it if necessary
--]]
function antlejos.puaaa ()
   local f = faction.exists("puaaa")
   if f then
      return f
   end
   return faction.dynAdd( "Mercenary", "puaaa", _("PUAAA"), {clear_allies=true, clear_enemies=true} )
end

antlejos.protest_lines = rnd.permutation{
   _("No to terraforming!"),
   _("Leave the planets alone!"),
   _("The Universe is beautiful as it is!"),
   _("No modifying atmospheres!"),
   _("Keep our planets safe!"),
   _("No to destroying planets!"),
}

function antlejos.supplied_total ()
   return var.peek( "antlejos_supplied" ) or 0
end

function antlejos.supplied( amount )
   local n = antlejos.supplied_total()
   var.push( "antlejos_supplied", n+amount )
end

antlejos.rewards = {
   ant01 = 200e3,
   ant02 = 350e3,
   ant03 = 500e3,
   ant04 = 600e3,
   ant05 = 700e3,
   ant06 = 400e3,
   ant07 = 500e3,
   ant08 = 600e3,
   ant09 = 800e3,
}

return antlejos
