local misnname = "Derelict Rescue"

return function ()
   -- Lower duplicates by ignoring active
   if player.misnActive(misnname) then
      return
   end

   return {
      mission = misnname,
   }
end
