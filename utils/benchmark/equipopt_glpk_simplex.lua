local equipopt = require 'equipopt'
local reps = 100
function benchmark( testname, sparams )
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

local csvfile = file.new("benchmark.csv")
csvfile:open("w")
csvfile:write( "mean,stddev,meth,pricing,r_test,presolve" )
for i=1,reps do
   csvfile:write(string.format(",rep%d",i))
end
csvfile:write("\n")

function csv_writereps( vals )
   for i,v in ipairs(vals) do
      csvfile:write(string.format(",%f",v))
   end
   csvfile:write("\n")
end

local tstart = naev.clock()
print("====== BENCHMARK START ======")
local bl_mean, bl_stddev, bl_vals = benchmark( "Baseline" )
local def_mean, def_stddev, def_vals = benchmark( "Defaults", {} )
csvfile:write(string.format("%f,%f,-,-,-,-", bl_mean, bl_stddev ) )
csv_writereps( bl_vals )
csvfile:write(string.format("%f,%f,def,def,def,def", def_mean, def_stddev ) )
csv_writereps( def_vals )
local curbest = def_mean
local trials = {}
for i,meth in ipairs{"primal","dual","dualp"} do
   for i,pricing in ipairs{"std","pse"} do
      for i,r_test in ipairs{"std","har"} do
         for i,presolve in ipairs{"on","off"} do
            -- Randomize in the hope of getting "close" in the search space earlier.
            -- May help data quality *a little* if the machine's speed varies over time too.
            table.insert(trials, 1+rnd.rnd(#trials), {meth, pricing, r_test, presolve} )
         end
      end
   end
end
for n, trial in ipairs(trials) do 
   local meth, pricing, r_test, presolve = table.unpack(trial)
   local s = string.format("meth=%s,pricing=%s,r_test=%s,presolve=%s",
      meth, pricing, r_test, presolve )
   local mean, stddev, vals = benchmark( s, {meth=meth, pricing=pricing, r_test=r_test, presolve=presolve} )
   csvfile:write( string.format("%f,%f,%s,%s,%s,%s", mean, stddev, meth, pricing, r_test, presolve ) )
   csv_writereps( vals )
   if mean < curbest then
      curbest = mean
   end
   local left = #trials - n
   local elapsed = naev.clock() - tstart
   print(string.format("Best: %.3f ms, Cur: %.3f (%.3f) ms, (%d of %d done, %.3f hours left)",
   curbest, mean, stddev, 1+n, 1+#trials, elapsed * left / (1+n) / 3600) )
end
--[[
local glpk_mean, glpk_stddev, glpk_vals = benchmark( "GLPK", { br_tech="dth", bt_tech="blb", pp_tech="all", sr_heur="on", fp_heur="off", ps_heur="off", gmi_cuts="off", mir_cuts="off", cov_cuts="off", clq_cuts="off"} )
--local def_mean, def_stddev, def_vals = benchmark( "Defaults", {} )
print( string.format( "% 10s: %.3f (%.3f)", "GLPK", glpk_mean, glpk_stddev ) )
print( string.format( "% 10s: %.3f (%.3f)", "Defaults", def_mean, def_stddev ) )
--]]
print("====== BENCHMARK END ======")
csvfile:close()

