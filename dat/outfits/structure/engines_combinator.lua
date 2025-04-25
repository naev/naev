
local fmt = require "format"

notactive = true -- Doesn't become active
hidestats = true -- We do hacks to show stats, so hide them


local engines_comb_dir = "_ec"

local mobility_params = {"accel","turn","speed"}
local eml_name

local function mkstat()
   local out = {}
   local needs_avg = {}
   for _,k in ipairs(mobility_params) do
      needs_avg[k] = true
   end
   for k,s in ipairs(naev.shipstats()) do
      if needs_avg[s.name] then
         out[k] = s
      elseif s.name == "engine_limit" then
         eml_name = s.display
      end
   end
   return out
end

local mobility_stats = mkstat()

-- inverted unit name display
local function adddesc( t, total)
   local out = ""
   for _i,k in pairs(mobility_stats) do
      local name = k['name']

      k['val']=t[name]
      out = out .. _(fmt.f("\t#g{display} {val} {unit}#0",k))
      if total then
         out =out .. "  #y[" .. fmt.number(t[name]*t['engine_limit']/total) .. " " .. k.unit .. "]#0"
      end
      out = out .. "\n"
   end
   return out
end

descextra=function ( p, _o, _po)
   if p == nil then
      return "Can't see it due to the descextra p==nil bug."
   end

   local sm = p:shipMemory()
   local dat = ((sm and sm[engines_comb_dir]) or {})
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
      local total = (sm and sm[engines_comb_dir.."_total"]) or {}
      for i,v in pairs(dat) do
         local outfit_name = p:outfitGet(i):name()
         if v['engine_limit'] and v['halted']~=true then
            out = out .. "#g[" .. tostring(i) .. "]#0 " .. outfit_name .. " : #y" .. fmt.number(v['part']) .."%#0 of " .. eml_name .."\n"
            out = out .. adddesc(v, total['engine_limit'])
         else
            out = out .. "#g[" .. tostring(i) .. "]#0 " .. outfit_name .. " : #rHALTED#0\n"
         end
      end
      out = out .. "\n#y[TOTAL]#0\n"
      out = out .. adddesc(total)
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

   if not sm[engines_comb_dir.."_needs_refresh"] then
      --print ("Unneeded refresh")
      return
   end

   local data = ((sm and sm[engines_comb_dir]) or {})
   local dataon = {} -- the subset of if that is active
   local comb = ((sm and sm[engines_comb_dir.."_total"]) or {})
   sm[engines_comb_dir.."_total"] = comb

   for k,v in pairs(data) do
      if v['engine_limit'] and v['halted']~=true then
         dataon[k] = v
      end
   end

   for _,s in ipairs(mobility_params) do
      comb[s] = 0
   end
   comb['engine_limit'] = 0

   local count = 0
   for k,v in pairs(dataon) do
      count = count + 1
   end

   po:clear()
   if count>0 then
      local den=0

      for k,v in pairs(dataon) do
         den = den + v['engine_limit']
      end
      if den > 0 then
         comb['engine_limit'] = den
         for k,v in pairs(dataon) do
            data[k]['part'] = math.floor(0.5 + (100*v['engine_limit'])/den)
         end
         for _,s in ipairs(mobility_params) do
            local acc = 0
            for _k,v in pairs(dataon) do
               acc = acc + (v[s] or 0) * v['engine_limit']
            end
            local val = math.floor( 0.5 + (acc / den) )
            comb[s] = val
            po:set(s, val)
         end
      end
   end
   sm[engines_comb_dir.."_needs_refresh"]= nil
end

init=onadd

