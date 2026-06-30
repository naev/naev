local lg = require 'love.graphics'
local lf = require 'love.filesystem'
local love_shaders = require 'love_shaders'
local explosion = require 'luaspfx.explosion'

local agamemnon_shader

local DURATION = 2
local SIZE_SHADER = 150 -- Size of the shader
local SIZE     = 300
local DAMAGE   = 1000

local function update( sp, dt )
   local d = sp:data()
   d.timer = d.timer + dt
end

local function render( sp, x, y, z )
   local d = sp:data()
   --flames_shader:send( "u_time",  d.timer )
   --flames_shader:send( "u_r", d.r )

   local s = SIZE_SHADER * z
   local old_shader = lg.getShader()
   --lg.setShader( agamemnon_shader )
   lg.setShader( nil )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end

local function remove( sp )
   local d = sp:data()
   explosion( sp:pos(), sp:vel(), SIZE, DAMAGE, {
      colourbase  = {1.1, 0.5, 1.0, 0.7}, -- More purpleish
      parent      = d.parent,
   } )
end

local function nuke( parent, pos, vel )
   -- Lazy loading
   if not agamemnon_shader then
      local agamemnon_shader_frag = lf.read( "shaders/cleansing_flames.frag" )
      agamemnon_shader = lg.newShader( agamemnon_shader_frag )
      -- TODO sfx?
   end

   -- Create the spfx
   local s  = spfx.new( DURATION, update, render, nil, nil, pos, vel, nil, SIZE_SHADER*0.5, remove, {
      parent = parent,
      timer  = 0,
      r      = rnd.rnd(),
   } )

   return s
end

return nuke
