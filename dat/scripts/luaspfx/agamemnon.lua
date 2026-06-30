local lg = require 'love.graphics'
local lf = require 'love.filesystem'
local love_shaders = require 'love_shaders'
local explosion = require 'luaspfx.explosion'

local agamemnon_shader

local DURATION = 2
local SIZE_SHADER = 50 -- Size of the shader
local SIZE     = 300
local DAMAGE   = 1000

local function explode( sp, d )
   explosion( sp:pos(), sp:vel(), SIZE, DAMAGE, {
      colourbase  = {1.1, 0.5, 1.0, 0.7}, -- More purpleish
      coloursmoke = {0.45, 0.15, 0.35, 0.1},
      parent      = d.parent,
      grain       = 0.4,
      rollspeed   = 0.5,
      smokefade   = 1.8,
   } )
   d.exploded = true
end

local function update( sp, dt )
   local d = sp:data()
   d.timer = d.timer + dt
   if d.timer > 1.95 and not d.exploded then
      explode( sp, d )
   end
end

local function render( sp, x, y, z )
   local d = sp:data()
   agamemnon_shader:send( "u_time",  d.timer )
   agamemnon_shader:send( "u_r", d.r )

   local s = SIZE_SHADER * z
   local old_shader = lg.getShader()
   lg.setShader( agamemnon_shader )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end

local function remove( sp )
   local d = sp:data()
   if not d.exploded then
      explode( sp, d )
   end
end

local function nuke( parent, pos, vel )
   -- Lazy loading
   if not agamemnon_shader then
      local agamemnon_shader_frag = lf.read( "scripts/luaspfx/shaders/agamemnon.frag" )
      agamemnon_shader = lg.newShader( agamemnon_shader_frag )
      -- TODO sfx?
   end

   -- Create the spfx
   local s  = spfx.new( DURATION, update, nil, nil, render, pos, vel, nil, SIZE_SHADER*0.5, remove, {
      parent = parent,
      timer  = 0,
      r      = rnd.rnd(),
   } )

   return s
end

return nuke
