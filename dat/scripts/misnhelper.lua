--[[

   Mission Helper

--]]


--[[
-- @brief Wrapper for player.misnActive that works on a table of missions.
--
-- @usage if anyMissionActive( { "Cargo", "Cargo Rush" } ) then -- at least one Cargo or Cargo Rush is active
--
--    @luaparam names Table of names of missions to check
--    @luareturn true if any of the listed missions are active
--
-- @luafunc anyMissionActive( names )
--]]
function anyMissionActive( names )
   for i, j in ipairs( names ) do
      if player.misnActive( j ) then
         return true
      end
   end

   return false
end
