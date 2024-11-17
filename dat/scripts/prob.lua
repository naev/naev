--[[--
   Probability-related utilities.
   @module prob
--]]
local prob = {}

--[[--
Samples from the Poisson distribution.
   @tparam number lambda Expected mean and variance of the distribution.
--]]
function prob.poisson_sample( lambda )
   local x = 0
   local p = math.exp(-lambda)
   local s = p
   local u = rnd.rnd()
   while u > s do
      x = x+1
      p = p * lambda / x
      s = s + p
   end
   return x
end

return prob
