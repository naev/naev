--[[--
   Small helper for merging different tables.
   @module merge_tables
--]]
local merge_tables = {}

--[[--
   Merges src table into dest. Does not recurse into subtables.

   @tparam table dest Destination table. Must not be nil.
   @tparam[opt={}] table src Source table to be merged into dest.
   @treturn table Returns dest.
--]]
function merge_tables.merge_tables( dest, src )
   src = src or {}
   for k,v in pairs(src) do
      dest[k] = v
   end
   return dest
end

--[[--
   Merges src table into dest recursively.

   @tparam table dest Destination table. Must not be nil.
   @tparam[opt={}] table src Source table to be merged into dest.
   @treturn table Returns dest.
--]]
function merge_tables.merge_tables_recursive( dest, src )
   src = src or {}
   for k,v in pairs(src) do
      if type(v)=="table" then
         dest[k] = merge_tables.merge_tables_recursive( dest[k] or {}, v )
      else
         dest[k] = v
      end
   end
   return dest
end

return merge_tables
