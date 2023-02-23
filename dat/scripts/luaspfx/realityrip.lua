local lf = require 'love.filesystem'
--local audio = require 'love.audio'
local pp_shaders = require "pp_shaders"

local rip_bg_shader_frag

local function update( sp, dt )
   local d = sp:data()
   if d.timer < 1 then
      d.timer = d.timer + dt
      d.shader:send( "u_timer", d.timer )
   end

   -- Apply effect to all pilots in range
   for k,p in ipairs(pilot.getInrange( d.pos, d.size )) do
      p:effectAdd("Fractured Reality")
   end
end

local function render( sp, x, y, z )
   local d = sp:data()
   print( x, y, z )
   d.shader:send( "u_pos", x, y, z )
end

local function remove( sp )
   local d = sp:data()
   d.shader:rmPPShader()
end

local function realityrip( pos, size, params )
   params = params or {}
   -- Lazy loading shader / sound
   if not rip_bg_shader_frag then
      rip_bg_shader_frag = lf.read( "scripts/luaspfx/shaders/realityrip.frag" )
   end

   -- Sound is handled separately in outfit
   local s = spfx.new( math.huge, update, nil, nil, render, pos, params.vel, nil, size, remove )
   local d  = s:data()
   d.pos    = pos
   d.timer  = 0
   d.size   = size
   d.shader = pp_shaders.newShader( rip_bg_shader_frag ) -- Have to recreate each time
   d.shader:send( "u_size", size )
   d.shader:addPPShader("game", 20)
   return s
end

return realityrip
