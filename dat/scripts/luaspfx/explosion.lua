local lg = require 'love.graphics'
local lf = require 'love.filesystem'
--local audio = require 'love.audio'
local love_shaders = require 'love_shaders'

local explosion_shader_frag = lf.read( "glsl/explosion.frag" )
local explosion_shader, explosion_sfx

local function do_damage( pos, radius, damage, parent )
   local pp = player.pilot()
   for k,p in ipairs(pilot.getInrange( pos, radius )) do
      local norm, angle = (p:pos() - pos):polar()
      local mod = 1 - norm / radius
      local mass = math.pow( damage / 30, 2 )

      -- Damage and knockback
      p:damage( damage, 0, 50, "normal", parent )
      p:knockback( mass, vec2.new( mod*radius, angle ), nil, 0.5 )

      -- Shake the screen for the player
      if p == pp then
         camera.shake( mass * 0.09 )
      end
   end
end

local function update( s, dt )
   local d = s:data()
   d.timer = d.timer + dt
end

local function render( sp, x, y, z )
   local d = sp:data()
   explosion_shader:send( "u_time",  d.timer )
   explosion_shader:send( "u_grain", d.grain )
   explosion_shader:send( "u_speed", d.speed )
   explosion_shader:send( "u_steps", d.steps )

   local s = d.size * z
   local old_shader = lg.getShader()
   lg.setShader( explosion_shader )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end

local function spfx_explosion( pos, vel, size )
   local length = 0.00486483 * size + 1.26438
   local sfx = explosion_sfx[ rnd.rnd(1,#explosion_sfx) ]
   local s  = spfx.new( length, update, nil, nil, render, pos, vel, sfx )
   local d  = s:data()
   d.timer  = 0
   d.size   = size
   d.grain  = 0.0016265 * size + 0.0944304
   d.speed  = math.max( -0.000940296 * size + 0.719132, 0.2 )
   d.steps  = math.min( math.floor(0.0111688 * size + 8.16463 + 0.5), 16 )
end

local function explosion( pos, vel, radius, damage, parent, params )
   params = params or {}
   -- Lazy loading shader / sound
   if not explosion_shader then
      explosion_shader = lg.newShader( explosion_shader_frag )
      explosion_sfx = {
         audio.new( "snd/sounds/explosion0.wav" ),
         audio.new( "snd/sounds/explosion1.wav" ),
         audio.new( "snd/sounds/explosion2.wav" ),
      }
   end

   -- Do damage if applicable
   if damage then
      do_damage( pos, radius, damage, parent )
   end

   -- Create the explosions
   spfx_explosion( pos, vel, radius )
end

return explosion
