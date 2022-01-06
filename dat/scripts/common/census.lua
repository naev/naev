--[[
   Helper for census missions (where the player has to get in range
   of a given number of ships of a certain faction)
]]
local lmisn = require "lmisn"
local fmt   = require "format"

local cens = {}

-- Test if an element is in a list
local function elt_inlist( elt, list )
   for i, elti in ipairs(list) do
      if elti == elt then
         return true
      end
   end
   return false
end

-- Gets a proper target system for a census mission.
function cens.findTarget( minrange, maxrange, fact_name, min_pres )
   local systems = lmisn.getSysAtDistance( system.cur(), minrange, maxrange, lmisn.sysFilters.faction( fact_name, min_pres ) )
   if #systems == 0 then return nil end -- In case no suitable systems are in range.
   local index = rnd.rnd(1, #systems)
   return systems[index]
end

-- Returns payment and number of ships to scan
function cens.calculateNb( system, fact_list )
   local p = system:presences()
   local pres = 0
   for i, fact in ipairs(fact_list) do
      local pp = p[ fact:nameRaw() ]
      if pp ~= nil then
         pres = pres + p[ fact:nameRaw() ]
      end
   end
   local nbships = math.floor( pres/(20*(3+rnd.rnd())) ) + 2
   local credits = (20+nbships)*(5+rnd.rnd())*200
   return nbships, credits
end

-- Print osd
function cens.osd( title, sys, nb, det, fact, target )
   local str
   if nb == 1 then
      str = _("Go to {sys} and detect {nb} {target} ship ({nbl} left)") -- nb=1
   else
      str = _("Go to {sys} and detect {nb} {target} ships ({nbl} left)")
   end
   misn.osdCreate( title, {
      fmt.f(str, {sys=sys, nb=nb, target=target, nbl=(nb-det)} ),
      fmt.f(_("Land on any {fact} planet or station"), {fact=fact}),
   } )
end

-- Test pilots in sensor range
-- Remark: the list `detected` contains pilots that no longer exist.
-- This is normal behaviour as they have been counted
function cens.testInRange( detected, fact_list )
   local visibles = player.pilot():getVisible()
   for i, p in ipairs(visibles) do -- Has to be visible
      local d, s = player.pilot():inrange(p)
      if s then -- Fuzzy not allowed
         if elt_inlist( p:faction(), fact_list ) then -- Has to be in factions list
            if not elt_inlist( p, detected ) then -- Has not to be in pilots list
               detected[#detected+1] = p
               player.msg( fmt.f(_("Data on a {fact_name} ship acquired"), {fact_name=p:faction():name()} ) )
            end
         end
      end
   end
   return detected
end

return cens
