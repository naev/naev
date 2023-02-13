local explosion = require "luaspfx.explosion"

function explode_init( p )
   local s = p:stats()
   local armour_max, shield_max = s.armour, s.shield
   local sw, sh = p:ship():dims()

   mem.exploded = false
   mem.dtimer = math.min( 1 + math.sqrt( armour_max * shield_max ) / 650, 3 + math.pow( armour_max * shield_max, 0.4) / 500 )
   mem.timer = 0
   mem.r = (sw+sh)*0.25 -- Radius
end

local function exp_params( _size )
   return {
      colorbase = {1.0, 0.8, 0.8, 0.7},
      colorsmoke = {0.3, 0.3, 0.3, 0.1},
      smokiness = 0.4,
      --smokefade = 1.6,
      --rollspeed = 0.3,
      --grain = 0.1 + size*0.001,
   }
end

function explode_update( p, dt )
   mem.timer = mem.timer - dt
   mem.dtimer = mem.dtimer - dt
   if mem.timer < 0 then
      mem.timer = 0.08 * (mem.dtimer - mem.timer) / mem.dtimer

      local pos = p:pos() + vec2.newP( mem.r*rnd.rnd(), rnd.angle() )
      local r = mem.r * (0.2 + 0.3 * rnd.rnd())

      -- TODO maybe explosions should be under player when applicable...
      explosion( pos, p:vel(), r*1.2, nil, tmerge( {silent=true}, exp_params(r) ) )
   end

   if not mem.exploded and mem.dtimer < 0.25 then
      mem.exploded = true
      local a = math.sqrt( p:mass() )
      local r = mem.r * 1.2 + a
      local d = math.max( 0, 2 * (a * (1+math.sqrt(p:fuel()+1)/28)))
      local params = {
         penetration = 1,
      }
      spfx.debris( p:mass(), mem.r, p:pos(), p:vel() )
      explosion( p:pos(), p:vel(), r, d, tmerge( params, exp_params(r) ) )
      p:cargoJet("all")
   end

   if mem.dtimer < 0 then
      p:rm()
   end
end
