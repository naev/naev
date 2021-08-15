function init( p, po )
   po:clear()
end

function cooldown( p, po, done, opt )
   if not done then
      -- Initialize repairs, we calculate how much is needed to repair to full
      local a, s = p:health(true)
      local ps = p:stats()
      local regen = (ps.armour - a) / opt
      po:set( "armour_regen_malus", -regen );
   else
      -- Stop repairing
      po:clear()
   end
end
