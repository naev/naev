
return function ( p, po )
   if false and p and po then
      local count = 0
      local mismatch = false
      for k,o in ipairs(p:outfits()) do
         if o and o:type() == "Core Systems (Engine)" then
            if o:name() == po:name() then
               count = count+1
            else
               mismatch = true
            end
         end
      end
      if count == 2 and not mismatch then
         print ("Ok")
      else
         print ("Not ok")
      end
   end
end
