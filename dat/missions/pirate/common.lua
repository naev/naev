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
      "pirate1",
      "pirate2",
      "pirate3",
      "pirate4"
   }

   return portraits[ rnd.rnd( 1, #portraits ) ]
end
