
function init( p )
   -- Pirates get twice the effect of stellar winds
   if system.cur():tags().stellarwind then
      print("fuel regen!")
      p:shippropSet( "fuel_regen", 1 )
   else
      print("no fuel regen!")
      p:shippropSet( "fuel_regen", 0 )
   end
end
