
return function ( p, _po )
   if p and _po then
      local o = p:outfitSlot ('engines_secondary')
      if o then
         print "Remige is Unhappy."
         -- That triggers an infinite loop:
         --if p:outfitRmSlot('engines_secondary') then
         --   print('You cannot do that.')
         --end
      else
         print "Remige is Happy."
      end
   end
end
