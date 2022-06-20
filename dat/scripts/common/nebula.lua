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

return nebula
