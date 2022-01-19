local equipopt = require 'equipopt'
local benchmark = {}
function benchmark.run( _testname, reps, sparams )
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
         for k2,f in ipairs(factions) do
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

function benchmark.csv_open( header, reps )
   local csvfile = file.new("benchmark.csv")
   csvfile:open("w")
   csvfile:write( header )
   for i=1,reps do
      csvfile:write(string.format(",rep%d",i))
   end
   csvfile:write("\n")
   return csvfile
end

function benchmark.csv_writereps( csvfile, vals )
   for i,v in ipairs(vals) do
      csvfile:write(string.format(",%f",v))
   end
   csvfile:write("\n")
end

function benchmark.product( ... )
   local sets = { ... }
   local tuple = {}
   local function descend( i )
      if i == #sets then
         for k,v in pairs(sets[i]) do
            tuple[i] = v
            coroutine.yield( tuple )
         end
      else
         for k,v in pairs(sets[i]) do
            tuple[i] = v
            descend( i + 1 )
         end
      end
   end
   return coroutine.wrap(function() descend(1) end)
end

return benchmark
