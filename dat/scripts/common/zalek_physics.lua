--[[

   Za'lek Particle Physics Common Functions

--]]
local vn = require "vn"
local mt = require 'merge_tables'

local zpp = {}

-- Noona Sanderaite
zpp.noona = {
   portrait = "zalek3.png",
   image = "zalek3.png",
   name = _("Noona"),
   color = nil,
   transition = nil, -- Use default
   description = _("You see Noona who looks like she might have a job for you."),
}

function zpp.vn_noona( params )
   return vn.Character.new( zpp.noona.name,
         mt.merge_tables( {
            image=zpp.noona.image,
            color=zpp.noona.colour,
         }, params) )
end

-- Function for adding log entries for miscellaneous one-off missions.
function zpp.log( text )
   shiplog.create( "zlk_physics", _("Particle Physics"), _("Za'lek") )
   shiplog.append( "zlk_physics", text )
end

zpp.rewards = {
   zpp01 = 200e3,
   zpp02 = 300e3,
   zpp03 = 200e3, -- + "Heavy Weapons Combat License" permission
   zpp04 = 400e3,
   --zpp05 = 0, -- No payment, reall small flashback really
   zpp06 = 200e3, -- + "Heavy Combat Vessel License" permission
}

return zpp
