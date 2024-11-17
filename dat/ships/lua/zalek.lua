local explosion = require "luaspfx.explosion"
local emp = require "luaspfx.emp"

local explib = require "ships.lua.lib.explode"

explode_init, explode_update = explib{
   disable = 0.4, -- Significant disable damage
   boom_func = function( p )
      mem.timer = 0.05 * (mem.dtimer - mem.timer) / mem.dtimer

      local pos = p:pos() + vec2.newP( mem.r*rnd.rnd(), rnd.angle() )

      if rnd.rnd() < 0.3 then
         local r = mem.r * (0.2 + 0.3 * rnd.rnd())
         explosion( pos, p:vel(), r, nil, {silent=true} )
      else
         local r = math.max( 10, mem.r * (0.1 + 0.1 * rnd.rnd()) )
         emp( pos, p:vel(), r, nil, {silent=true} )
      end
   end,
}
