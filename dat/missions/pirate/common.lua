--[[
-- Common Pirate Mission framework
--
-- This framework allows to keep consistency and abstracts around commonly used
--  Pirate mission functions.
--]]


--[[
-- @brief Gets a random pirate lord portrait name.
--
-- @return A random pirete lord portrait name.
--]]
function pir_getLordRandomPortrait ()
   local portraits = {
      "pirate/pirate1",
      "pirate/pirate2",
      "pirate/pirate3",
      "pirate/pirate4",
      "pirate/pirate5",
   }

   return portraits[ rnd.rnd( 1, #portraits ) ]
end


function pir_modDecayFloor( n )
   local floor = var.peek("_ffloor_decay_pirate")
   if floor == nil then floor = -20 end
   floor = math.min(floor + n, -1)
   var.push("_ffloor_decay_pirate", floor)
end

