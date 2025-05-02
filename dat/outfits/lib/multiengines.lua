
local tfs = require "tfs"

local multiengines = {
   mobility_params = {'accel', 'turn', 'speed'},
   is_mobility = {},
   is_param = {},
   mobility_stats = {},
   -- will also have eml_stat
}

multiengines.is_param['engine_limit'] = true

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

function multiengines.engine_stats( root, id )
   res = tfs.readdir(root, {'engines', id})
   if res then
      res['total'] = tfs.readdir(root, {'total'})['engine_limit']
   end
   return res
end

function multiengines.refresh( root, po, force )
   if not root or not po then
      return
   end

   if not tfs.readfile(root, {'needs_refresh'}) and not force then
      return
   end

   --po:clear()

   local data = tfs.checkdir(root, {'engines'})
   local dataon = {} -- the subset of if that is active
   local comb = tfs.checkdir(root, {'total'})

   local count = 0
   for k,v in pairs(tfs.readdir(data)) do
      if v['engine_limit'] and v['halted'] ~= true then
         dataon[k] = v
         count = count + 1
      end
   end

   for _,s in ipairs(multiengines.mobility_params) do
      comb[s] = 0
   end
   comb['engine_limit'] = 0

   if count>0 then
      local den=0

      for _k,v in pairs(dataon) do
         den = den + v['engine_limit']
      end
      if den > 0 then
         comb['engine_limit'] = den
         for k,v in pairs(dataon) do
            data[k]['part'] = math.floor(0.5 + (100*v['engine_limit'])/den)
         end
         for _i,s in ipairs(multiengines.mobility_params) do
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
   tfs.writefile(root, {'needs_refresh'}, nil)
end

function multiengines.halted_n( root, n )
   return tfs.readfile(root, {'engines', n, 'halted'})
end

function multiengines.halt_n( root, n, what )
   local res = tfs.writefile(root, {'engines', n, 'halted'}, what)

   if res == nil then -- could not write
      warn('Could not write to shimemfs. (invalid pilot or path)')
      return tfs.updatefile(root,{'needs_refresh'}, function ( crt )
            return crt or res
         end)
   end
end

-- sign:
--  -1 for remove
--   0 for update
--   1 for add
function multiengines.decl_engine_stats( root, id, sign, t )
   local changed = tfs.readfile(root, {'needs_refresh'})
   local comb = tfs.checkdir(root, {'engines'})
   local bef

   if sign == -1 then
      changed = changed or comb[id] ~= nil
      comb[id] = nil
   else
      local combid = tfs.checkdir(root, {'engines', id})
      changed = changed or ((sign == 1) and (comb[id] == nil))

      for k,v in pairs(t or {}) do
         bef = combid[k]
         combid[k] = v
         changed = changed or (bef ~= v)
      end
   end
   tfs.writefile(root, {'needs_refresh'}, changed)
   return comb[id]
end

return multiengines
