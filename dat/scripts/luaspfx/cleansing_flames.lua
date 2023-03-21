local lg = require 'love.graphics'
local lf = require 'love.filesystem'
--local audio = require 'love.audio'
local love_shaders = require 'love_shaders'

local flames_shader, flames_sfx

local function update( s, dt )
   local d = s:data()
   d.timer = d.timer + dt
end

local function render( sp, x, y, z )
   local d = sp:data()
   flames_shader:send( "u_time",  d.timer )
   flames_shader:send( "u_speed",  d.speed )
   flames_shader:send( "u_grain",  d.grain )
   flames_shader:send( "u_r", d.r )

   local s = d.size * z
   local old_shader = lg.getShader()
   lg.setShader( flames_shader )
   love_shaders.img:draw( x-s*0.5, y-s*0.5, 0, s )
   lg.setShader( old_shader )
end

local function flames( pos, vel, radius, params )
   params = params or {}

   -- Lazy loading shader / sound
   if not flames_shader then
      local flames_shader_frag = lf.read( "glsl/cleansing_flames.frag" )
      flames_shader = lg.newShader( flames_shader_frag )
      flames_sfx = {
         -- TODO sound
         audio.new( "snd/sounds/empexplode.ogg" ),
      }
   end

   -- Create the flames
   local size = radius * 1.5 -- Flames look a bit smaller in reality, so we increase in size
   local speed = params.speed or math.max(1.5-(size/350)^0.5, 0.4)
   local sfx
   if not params.silent then
      sfx = flames_sfx[ rnd.rnd(1,#flames_sfx) ]
   end
   local s  = spfx.new( 1/speed, update, nil, nil, render, pos, vel, sfx, size*0.5 )
   local d  = s:data()
   d.timer  = 0
   d.size   = size
   d.speed  = speed
   d.grain  = params.grain or (size/30)^0.5
   d.r      = rnd.rnd()
   if params.volume then
      local ss = s:sfx()
      ss:setVolume( params.volume )
   end

   if params.parent then
      -- Apply effect-
      for k,p in ipairs(params.parent:getEnemies( radius, pos, true, false, true )) do
         -- TODO change to effect
         p:effectAdd("Chakra Corruption")
      end
      -- Clear debuffs
      for k,p in ipairs(params.parent:getAllies( radius, pos, true, false, true )) do
         p:effectClear( false, true, true )
      end
   end

   return s
end

return flames
