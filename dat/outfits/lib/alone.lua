
return function ( p, po )
   if false and p and po then
      local s = p:ship()
      if s then
         local slot
         for i, v in ipairs( ship.getSlots( s ) ) do
            if v['name'] == 'engines_secondary' then
               slot = i
               break
            end
         end
         if slot then
            local o = s:outfitSlot (p, slot)
            if o:outfitRmSlot(p,'engines_secondary') then
               print 'You cannot do that.'
            end
         end
      end
   end
end
