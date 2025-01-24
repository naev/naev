function init( p )
   -- Pirates get twice the effect of stellar winds
   if system.cur():tags().stellarwind then
      p:shippropSet( "fuel_regen", 1 )
   else
      p:shippropSet( "fuel_regen", 0 )
   end
end
