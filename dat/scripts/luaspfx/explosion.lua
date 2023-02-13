local lg = require 'love.graphics'
local lf = require 'love.filesystem'
--local audio = require 'love.audio'
local love_shaders = require 'love_shaders'

local explosion_shader, explosion_sfx

local function do_damage( pos, radius, damage, penetration, parent )
   local pp = player.pilot()
   for k,p in ipairs(pilot.getInrange( pos, radius )) do
      local norm, angle = (p:pos() - pos):polar()
      local mod = 1 - norm / radius
      local dmg = damage*(0.5 + 0.5*mod)
      local mass = math.pow( dmg / 15, 2 )

      -- Damage and knockback
      p:damage( dmg, 0, penetration, "normal", parent )
      p:knockback( mass, vec2.newP( mod*radius, angle ), pos, 1 )

      -- Shake the screen for the player
      if p == pp then
         camera.shake( mass / p:mass() )
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
   explosion_shader:send( "u_smokiness", d.smokiness )
   explosion_shader:send( "u_colorbase", d.colorbase )
   explosion_shader:send( "u_colorsmoke", d.colorsmoke )
   explosion_shader:send( "u_smoke_fade", d.smokefade )
   explosion_shader:send( "u_roll_speed", d.rollspeed )

   local s = d.size * z
   local old_shader = lg.getShader()
   lg.setShader( explosion_shader )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end

local function spfx_explosion( pos, vel, size, params )
   size = size * 2 -- Explosions look much smaller in reality, so we double
   local speed = params.speed or math.max( -0.000940296 * size + 0.719132, 0.2 )
   local sfx
   if not params.silent then
      sfx = explosion_sfx[ rnd.rnd(1,#explosion_sfx) ]
   end
   local s  = spfx.new( 1/speed, update, nil, render, nil, pos, vel, sfx, size*0.5 )
   local d  = s:data()
   d.timer  = 0
   d.size   = size
   d.grain  = params.grain or (0.0016265 * size + 0.0944304)
   d.speed  = speed
   d.steps  = params.steps or math.min( math.floor(0.0111688 * size + 8.16463 + 0.5), 16 )
   d.smokiness = params.smokiness or 0.588
   d.colorbase = params.colorbase or {1.2, 0.9, 0.5, 0.7}
   d.colorsmoke = params.colorsmoke or {0.15, 0.15, 0.15, 0.1}
   d.smokefade = params.smokefade or 1.4
   d.rollspeed = params.rollspeed or 1.0
   if params.volume then
      local ss = s:sfx()
      ss:setVolume( params.volume )
   end
end

local function explosion( pos, vel, radius, damage, params )
   params = params or {}

   -- Lazy loading shader / sound
   if not explosion_shader then
      local explosion_shader_frag = lf.read( "glsl/explosion.frag" )
      explosion_shader = lg.newShader( explosion_shader_frag )
      explosion_sfx = {
         audio.new( "snd/sounds/explosion0.wav" ),
         audio.new( "snd/sounds/explosion1.wav" ),
         audio.new( "snd/sounds/explosion2.wav" ),
      }
   end

   -- Do damage if applicable
   if damage then
      do_damage( pos, radius, damage, params.penetration, params.parent )
   end

   -- Create the explosions
   spfx_explosion( pos, vel, radius, params )
end

return explosion
