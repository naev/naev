--[[

   Simple deterministic Lua PRNG. Meant when you want worse quality random
    numbers and more deterministic than what the rnd module provides.

--]]


prng = { z = 1 }

function prng.init( num )
   prng.z = num
end

function prng.num ()
   prng.z = math.abs( math.fmod( prng.z * 279470273, 4294967295 ) )
   return prng.z / 4294967295
end

