local misnname = "Black Cat"

return function ()
   -- Must not have been done yet
   if player.misnDone(misnname) or player.misnActive(misnname) then
      return
   end

   -- Must have Wild Ones presence
   if system.cur():presence("Wild Ones") <= 0 then
      return
   end

   return {
      mission = misnname,
      weight = 2,
   }
end
