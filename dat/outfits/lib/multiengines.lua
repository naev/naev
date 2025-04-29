
local smfs = require "shipmemfs"

local multiengines_dir = "_multiengines"

local multiengines = {
   mobility_params = {"accel", "turn", "speed"},
   is_mobility = {},
   is_param = {},
   mobility_stats = {},
   -- will also have eml_stat
}

multiengines.is_param["engine_limit"] = true

for _i,k in ipairs(multiengines.mobility_params) do
   multiengines.is_mobility[k] = true
   multiengines.is_param[k] = true
end

for k,s in ipairs(naev.shipstats()) do
   if multiengines.is_mobility[s.name] then
      multiengines.mobility_stats[k] = s
   elseif multiengines.is_param[s.name] then
      multiengines.eml_stat = s
   end
end

function multiengines.total( p )
   return smfs.readdir(p, {multiengines_dir,"total"})
end

function multiengines.stats( p )
   return smfs.readdir(p, {multiengines_dir, 'engines'})
end

function multiengines.engine_stats( p, id )
   return smfs.readdir(p, {multiengines_dir, 'engines', id})
end

function multiengines.refresh( p, po )
   if not p or not po then
      return
   end

   if not smfs.readfile(p, {multiengines_dir, "needs_refresh"}) then
      --print ("Unneeded refresh")
      return
   --else
   --   print ("Needed refresh")
   end

   local data = smfs.checkdir(p, {multiengines_dir, 'engines'})
   local dataon = {} -- the subset of if that is active
   local comb = smfs.checkdir(p, {multiengines_dir, "total"})

   local count = 0
   for k,v in pairs(smfs.listdir(data)) do
      if v['engine_limit'] and v['halted'] ~= true then
         dataon[k] = v
         count = count + 1
      end
   end

   for _,s in ipairs(multiengines.mobility_params) do
      comb[s] = 0
   end
   comb['engine_limit'] = 0

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
         for _,s in ipairs(multiengines.mobility_params) do
            local acc = 0
            for _k,v in pairs(dataon) do
               acc = acc + (v[s] or 0) * v['engine_limit']
            end
            local val = math.floor(0.5 + (acc/den))
            comb[s] = val
            po:set(s, val)
         end
      end
   end
   smfs.writefile(p, {multiengines_dir, "needs_refresh"}, nil)
end

function multiengines.halted_n( p, n )
   return smfs.readfile(p, {multiengines_dir, 'engines', n, 'halted'})
end

function multiengines.halt_n( p, n, what )
   local res = smfs.writefile(p, {multiengines_dir, 'engines', n, 'halted'}, what)

   if res == nil then -- could not write
      warn("Could not write to shimemfs. (invalid pilot or path)")
   else
      return smfs.updatefile(p,{multiengines_dir, "needs_refresh"}, function ( crt )
            return crt or res
         end)
   end
end

function multiengines.lock( p )
   return smfs.writefile(p, {multiengines_dir, "lock"}, true)
end

function multiengines.unlock( p )
   smfs.writefile(p, {multiengines_dir, "lock"}, nil)
end

-- sign:
--  -1 for remove
--   0 for update
--   1 for add
function multiengines.decl_engine_stats( p, po, sign, t )
   local id = po:id()
   local changed = smfs.readfile(p, {multiengines_dir, "needs_refresh"})
   local comb = smfs.checkdir(p, {multiengines_dir, 'engines'})
   local bef

   if sign == -1 then
      changed = changed or comb[id] ~= nil
      comb[id] = nil
   else
      local combid = smfs.checkdir(p, {multiengines_dir, 'engines', id})
      changed = changed or ((sign == 1) and (comb[id] == nil))

      for k,v in pairs(t or {}) do
         bef = combid[k]
         combid[k] = v
         changed = changed or (bef ~= v)
      end
   end
   smfs.writefile(p, {multiengines_dir, "needs_refresh"}, changed)
   return changed
end

return multiengines

