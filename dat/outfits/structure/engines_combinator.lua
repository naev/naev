
local fmt = require "format"
local multiengines = require "outfits/lib/multiengines"

local mobility_stats = multiengines.mobility_stats
local eml_name = multiengines.eml_stat.display


notactive = true -- Doesn't become active
hidestats = true -- We do hacks to show stats, so hide them


-- inverted unit name display
local function adddesc( t, total)
   local out = ""
   for _i,k in pairs(mobility_stats) do
      local name = k['name']

      k['val']=t[name]
      out = out .. fmt.f(_("\t#g{display} {val} {unit}#0"),k)
      if total then
         out =out .. "  #y[" .. fmt.number(t[name]*t['engine_limit']/total) .. " " .. k.unit .. "]#0"
      end
      out = out .. "\n"
   end
   return out
end

descextra = function ( p, _o, po)
   local dat = multiengines.stats( p )

   if dat == nil then
      return "This outfit is not supposed to be off its slot."
   end

   local count = 0
   for i,v in pairs(dat) do
      if v['engine_limit'] and v['halted']~=true then
         count = count + 1
      end
   end

   local out="\n"
   if count == 0 then
      out = "#o".._("No active engine equipped").."#0\n"
   else
      local total = multiengines.total(p)
      for i,v in pairs(dat) do
         local outfit_name = p:outfitGet(i):name()
         local engine_number = i - ((po and po:id()) or 0)

         if v['engine_limit'] and v['halted'] ~= true then
            out = out .. "#g[" .. tostring(engine_number) .. "]#0 " .. outfit_name .. " : #y" .. fmt.number(v['part']) .."%#0 of " .. eml_name .."\n"
            out = out .. adddesc(v, total['engine_limit'])
         else
            out = out .. "#g[" .. tostring(engine_number) .. "]#0 " .. outfit_name .. " : #rHALTED#0\n"
         end
      end
      out = out .. "\n#y[TOTAL]#0\n"
      out = out .. adddesc(total)
   end
   return out
end

init = multiengines.refresh

