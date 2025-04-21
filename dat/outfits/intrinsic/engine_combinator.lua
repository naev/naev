
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

   if sm._engine_count == 0 then
      return "No engine equipped."
   end

   local out = "Engines equipped:" .. count .. "\n"
   if count>0 then
      -- temporary workaround
      count = count * 2
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
   if intrinsics then
      for _,v in ipairs(intrinsics) do
         if v == o then
            p:outfitRmIntrinsic(v)
            break
         end
      end
   end

   local sm = p:shipMemory()
   local count = sm._engine_count

   po:clear()
   if count and count~=0 then
      local acc=""
      for s,_ in pairs(needs_avg) do
         po:set(s,sm["_"..s]/count)
         acc = acc .. fmt.f("  {k} {v}",{k=s,v=fmt.number(sm["_"..s]/count)})
      end
      print(acc)
   end
end

init=onadd

