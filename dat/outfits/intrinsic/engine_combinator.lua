
notactive = true -- Doesn't become active

-- in order
local mobility_params = {"accel","turn","speed"}

local fmt = require "format"

descextra=function ( p, _o, _po)
   if p == nil then
      return "Can't see it due to the descextra p==nil bug."
   end

   local sm = p:shipMemory()
   local count = sm and sm._engine_count or 0

   local out
   if count == 0 then
      out = "#o".._("No engine equipped").."#0\n"
      count = 1
   else
      out = "#o"..fmt.f(_("Engines equipped: {count}"), {count = "#g"..fmt.number(count).."#0"}).."#0\n"
   end
   for _,s in ipairs(mobility_params) do
      if sm["_"..s] then
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
   local last = 0
   if intrinsics then
      for _,v in ipairs(intrinsics) do
         if v == o then
            last = last + 1
         end
      end
      for _,v in ipairs(intrinsics) do
         if v == o then
            ocount = ocount + 1
            if ocount == last then
               break
            end
            p:outfitRmIntrinsic(v)
            print ("Removed Engine Combinator in excess.")
         end
      end
   end

   local sm = p:shipMemory()
   local count = sm._engine_count or 0

   po:clear()
   if count>0 then
      for _,s in ipairs(mobility_params) do
         po:set(s, (sm["_"..s] or 0)/count)
      end
   else
      for _,s in ipairs(mobility_params) do
         sm["_"..s] = nil
      end
   end
end

init=onadd

