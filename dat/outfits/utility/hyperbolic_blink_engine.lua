masslimit = 5000^2 -- squared
jumpdist = 2000
cooldown = 8
warmup = 3
penalty = -50

function init( p, po )
   po:state("off")
   mem.timer = 0
   mem.warmup = false
end

function update( p, po, dt )
   mem.timer = mem.timer - dt
   if mem.timer < 0 then
      if mem.warmup then
         -- Blink!
         mem.warmup = false
         local dist = jumpdist
         local m = p:mass()
         m = m*m
         -- We use squared values here so twice the masslimit is 25% efficiency
         if m > masslimit then
            dist = dist * masslimit / m
         end
         -- Direction is random
         p:setPos( p:pos() + vec2.newP( dist, 359*rnd.rnd() ) )
         -- TODO Add blink effect and sound effect

         -- Set cooldown and maluses
         po:state("cooldown")
         po:set("thrust_mod", penalty)
         po:set("turn_mod", penalty)
      else
         -- Cooldown is over
         po:state("off")

         -- Cancel maluses
         po:set("thrust_mod", 0)
         po:set("turn_mod", 0)
      end
   end
end

function ontoggle( p, po, on )
   -- Only care about turning on (outfit never has the "on" state)
   if not on then return end

   -- Not ready yet
   if mem.timer > 0 then return end

   -- Start the warmup
   mem.warmup = true
   mem.timer = warmup
   -- TODO add warm-up sound effect
end
