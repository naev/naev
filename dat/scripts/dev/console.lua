--[[

   Development scripts

A sort of useful scripts that will be loaded right away in the console, allowing for quick debugging.

--]]
local fmt = require "format"

local function _disp( v, mul )
   if v~=nil then
      local mul_str=""
      if mul>1 then
         mul_str=" (x"..tostring(mul)..")"
      end
      print("   "..tostring(v)..mul_str)
   end
end

-- luacheck: globals inspect
function inspect( t )
   t = t or player.pilot():target() or player.pilot()

   --print(fmt.f("Pilot: {pilot}", {pilot = t}))
   local leader = t:leader()
   if leader then
      print(fmt.f("Leader: {leader}", {leader = leader}))
   end
   print(fmt.f("AI: {ainame}", {ainame = t:ainame()}))
   local taskname, subtask = t:taskname()
   print(fmt.f("Task: {taskname}", {taskname = taskname or "N/A"}))
   if subtask then
      print(fmt.f("   Subtask: {subtask}", {subtask = subtask}))
   end
   local taskdata = t:taskdata()
   if taskdata then
      print(fmt.f("   Data: {data}", {data = taskdata}))
   end

   -- Print outfits if applicable
   local outfits = t:outfits()
   if #outfits <= 0 then
      print("No outfits")
   else
      local prv=nil
      local acc=0
      print("Outfits:")
      for k,v in ipairs(outfits) do
         if v then
            if v==prv then
               acc=acc+1
            else
               _disp(prv,acc)
               prv=v
               acc=1
            end
         end
      end
      _disp(prv,acc)
   end

   print(fmt.f("Mass: {mass} / {limit}",  {mass = t:mass(), limit = t:shipstat('engine_limit')}))
   print(fmt.f("Speed: {vel} / {spmax}",  {vel = fmt.number(t:vel():dist()), spmax = fmt.number(t:speedMax())}))

   -- Print intrinsics if applicable
   local intrinsics = t:outfitsList("intrinsic")
   if #intrinsics > 0 then
      print("Intrinsics:")
      for k,v in ipairs(intrinsics) do
         if v then
            print("   "..tostring(v))
         end
      end
   end

   -- See if anything seems wrong
   local spaceworthy, reason = t:spaceworthy()
   if not spaceworthy then
      print(fmt.f("Not spaceworthy because of: {reason}", {reason = reason}))
   end
   local stats = t:stats()
   if stats.cpu < 0 then
      print(fmt.f("Has {cpu} CPU remaining!", {cpu = stats.cpu}))
   end
   if stats.fuel < 0 then
      print(fmt.f("Has {fuel} fuel!", {fuel = stats.fuel}))
   end
end

-- luacheck: globals pprint
pprint = require "dev.pprint"

-- luacheck: globals dev
dev = {}
dev.weapon_test = require "dev.weapon_test"
dev.system_tour = function () naev.eventStart("System Tour") end

-- Shows all the map to the player
dev.show_map = function ()
   for k,s in ipairs(system.getAll()) do
      s:setKnown( true, true )
   end
end

-- Lazy load equipopt as it requires outfits to be loaded and such
local equipopt
dev.debug_goodness = function (...)
   equipopt = require "equipopt"
   return equipopt.optimize.debug_goodness(...)
end
