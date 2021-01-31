--[[

   Simple deterministic Lua PRNG. Meant when you want worse quality random
    numbers and more deterministic than what the rnd module provides.

--]]

local prng = { z = 1 }
function prng:call()
   return self:random()
end
local prng_mt = { __index = prng, __call = prng.call }
function prng.new( seed )
   local p = {}
   setmetatable( p, prng_mt )
   p:setSeed( num )
   return p
end

function prng:setSeed( seed )
   if type(seed)=="number" then
      self.z = seed or 1
   elseif type(seed)=="string" then
      local hash  = 5381
      local bytes = { string.byte( seed, 1, string.len(seed) ) }
      for _,c in ipairs(bytes) do
         hash = hash * 33 + c
      end
      self.z = math.abs( math.fmod( hash, 4294967295 ) )
   end
end

function prng:random( min, max )
   if min == nil then
      return self:_rnd()
   elseif max == nil then
      return self:_range(1,min)
   else
      return self:_range(min,max)
   end
end

-- Value between 0 and 1
function prng:_rnd ()
   self.z = math.abs( math.fmod( self.z * 279470273, 4294967295 ) )
   return self.z / 4294967295
end

-- Value between min and max
function prng:_range( min, max )
   local n = self:_rnd()
   return math.floor( min + n*(max-min) + 0.5 )
end

return prng
