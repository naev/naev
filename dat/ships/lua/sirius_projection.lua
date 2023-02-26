local chakra = require "luaspfx.chakra_explosion"

local explib = require "ships.lua.lib.explode"

explode_init, explode_update = explib{
   dtimer_mod = 0.3, -- Fast explosions
   boom_func = function( p )
      -- Fast explosions
      mem.timer = 0.05 * (mem.dtimer - mem.timer) / mem.dtimer
      local pos = p:pos() + vec2.newP( mem.r*rnd.rnd(), rnd.angle() )
      local r = mem.r * (0.4 + 0.2 * rnd.rnd())
      chakra( pos, p:vel(), r, nil, {silent=true} )
   end,
   exp_func = function( p )
      local a = math.sqrt( p:mass() )
      local r = mem.r * 1.2 + a
      spfx.debris( p:mass(), mem.r, p:pos(), p:vel() )
      chakra( p:pos(), p:vel(), r )
      --p:cargoJet("all") -- shouldn't have cargo
   end,
}
