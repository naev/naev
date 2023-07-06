--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Quadtree Benchmark">
 <location>none</location>
 <chance>0</chance>
</event>
--]]
--[[
   Used to perform grid search on the quadtree parameters to find the best set.
   Trigger it with naev.eventStart("Quadtree Benchmark")
   When finishes, outputs a csv table that can be used directly
--]]
local TRIES = 5

local tests = {}
function create ()
   for i,max_elem in ipairs{4,8,16,32,64,128,256} do
      for j,depth in ipairs{1,2,3,4,5,6,7,8} do
         local e = {
            max_elem = max_elem,
            depth = depth,
            avg = {},
            wrst = {},
            elapsed = {},
         }
         table.insert( tests, e )
      end
   end

   player.pilot():setPos( vec2.new(1e6, 1e6) )
   player.pilot():setVel( vec2.new() )

   hook.timer( 0, "donext" )
   hook.custom( "benchmark", "donext" )
end

local function doaverage( tbl )
   local ret = 0
   for k,v in ipairs(tbl) do
      ret = ret + v
   end
   return ret / #tbl
end

local cur = 1
function donext( data )
   if type(data)=="table" then
      table.insert( tests[cur].avg, data.avg )
      table.insert( tests[cur].wrst, data.wrst )
      table.insert( tests[cur].elapsed, data.elapsed )
   end
   if #tests[cur].avg >= TRIES then
      cur = cur+1
   end
   local curtest = tests[ cur ]
   if not curtest then
      print("max_elem, depth,   avg,  wrst")
      for k,t in ipairs(tests) do
         local avg = doaverage( t.avg )
         local wrst = doaverage( t.wrst )
         print(string.format("% 8d,% 6d, %.2f, %.2f",t.max_elem,t.depth,avg,wrst))
      end
      evt.finish()
      return
   end
   naev.quadtreeParams( curtest.max_elem, curtest.depth )
   naev.eventStart("Skirmish Benchmark") -- triggers a player.teleport that applies the quadtrees
end
