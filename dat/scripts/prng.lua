--[[

   Simple deterministic Lua PRNG. Meant when you want worse quality random
    numbers and more deterministic than what the rnd module provides.

--]]

local prng = { z = 1 }

function prng.init( num )
   prng.z = num
end

function prng.initHash( str )
   local hash  = 5381
   local bytes = { string.byte( str, 1, string.len(str) ) }
   for _,c in ipairs(bytes) do
      hash = hash * 33 + c
   end
   prng.z = math.abs( math.fmod( hash, 4294967295 ) )
end

-- Value between 0 and 1
function prng.num ()
   prng.z = math.abs( math.fmod( prng.z * 279470273, 4294967295 ) )
   return prng.z / 4294967295
end

-- Value between min and max
function prng.range( min, max )
   local n = prng.num()
   return math.floor( min + n*(max-min) + 0.5 )
end

return prng
