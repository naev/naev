
notactive = true -- Doesn't become active

local needs_avg = {
   speed = true,
   turn = true,
   accel = true,
}

local fmt = require "format"

descextra=function ( p, _o, _po)
   if p == nil then
      return "Can't see it due to the descextra p==nil bug."
   end

   local sm = p:shipMemory()
   local count = sm._engine_count or 0

   if count == 0 then
      return "No engine equipped."
   end

   local out = "#bEngines equipped:#0 #g" .. count .. "#0\n"
   if count>0 then
      for s,_ in pairs(needs_avg) do
         out = out .. "#g" .. s .. ": " .. fmt.number(sm["_"..s]/count) .. "#0\n"
      end
   end
   return out
end

function onadd( p, po )
   if not p or not po then
      return
   end

   local o = po:outfit()
   if not o then
      return
   end

   local intrinsics = p:outfitsList("intrinsic")
   local ocount = 0
   if intrinsics then
      for _,v in ipairs(intrinsics) do
         if v == o then
            ocount = ocount + 1
            if ocount > 1 then
               p:outfitRmIntrinsic(v)
            end
         end
      end
   end

   local sm = p:shipMemory()
   local count = sm._engine_count or 0

   po:clear()
   if count>0 then
      --local acc=""
      for s,_ in pairs(needs_avg) do
         po:set(s,(sm["_"..s] or 0)/count)
         --acc = acc .. fmt.f("  {k} {v}",{k=s,v=fmt.number(sm["_"..s]/count)})
      end
      --print(acc)
   end
end

init=onadd

