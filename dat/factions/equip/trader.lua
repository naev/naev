require "factions/equip/generic"


-- Probability of cargo by class.
equip_classCargo["Yacht"] = .8
equip_classCargo["Luxury Yacht"] = .8
equip_classCargo["Scout"] = .1
equip_classCargo["Courier"] = .8
equip_classCargo["Freighter"] = .8
equip_classCargo["Armoured Transport"] = .8
equip_classCargo["Fighter"] = .2
equip_classCargo["Bomber"] = .2
equip_classCargo["Corvette"] = .3
equip_classCargo["Destroyer"] = .4
equip_classCargo["Cruiser"] = .4
equip_classCargo["Carrier"] = .6
equip_classCargo["Drone"] = .1
equip_classCargo["Heavy Drone"] = .1


--[[
-- @brief Does trader pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
