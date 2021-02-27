require("factions/equip/generic")


-- Probability of cargo by class.
equip_classCargo = {
   ["Yacht"] = .8,
   ["Luxury Yacht"] = .8,
   ["Scout"] = .1,
   ["Courier"] = .8,
   ["Freighter"] = .8,
   ["Armoured Transport"] = .8,
   ["Fighter"] = .2,
   ["Bomber"] = .2,
   ["Corvette"] = .3,
   ["Destroyer"] = .4,
   ["Cruiser"] = .4,
   ["Carrier"] = .6,
   ["Drone"] = .1,
   ["Heavy Drone"] = .1
}


--[[
-- @brief Does trader pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
