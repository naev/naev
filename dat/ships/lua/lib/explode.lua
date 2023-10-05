local explosion = require "luaspfx.explosion"

local function setup( params )
   params = params or {}
   params.dtimer_mod = params.dtimer_mod or 1
   params.boom_size_mod = params.boom_size_mod or 1
   params.exp_size_mod = params.exp_size_mod or 1

   local function explode_init( p )
      local s = p:stats()
      local armour_max, shield_max = s.armour, s.shield
      local sw, sh = p:ship():dims()

      mem.dtimer = params.dtimer_mod * math.min( 1 + math.sqrt( armour_max * shield_max ) / 650, 3 + math.pow( armour_max * shield_max, 0.4) / 500 )
      mem.exploded = false
      mem.timer = 0
      mem.r = (sw+sh)*0.25 -- Radius
   end

   local function boom_default( p )
      mem.timer = 0.08 * (mem.dtimer - mem.timer) / mem.dtimer
      local pos = p:pos() + vec2.newP( mem.r*rnd.rnd(), rnd.angle() )
      local r = mem.r * (0.2 + 0.3 * rnd.rnd()) * params.boom_size_mod

      -- TODO maybe explosions should be under player when applicable...
      local prms = { silent=true }
      if params.boom_params then
         prms = tmerge( prms, params.boom_params( r ) )
      end
      explosion( pos, p:vel(), r, nil, prms )
   end

   local function explode_default( p )
      local a = math.sqrt( p:mass() )
      local r = mem.r * 1.2 * params.exp_size_mod + a
      local d = math.max( 0, 2 * (a * (1+math.sqrt(p:fuel()+1)/28)))
      local prms = {
         penetration = 1,
         dmgtype = params.dmgtype or "normal",
         disable = params.disable or 0,
      }
      if params.exp_params then
         prms = tmerge( prms, params.exp_params( r ) )
      end
      spfx.debris( p:mass(), mem.r, p:pos(), p:vel() )
      explosion( p:pos(), p:vel(), r, d, prms )
      p:cargoJet("all")
   end

   local boom_func      = params.boom_func or boom_default
   local explode_func   = params.exp_func or explode_default

   local function explode_update( p, dt )
      mem.timer = mem.timer - dt
      mem.dtimer = mem.dtimer - dt

      if not mem.exploded then
         if mem.timer < 0 then
            boom_func( p )
         end

         if not mem.exploded and mem.dtimer < 0.2 then
            mem.exploded = true
            explode_func( p )
         end
      end

      if mem.dtimer < 0 then
         p:explode()
      end
   end

   return explode_init, explode_update
end

return setup
