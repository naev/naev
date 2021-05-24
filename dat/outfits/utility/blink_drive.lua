masslimit = 800^2 -- squared
jumpdist = 500
cooldown = 8

function init( p, po )
   po:state("off")
   mem.timer = 0
end

function update( p, po, dt )
   mem.timer = mem.timer - dt
   po:progress( mem.timer / cooldown )
   if mem.timer < 0 then
      po:state("off")
   end
end

function ontoggle( p, po, on )
   -- Only care about turning on (outfit never has the "on" state)
   if not on then return end

   -- Not ready yet
   if mem.timer > 0 then return end
      
   -- Blink!
   local dist = jumpdist
   local m = p:mass()
   m = m*m
   -- We use squared values here so twice the masslimit is 25% efficiency
   if m > masslimit then
      dist = dist * masslimit / m
   end
   -- TODO add energy cost and SPFX?
   p:setPos( p:pos() + vec2.newP( dist, p:dir() ) )
   mem.timer = cooldown
   po:state("cooldown")
   po:progress(1)
end
