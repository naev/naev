--[[
-- Helper stuff for the Totoran Coliseum
--]]

local vn = require 'vn'
local colour = require 'colour'
local portrait = require 'portrait'
require 'numstring'

local totoran = {
   guide = {
      name = _("Coliseum Guide"),
      portrait = "minerva_terminal.png",
      image = "minerva_terminal.png",
      desc = _("Information on the Totoran Coliseum."),
      colour = nil,
   }
}

local function _merge_tables( p, params )
   params = params or {}
   for k,v in pairs(params) do p[k] = v end
   return p
end

-- Helpers to create main characters
function totoran.vn_guide( params )
   return vn.Character.new( totoran.guide.name,
         _merge_tables( {
            image=totoran.guide.image,
            color=totoran.guide.colour,
         }, params) )
end


-- Emblem stuff
function totoran.emblems_get()
   return var.peek( "totoran_emblems" ) or 0
end
function totoran.emblems_get_gained()
   return var.peek( "totoran_emblems_gained" ) or 0
end
function totoran.emblems_pay( amount )
   local v = totoran.emblems_get()
   var.push( "totoran_emblems", v+amount )
   -- Store lifetime earnings
   if amount > 0 then
      v = var.peek( "totoran_emblems_gained" ) or 0
      var.push( "totoran_emblems_gained", v+amount )
   end
end
function totoran.emblems_str( amount )
   return gettext.ngettext(
      "#r%s Coliseum Emblems#0",
      "#r%s Coliseum Emblems#0", amount ):format(
         numstring(amount) )
end

return totoran
