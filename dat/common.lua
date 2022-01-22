function inlist( tbl, elm )
   for k,v in ipairs(tbl) do
      if v==elm then
         return true
      end
   end
   return false
end
