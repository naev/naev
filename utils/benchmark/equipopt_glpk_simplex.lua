local benchmark = require "utils.benchmark.equipopt_glpk_common"

local reps = 100

local csvfile = benchmark.csv_open( "mean,stddev,meth,pricing,r_test,presolve", reps )

local tstart = naev.clock()
print("====== BENCHMARK START ======")
local bl_mean, bl_stddev, bl_vals = benchmark.run( "Baseline", reps )
local def_mean, def_stddev, def_vals = benchmark.run( "Defaults", reps, {} )
csvfile:write(string.format("%f,%f,-,-,-,-", bl_mean, bl_stddev ) )
benchmark.csv_writereps( csvfile, bl_vals )
csvfile:write(string.format("%f,%f,def,def,def,def", def_mean, def_stddev ) )
benchmark.csv_writereps( csvfile, def_vals )
local curbest = def_mean
local trials = {}
for  row in benchmark.product(
      {"primal","dual","dualp"},
      {"std","pse"},
      {"std","har"},
      {"on","off"} ) do
   -- Randomize in the hope of getting "close" in the search space earlier.
   -- May help data quality *a little* if the machine's speed varies over time too.
   table.insert( trials, 1+rnd.rnd(#trials), row )
end
for n, trial in ipairs(trials) do
   local meth, pricing, r_test, presolve = table.unpack(trial)
   local s = string.format("meth=%s,pricing=%s,r_test=%s,presolve=%s",
      meth, pricing, r_test, presolve )
   local mean, stddev, vals = benchmark.run( s, reps, {meth=meth, pricing=pricing, r_test=r_test, presolve=presolve} )
   csvfile:write( string.format("%f,%f,%s,%s,%s,%s", mean, stddev, meth, pricing, r_test, presolve ) )
   benchmark.csv_writereps( csvfile, vals )
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
