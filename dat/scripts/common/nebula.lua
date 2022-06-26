--[[

   Helper library about things related to the Nebula.

--]]
local nebula = {}

nebula.blacklist = {
   ["Mizar"]   = true,
   ["Haven"]   = true,
   ["Amaroq"]  = true,
   ["PSO"]     = true,
   ["Sultan"]  = true,
   ["Faust"]   = true,
}

--[[--
Checks to see if a system is part of the Nebula.
   @tparam System s System to check to see if it is part of the Nebula.
   @treturn boolean Whether or not the system is part of the Nebula.
--]]
function nebula.isNebula( s )
   if nebula.blacklist[ s:nameRaw() ] then
      return false
   end
   return (s:nebula() > 0)
end

function nebula.jumpDist( sys, hidden )
   -- Get default parameters
   sys = sys or system.cur()

   if nebula.isNebula( sys ) then
      return 0
   end

   local open  = { sys }
   local close = { [sys:nameRaw()]=sys }
   local d = 0

   -- Run max times
   while #open > 0 do
      local nopen = {}
      -- Get all the adjacent system of the current set
      for _j,s in ipairs(open) do
         local adjsys = s:adjacentSystems( hidden ) -- Get them all
         for _k,a in ipairs(adjsys) do
            if nebula.isNebula( a ) then
               return d
            end

            -- Must not have been explored previously
            if close[ a:nameRaw() ] == nil then
               nopen[ #nopen+1 ] = a
               close[ a:nameRaw() ] = a
            end
         end
      end
      open = nopen -- New table becomes the old
      d = d+1
   end

   return nil
end

return nebula
