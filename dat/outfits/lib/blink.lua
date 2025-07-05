-- Library to help with blink drives and friends
local lib = {}

local bonuses = {
   ["Jack's Eyepatch"] = 1, -- Doubles range
}

function lib.bonus_range( p )
   local bonus = 1
   for k,o in ipairs(p:outfitsList()) do
      local b = bonuses[ o:nameRaw() ] or 0
      bonus = bonus + b
   end
   return bonus
end

return lib
