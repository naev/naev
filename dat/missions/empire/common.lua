--[[
-- Common Empire Mission framework
--
-- This framework allows to keep consistency and abstracts around commonly used
--  empire mission functions.
--]]


--[[
-- @brief Gets a random official portrait name.
--
-- @return A random official portrait name.
--]]
function emp_getOfficialRandomPortrait ()
   local portraits = {
      "empire1",
      "empire2",
      "empire3",
      "empire4"
   }

   return portraits[ rnd.rnd( 1, #portraits ) ]
end
