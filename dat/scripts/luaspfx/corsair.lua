local lg = require 'love.graphics'
local lf = require 'love.filesystem'

local corsair_shader
local TTL = 1
local GROWTH = 0.15

local function update( s, dt )
   local d = s:data()
   d.timer = d.timer + dt / TTL
end

local function render( sp, x, y, z )
   local d = sp:data()
   local c = d.canvas

   corsair_shader:send( "u_progress", 1-d.timer )

   -- Get slightly bigger over time
   local s = z * (1+GROWTH*d.timer)

   lg.setColour( 1, 1, 1 )
   local old_shader = lg.getShader()
   lg.setShader( corsair_shader )
   -- We have to flip the y axis
   c:draw( x-c.w*s*0.5, y+c.h*s*0.5, 0, s, -s )
   lg.setShader( old_shader )
end

local function corsair( p, pos, vel )
   if not corsair_shader then
      local corsair_shader_frag = lf.read( "scripts/luaspfx/shaders/corsair.frag" )
      corsair_shader = lg.newShader( corsair_shader_frag )
   end

   local c = lg.newCanvas( p:render() )
   local s = spfx.new( TTL, update, render, nil, nil, pos, vel, nil, (c.w+c.h)*0.25 )
   local d  = s:data()
   d.canvas = c
   d.timer = 0
   return s
end

return corsair
