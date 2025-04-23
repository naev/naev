
notactive = true -- Doesn't become active

-- in order
local mobility_params = {"accel","turn","speed"}

local fmt = require "format"

descextra=function ( p, _o, _po)
   if p == nil then
      return "Can't see it due to the descextra p==nil bug."
   end

   local sm = p:shipMemory()
   local dat= ((sm and sm._engines_combinator) or {})
   local count=0
   for _,v in pairs(dat) do
      if v['engine_limit'] then
         count = count + 1
      end
   end

   local out=""
   if count == 0 then
      out = "#o".._("No engine equipped").."#0\n"
   else
      for i,v in pairs(dat) do
         out = out .. "[" .. tostring(i) .. "] : " .. fmt.number(v['part']) .."%\n"
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

   local sm = p:shipMemory()
   local data = ((sm and sm._engines_combinator) or {})
   local count = 0
   for k,v in pairs(data) do
      if v['engine_limit'] then
         count = count + 1
      end
   end
   po:clear()

   if count>0 then
      local den=0

      for _k,v in pairs(data) do
         den = den + (v['engine_limit'] or 0)
      end
      if den>0 then
         for k,v in pairs(data) do
            p:shipMemory()._engines_combinator[k]['part']=(100*(v['engine_limit'] or 0))/den
         end
         for _,s in ipairs(mobility_params) do
            local acc=0
            for _k,v in pairs(data) do
               acc = acc + (v[s] or 0) * (v['engine_limit'] or 0)
            end
            po:set(s, acc / den )
         end
      end
   end
end

init=onadd

