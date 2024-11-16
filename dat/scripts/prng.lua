--[[--
Simple deterministic Lua PRNG. Meant when you want worse quality random numbers
and more deterministic than what the rnd module provides.

@module prng
--]]

--[[--
A pseudo random number generator class.

@type prng
--]]
local prng = { z = 1 }
function prng:call(...)
   return self:random(...)
end
local prng_mt = { __index = prng, __call = prng.call }
--[[--
Creates a new pseudo random number generator.

   @tparam[opt=1] string|number seed Seed to initialize with.
   @treturn prng A new PRNG object.
--]]
function prng.new( seed )
   local p = {}
   setmetatable( p, prng_mt )
   seed = seed or 1
   p:setSeed( seed )
   return p
end

--[[--
Sets the seed of a PRNG object.

   @tparam string|number seed Seed to set.
--]]
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

--[[--
Gets a random integer value between min and max.

If max is omitted, a value between [1,max] is returned.

If both min and max are nil, a floating point value between [0,1] is returned.

   @tparam[opt=0] number min Minimum value to get from.
   @tparam[opt=1] number max Maximum value to get from.
   @treturn number Random number value.
--]]
function prng:random( min, max )
   if min == nil then
      return self:_rnd()
   elseif max == nil then
      return self:_range(1,min)
   else
      return self:_range(min,max)
   end
end


--[[--
Gets a value between 0 and 1.

   @treturn number Random number value.
--]]
function prng:_rnd ()
   self.z = math.abs( math.fmod( self.z * 279470273, 4294967295 ) )
   return self.z / 4294967295
end

function prng:_range( min, max )
   local n = self:_rnd()
   return math.floor( min + n*(max-min) + 0.5 )
end

return prng
