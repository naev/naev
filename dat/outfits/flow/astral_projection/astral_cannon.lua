function onimpact( _p, target )
   -- We apply chakra corruption, but only at 10% debuff
   target:effectAdd( "Chakra Corruption", 3, 10/25 )
end
