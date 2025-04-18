
function onoutfitchange( p, po )
   if p and po then
      local count = 0
      local mismatch = false
      for _k,o in ipairs(p:outfits()) do
         if o and o:type() == "Core Systems (Engine)" then
            if o:nameRaw() == po:outfit():nameRaw() then
               count = count+1
            else
               mismatch = true
            end
         end
      end
      local ok = (count == 2 and mismatch~=true)
      if ok then
         print ("Twins are together thus happy.")
         turnon( p, po)
      else
         print ("Alone twin is unhappy.")
         turnoff( p, po)
      end
   end
end

