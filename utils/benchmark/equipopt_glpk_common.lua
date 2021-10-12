local equipopt = require 'equipopt'
local function benchmark( testname, reps, sparams )
   local ships = {
      "Llama",
      "Hyena",
      "Ancestor",
      "Vendetta",
      "Lancelot",
      "Admonisher",
      "Pacifier",
      "Vigilance",
      "Kestrel",
      "Goddard",
   }
   local factions = {
      equipopt.generic,
      equipopt.empire,
      equipopt.zalek,
      equipopt.dvaered,
      equipopt.sirius,
      equipopt.soromid,
      equipopt.pirate,
      equipopt.thurion,
      equipopt.proteron,
   }

   pilot.clear()
   local pos = vec2.new(0,0)
   local vals = {}
   for i=1,reps do
      collectgarbage("collect")
      collectgarbage('stop')
      local rstart = naev.clock()
      for k,s in ipairs(ships) do
         for k,f in ipairs(factions) do
            local p = pilot.add( s, "Dummy", pos, nil, {naked=true} )
            if sparams then
               equipopt.optimize.sparams = sparams
               f( p )
               --f( p, {rnd=0.0})
            end
            p:rm()
         end
      end
      table.insert( vals, (naev.clock()-rstart)*1000 )
      collectgarbage('restart')
   end

   local mean = 0
   local stddev = 0
   for k,v in ipairs(vals) do
      mean = mean + v
   end
   mean = mean / #vals
   for k,v in ipairs(vals) do
      stddev = stddev + math.pow(v-mean, 2)
   end
   stddev = math.sqrt(stddev / #vals)

   return mean, stddev, vals
end
return benchmark
