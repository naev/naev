
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
      if count == 2 and not mismatch then
         print ("Twins are happy")
      else
         print ("Twin is unhappy")
      end
   end
end
