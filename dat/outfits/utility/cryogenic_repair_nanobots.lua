notactive = true

function init( _p, po )
   po:clear()
end

function cooldown( p, po, done, opt )
   if not done then
      -- Initialize repairs, we calculate how much is needed to repair to full
      local a = p:health(true)
      local ps = p:stats()
      local regen = (ps.armour - a) / opt
      po:set( "armour_regen_malus", -regen )
   else
      -- We force set it to 100% here in the case of outfits that trigger full cycles
      if opt then
         local _a, s = p:health()
         p:setHealth( 100, s )
      end
      -- Stop repairing
      po:clear()
   end
end
