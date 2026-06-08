require "ships.lua.zalek"

function init( p )
   -- Sorry player, you don't get the bonus
   if p and p:faction() ~= faction.player() then
      p:shippropReset()
      p:shippropSet( "fbay_capacity", 50 )
   end
end
