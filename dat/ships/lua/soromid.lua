local explosion = require "luaspfx.explosion"

local explib = require "ships.lua.lib.explode"

local function exp_params( size )
   return {
      colourbase = {0.1, 0.9, 0.1, 0.2},
      smokiness = 0.4,
      coloursmoke = {0.6, 0.7, 0.1, 0.5},
      smokefade = 1.6,
      rollspeed = 0.3,
      grain = 0.1 + size*0.0015,
   }
end

explode_init, explode_update = explib{
   dtimer_mod = 1.3, -- slower explosions
   boom_func = function( p )
      mem.timer = 0.08 * (mem.dtimer - mem.timer) / mem.dtimer

      local pos = p:pos() + vec2.newP( mem.r*rnd.rnd(), rnd.angle() )
      local r = mem.r * (0.2 + 0.3 * rnd.rnd())

      -- TODO maybe explosions should be under player when applicable...
      if rnd.rnd() < 0.1 then
         explosion( pos, p:vel(), r, nil, {silent=true} )
      else
         explosion( pos, p:vel(), r*1.2, nil, tmerge( {silent=true}, exp_params(r) ) )
      end
   end,
   exp_func = function( p )
      local a = math.sqrt( p:mass() )
      local r = mem.r * 1.0 + a
      local d = math.max( 0, 2 * (a * (1+math.sqrt(p:fuel()+1)/28)))
      local params = {
         penetration = 1,
      }
      spfx.debris( p:mass(), mem.r, p:pos(), p:vel() )
      explosion( p:pos(), p:vel(), r, d, tmerge( params, exp_params(r) ) )
      p:cargoJet("all")
   end,
}
