--[[
   Script to update outfits and ships from a saved game in the case they don't exist.
--]]

local outfit_list = {
   ["Cargo Pod"] = "Small Cargo Pod",
   ["Fuel Pod"] = "Small fuel Pod",
}
--[[--
   Takes an outfit name and should return either a new outfit name or the amount of credits to give back to the player.
--]]
function outfit( name )
   return outfit_list[name]
end
