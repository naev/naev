-- Reboot shield on cooldown
function cooldown ( p, done, success )
   if done and success then
      local armour, shield, stress = p:health(true)
      if shield <= 0 then
         p:setHealthAbs( armour, 1, stress )
      end
   end
end
