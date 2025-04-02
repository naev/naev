--[[

   Development scripts

A sort of useful scripts that will be loaded write away in the console, allowing for quick debugging.

--]]
local fmt = require "format"

-- luacheck: globals inspect
function inspect( t )
   t = t or player.pilot():target()
   --print(fmt.f("Pilot: {pilot}", {pilot = t}))
   print(fmt.f("AI: {ainame}", {ainame = t:ainame()}))
   print(fmt.f("Task: {taskname}", {taskname = t:taskname()}))

   -- Print outfits if applicable
   local outfits = t:outfits()
   if #outfits <= 0 then
      print("No outfits")
   else
      print("Outfits:")
      for k,v in ipairs(outfits) do
         if v then
            print("   "..tostring(v))
         end
      end
   end

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
      print("#r"..fmt.f("Not spaceworthy because of: {reason}", {reason = reason}).."#0")
   end
   local stats = t:stats()
   if stats.cpu < 0 then
      print("#r"..fmt.f("Has {cpu} CPU remaining!", {cpu = stats.cpu} ).."#0")
   end
   if stats.fuel < 0 then
      print("#r"..fmt.f("Has {fuel} fuel!", {fuel = stats.fuel} ).."#0")
   end
end
