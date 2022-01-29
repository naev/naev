function inlist( tbl, elm )
   for k,v in ipairs(tbl) do
      if v==elm then
         return true
      end
   end
   return false
end

function tcopy( tbl, copy )
   copy = copy or {}
   for k,v in pairs(tbl) do
      copy[k] = v
   end
   return copy
end

function trepeat( tbl, num )
   local t = {}
   for k,v in ipairs(tbl) do
      for i=1,num do
         table.insert( t, v )
      end
   end
   return t
end
