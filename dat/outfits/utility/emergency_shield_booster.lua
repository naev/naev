active = 4 -- active time in seconds
cooldown = 8 -- cooldown time in seconds
boost = 5 -- How much the shield regen is increased by
efficiency = 3 -- MJ of energy used per shield recovered


function turnon( p, po )
   -- Still on cooldown
   if mem.timer > 0 then
      return false
   end
   po:state("on")
   po:progress(1)
   mem.active = true

   -- Apply effect
   -- We would want to recompute this effect every update, but this leads to
   -- the effect affecting itself and going to near infinity
   local ps = p:stats()
   local regen = boost * ps.shield_regen
   po:set( "shield_usage", -regen )
   po:set( "energy_loss", efficiency * regen )

   mem.timer = active

   return true
end

function turnoff( p, po )
   if not mem.active then
      return false
   end
   po:state("cooldown")
   po:progress(1)
   po:clear() -- clear stat modifications
   mem.timer = cooldown
   mem.active = false
   return true
end

function init( p, po )
   turnoff()
   mem.timer = 0
   po:state("off")
end

function update( p, po, dt )
   mem.timer = mem.timer - dt
   if mem.active then
      po:progress( mem.timer / active )
      local a, s = p:health()
      if mem.timer < 0 or s > 99 then
         turnoff( p, po )
      end
   else
      po:progress( mem.timer / cooldown )
      if mem.timer < 0 then
         po:state("off")
      end
   end
end

function ontoggle( p, po, on )
   if on then
      return turnon( p, po )
   else
      return turnoff( p, po )
   end
end
