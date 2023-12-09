local lg = require 'love.graphics'
local lf = require 'love.filesystem'
local audio = require 'love.audio'
local love_shaders = require 'love_shaders'

local alert_shader, alert_sound

local function update( s, dt )
   local d = s:data()
   d.timer = d.timer + dt
end

local function render( sp, x, y, z )
   local d = sp:data()
   alert_shader:send( "u_time", d.timer )
   alert_shader:send( "u_size", d.size )

   local s = d.size * z
   local old_shader = lg.getShader()
   lg.setShader( alert_shader )
   lg.setColour( d.col )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end

local function alert( pos, params )
   params = params or {}
   -- Lazy loading shader / sound
   if not alert_sound then
      alert_sound = audio.newSource('snd/sounds/alarm_warning.ogg')
   end
   if not alert_shader then
      local alert_bg_shader_frag = lf.read( "scripts/luaspfx/shaders/alert.frag" )
      alert_shader = lg.newShader( alert_bg_shader_frag )
   end

   local size = params.size or 100
   local s = spfx.new( 2.2, update, nil, nil, render, pos, nil, alert_sound, size*0.5 )
   local d  = s:data()
   d.timer  = 0
   d.size   = size
   d.col    = params.col or {1, 1, 0, 0.5}
   return s
end

return alert
