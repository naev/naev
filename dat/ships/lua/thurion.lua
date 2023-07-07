local explosion = require "luaspfx.explosion"
local emp = require "luaspfx.emp"
local spark = require "luaspfx.spark"

local explib = require "ships.lua.lib.explode"

explode_init, explode_update = explib{
   boom_func = function( p )
      mem.timer = 0.04 * (mem.dtimer - mem.timer) / mem.dtimer

      local pos = p:pos() + vec2.newP( mem.r*rnd.rnd(), rnd.angle() )

      local rn = rnd.rnd()
      if rn < 0.5 then
         local r = math.max( 15, mem.r * (0.2 + 0.1 * rnd.rnd()) )
         spark( pos, p:vel(), r, nil, {silent=true} )
      elseif rn < 0.6 then
         local r = mem.r * (0.2 + 0.3 * rnd.rnd())
         explosion( pos, p:vel(), r, nil, {silent=true} )
      else
         local r = math.max( 10, mem.r * (0.1 + 0.1 * rnd.rnd()) )
         emp( pos, p:vel(), r, nil, {silent=true} )
      end
   end,
   exp_func = function( p )
      local a = math.sqrt( p:mass() )
      local r = mem.r * 1.2 + a
      local d = math.max( 0, 2 * (a * (1+math.sqrt(p:fuel()+1)/28)))
      local prms = {
         penetration = 1,
         disable = 0.7, -- Mainly disabling damage
         dmgtype = "ion",
      }
      spfx.debris( p:mass(), mem.r, p:pos(), p:vel() )
      emp( p:pos(), p:vel(), r*1.2 )
      explosion( p:pos(), p:vel(), r*0.8, d, prms )
      p:cargoJet("all")
   end,
}
