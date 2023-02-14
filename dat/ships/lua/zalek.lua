local explosion = require "luaspfx.explosion"
local emp = require "luaspfx.emp"

function explode_init( p )
   local s = p:stats()
   local armour_max, shield_max = s.armour, s.shield
   local sw, sh = p:ship():dims()

   mem.exploded = false
   mem.dtimer = math.min( 2 + math.sqrt( armour_max * shield_max ) / 400, 4 + math.pow( armour_max * shield_max, 0.4) / 300 )
   mem.timer = 0
   mem.r = (sw+sh)*0.25 -- Radius
end

function explode_update( p, dt )
   mem.timer = mem.timer - dt
   mem.dtimer = mem.dtimer - dt

   if not mem.exploded then
      if mem.timer < 0 then
         mem.timer = 0.05 * (mem.dtimer - mem.timer) / mem.dtimer

         local pos = p:pos() + vec2.newP( mem.r*rnd.rnd(), rnd.angle() )

         -- TODO maybe explosions should be under player when applicable...
         if rnd.rnd() < 0.3 then
            local r = mem.r * (0.2 + 0.3 * rnd.rnd())
            explosion( pos, p:vel(), r, nil, {silent=true} )
         else
            local r = math.max( 10, mem.r * (0.1 + 0.1 * rnd.rnd()) )
            emp( pos, p:vel(), r, nil, {silent=true} )
         end
      end

      if mem.dtimer < 0.25 then
         mem.exploded = true
         local a = math.sqrt( p:mass() )
         local r = mem.r * 1.0 + a
         local d = math.max( 0, 2 * (a * (1+math.sqrt(p:fuel()+1)/28)))
         local params = {
            penetration = 1,
         }
         spfx.debris( p:mass(), mem.r, p:pos(), p:vel() )
         explosion( p:pos(), p:vel(), r, d, params )
         p:cargoJet("all")
      end
   end

   if mem.dtimer < 0 then
      p:rm()
   end
end
