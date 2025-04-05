--[[

   Development scripts

A sort of useful scripts that will be loaded right away in the console, allowing for quick debugging.

--]]
local fmt = require "format"

local function _disp(v,mul)
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
   t = t or player.pilot():target()
   --print(fmt.f("Pilot: {pilot}", {pilot = t}))
   print(fmt.f("AI: {ainame}", {ainame = t:ainame()}))
   print(fmt.f("Task: {taskname}", {taskname = t:taskname()}))

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

   print("Mass "..tostring(t:mass()).."/"..tostring(t:shipstat('engine_limit')))

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
