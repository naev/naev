notactive = true

function init( _p, po )
   mem.a = nil
   po:clear()
end

function cooldown( p, po, done, opt )
   if not done then
      -- Initialize repairs, we calculate how much is needed to repair to full from zero
      mem.a = p:armour(true)
      local ps = p:stats()
      local regen = ps.armour / opt
      po:set( "armour_regen_malus", -regen )
   else
      -- We force set it to 100% here in the case of outfits that trigger full cycles
      if opt then
         -- Really bad way of checking, but if starting health is the same as ending health,
         -- we assume it's run in row and thus doing single cycle and needs full heal.
         local a, s, stress = p:health(true)
         if mem.a==a then
            p:setHealthAbs( a, s, stress )
         end
      end
      -- Stop repairing
      po:clear()
   end
end
