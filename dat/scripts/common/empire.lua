--[[
-- Common Empire Mission framework
--
-- This framework allows to keep consistency and abstracts around commonly used
--  empire mission functions.
--]]
local portrait = require "portrait"
local vn = require "vn"

local emp = {}

emp.prefix = "#g".._("EMPIRE: ").."#0" -- Repeatable Empire mission prefix

function emp.addShippingLog( text )
   shiplog.create("empire_shipping", _("Empire Shipping"), _("Empire"))
   shiplog.append("empire_shipping", text)
end

function emp.addCollectiveLog( text )
   shiplog.create("empire_collective", _("Empire Collective Campaign"), _("Empire"))
   shiplog.append("empire_collective", text)
end

emp.czesc = {
   portrait = "empire/unique/czesc.webp",
   name = _("Lieutenant Czesc"),
   colour = nil,
   transition = "pixelize",
   description = _("Lieutenant Czesc, from the Empire Armada Shipping Division, is sitting at the bar."),
}
emp.czesc.image = portrait.getFullPath(emp.czesc.portrait)
emp.soldner = {
   portrait = "empire/unique/soldner.webp",
   name = _("Commander Soldner"),
   colour = nil,
   transition = "pixelize",
   description = _("You see Commander Soldner. He is expecting you."),
}
emp.soldner.image = portrait.getFullPath(emp.soldner.portrait)

function emp.vn_czesc( params )
   return vn.Character.new( emp.czesc.name,
      tmerge( {
         image=emp.czesc.image,
         colour=emp.czesc.colour,
      }, params) )
end

function emp.vn_soldner( params )
   return vn.Character.new( emp.soldner.name,
      tmerge( {
         image=emp.soldner.image,
         colour=emp.soldner.colour,
      }, params) )
end


emp.rewards = {
   cargo00 = 100e3,
   -- Empire Shipping
   es00 = 500e3,
   es01 = 500e3, -- + "Heavy Weapons Combat License" permission
   es02 = 750e3, -- + "Heavy Combat Vessel License" permission
   -- Long Distance Cargo
   ldc1 = 500e3,
   ldc2 = 500e3,
   ldc3 = 500e3,
   ldc4 = 500e3,
   ldc5 = 500e3,
   ldc6 = 500e3,
   -- Collective Campaign
   ec00 = 500e3,
   ec01 = 600e3,
   ec02 = 700e3,
   ec03 = 1e6,
   ec04 = 1e6,
   ec05 = 2e6,
   ec06 = 5e6, -- + "Left Boot" accessory
}

return emp
