local max_absorb = 200 -- Absorbation per second
local efficiency = 0.5 -- How much armour each energy can heal

function init( _p, po )
   mem.absorbed = 0
   mem.died = false
   po:state("off")
end

function update( _p, po, dt )
   mem.absorbed = math.max( 0, mem.absorbed - max_absorb * dt )
   if mem.absorbed <= 0 then
      po:state("off")
   else
      po:state("cooldown")
      po:progress( mem.absorbed / max_absorb )
   end
end

function onhit( p, po, armour, _shield, _attacker )
   if mem.died then return end
   local a = p:armour(true)
   if armour >= a then -- Lethal damage
      local abs = math.min( max_absorb - mem.absorbed, armour-a+1 )
      local e = p:energy(true)
      abs = math.min( e*efficiency, abs ) -- Check if enough energy

      if a + abs > armour then
         p:setHealthAbs( 1 ) -- Keep alive

         p:setEnergy( e-abs/efficiency, true ) -- Drain health
         mem.absorbed = mem.absorbed + abs -- Count damage as absorbed

         -- Update state
         po:state("cooldown")
         po:progress( mem.absorbed / max_absorb )
      else
         mem.died = true
      end
   end
end
