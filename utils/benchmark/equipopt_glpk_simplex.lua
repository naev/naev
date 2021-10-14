local benchmark = require "utils.benchmark.equipopt_glpk_common"

local reps = 100

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
local bl_mean, bl_stddev, bl_vals = benchmark( "Baseline", reps )
local def_mean, def_stddev, def_vals = benchmark( "Defaults", reps, {} )
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
   local mean, stddev, vals = benchmark( s, reps, {meth=meth, pricing=pricing, r_test=r_test, presolve=presolve} )
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
print("====== BENCHMARK END ======")
csvfile:close()

