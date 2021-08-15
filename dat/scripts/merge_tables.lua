local merge_tables = {}

function merge_tables.merge_tables( dest, src )
   src = src or {}
   for k,v in pairs(src) do
      dest[k] = v
   end
   return dest
end

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
