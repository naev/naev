
return function (po)
   if po and po:slot().tags and po:slot().tags.core then
      if po:slot().tags.secondary then
         return true,false
      else
         return false,true
      end
   else
      return false,false
   end
end

