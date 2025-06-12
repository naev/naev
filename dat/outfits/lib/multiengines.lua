
local tfs = require "tfs"

local multiengines = {
   mobility_params = {'accel', 'turn', 'speed', 'engine_limit'},
   is_mobility = {},
   mobility_stats = {},
}

for _k,s in ipairs(multiengines.mobility_params ) do
   multiengines.is_mobility[s] = true
end

for k,s in ipairs(naev.shipstats()) do
   if multiengines.is_mobility[s.name] then
      multiengines.mobility_stats[s.name] = s
   end
end

function multiengines.engine_stats( root, id )
   local res = tfs.readdir(root, {'engines', id})
   if res then
      local total = tfs.readdir(root, {'total'})
      res['total'] = total and total['engine_limit']
   end
   return res
end

function multiengines.refresh( root, po, force )
   if not root or not po then
      return
   end

   if (not force) and (not tfs.readfile(root, {'needs_refresh'})) then
      return
   end

   local data = tfs.checkdir(root, {'engines'})
   local dataon = {} -- the subset of if that is active
   local comb = tfs.checkdir(root, {'total'})

   mem.stats = mem.stats or {}
   for _,s in ipairs(multiengines.mobility_params) do
      comb[s] = 0
      --po:set(s, 0)
      mem.stats[s] = 0
   end

   local den=0
   for k,v in pairs(tfs.readdir(data)) do
      if v['engine_limit'] and v['halted'] ~= true then
         dataon[k] = v
         den = den + v['engine_limit']
      end
   end

   if den > 0 then
      comb['engine_limit'] = den
      for _i,s in ipairs(multiengines.mobility_params) do
         local acc = 0
         for _k,v in pairs(dataon) do
            acc = acc + (v[s] or 0) * v['engine_limit']
         end
         local val
         if s == 'engine_limit' then
            val = den
         else
            val = math.floor(0.5 + (acc/den))
         end
         comb[s] = val
         --po:set(s, val)
         mem.stats[s] = val
      end
   end
   tfs.writefile(root, {'needs_refresh'}, nil)
end

function multiengines.halt_n( root, n, what )
   local res = tfs.writefile(root, {'engines', n, 'halted'}, what)

   if res == nil then -- could not write
      warn('Could not write to tfs. (invalid pointer or path)')
   else
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
