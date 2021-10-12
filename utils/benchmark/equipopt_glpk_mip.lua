local benchmark = require "utils.benchmark.equipopt_glpk_common"

local reps = 3

local csvfile = file.new("benchmark.csv")
csvfile:open("w")
csvfile:write( "mean,stddev,br_tech,bt_tech,pp_tech,sr_heur,fp_heur,ps_heur,gmi_cuts,mir_cuts,cov_cuts,clq_cuts" )
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
local ntotal = math.pow(2,7)*3*4*5 + 1
print("====== BENCHMARK START ======")
local bl_mean, bl_stddev, bl_vals = benchmark( "Baseline", reps )
local def_mean, def_stddev, def_vals = benchmark( "Defaults", reps, {} )
csvfile:write(string.format("%f,%f,-,-,-,-,-,-,-,-,-,-", bl_mean, bl_stddev ) )
csv_writereps( bl_vals )
csvfile:write(string.format("%f,%f,def,def,def,def,def,def,def,def,def,def,def", def_mean, def_stddev ) )
csv_writereps( def_vals )
local curbest = def_mean
local trials = {}
for i,br_tech in ipairs{"ffv","lfv","mfv","dth","pch"} do
   for i,bt_tech in ipairs{"dfs","bfs","blb","bph"} do
      for i,pp_tech in ipairs{"none","root","all"} do
         for i,sr_heur in ipairs{"on","off"} do
            for i,fp_heur in ipairs{"on","off"} do
               for i,ps_heur in ipairs{"on","off"} do
                  for i,gmi_cuts in ipairs{"on","off"} do
                     for i,mir_cuts in ipairs{"on","off"} do
                        for i,cov_cuts in ipairs{"on","off"} do
                           for i,clq_cuts in ipairs{"on","off"} do
                              -- Randomize in the hope of getting "close" in the search space earlier.
                              -- May help data quality *a little* if the machine's speed varies over time too.
                              table.insert(trials, 1+rnd.rnd(#trials), {br_tech, bt_tech, pp_tech,
                              sr_heur, fp_heur, ps_heur, gmi_cuts, mir_cuts, cov_cuts, clq_cuts})
                           end
                        end
                     end
                  end
               end
            end
         end
      end
   end
end
for n, trial in ipairs(trials) do 
   local br_tech, bt_tech, pp_tech, sr_heur, fp_heur, ps_heur, gmi_cuts, mir_cuts, cov_cuts, clq_cuts = table.unpack(trial)
   local s = string.format("br=%s,bt=%s,pp=%s,sr=%s,fp=%s,ps=%s,gmi=%s,mir=%s,cov=%s,clq=%s",
      br_tech, bt_tech, pp_tech,
      sr_heur, fp_heur, ps_heur,
      gmi_cuts, mir_cuts, cov_cuts, clq_cuts )
   local mean, stddev, vals = benchmark( s, reps, {
         br_tech=br_tech, bt_tech=bt_tech, pp_tech=pp_tech,
         sr_heur=sr_heur, fp_heur=fp_heur, ps_heur=ps_heur,
         gmi_cuts=gmi_cuts, mir_cuts=mir_cuts, cov_cuts=cov_cuts,
         clq_cuts=clq_cuts
      } )
   csvfile:write( string.format("%f,%f,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", mean, stddev,
      br_tech, bt_tech, pp_tech,
      sr_heur, fp_heur, ps_heur,
      gmi_cuts, mir_cuts, cov_cuts, clq_cuts ) )
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
local glpk_mean, glpk_stddev, glpk_vals = benchmark( "GLPK", reps, { br_tech="dth", bt_tech="blb", pp_tech="all", sr_heur="on", fp_heur="off", ps_heur="off", gmi_cuts="off", mir_cuts="off", cov_cuts="off", clq_cuts="off"} )
--local def_mean, def_stddev, def_vals = benchmark( "Defaults", reps, {} )
print( string.format( "% 10s: %.3f (%.3f)", "GLPK", glpk_mean, glpk_stddev ) )
print( string.format( "% 10s: %.3f (%.3f)", "Defaults", def_mean, def_stddev ) )
--]]
print("====== BENCHMARK END ======")
csvfile:close()

