local lg = require 'love.graphics'
local lf = require 'love.filesystem'
local love_shaders = require 'love_shaders'

local track_shader, buoy_gfx, buoy_w, buoy_h

local function update( s, _dt )
   local d = s:data()
   if d.ready then
      local p = player.pos()
      local ret = vec2.collideLineLine( d.ppos, p, d.seg1, d.seg2 )
      if ret==1 then
         d.ready = false
         d.activate()
      end
      d.ppos = p
   end
end

local function render_buoy( v, z, w, h )
   local x, y = gfx.screencoords( v, true ):get()
   buoy_gfx:draw( x-w*0.5, y-h*0.5, 0, z )
   buoy_gfx:draw( x-w*0.5, y-h*0.5, 0, z )
end

local function render( sp, x, y, z )
   local d = sp:data()

   if d.col then
      local sw = d.size * 0.2 * z
      local sh = d.size * z
      local old_shader = lg.getShader()
      track_shader:send( "u_dimensions", sw, sh );
      lg.setShader( track_shader )
      lg.setColor( d.col )
      love_shaders.img:draw( x-sw*0.5, y-sh*0.5, d.rot, sw, sh )
      lg.setShader( old_shader )
   end

   lg.setColor{1,1,1}
   local w, h = buoy_w*z, buoy_h*z
   render_buoy( d.seg1, z, w, h )
   render_buoy( d.seg2, z, w, h )
end

local racetrack = {}
local racetrack_mt = { __index=racetrack }

local function racetrack_new( pos, rot, activate, params )
   params = params or {}
   -- Lazy loading shader
   if not track_shader then
      local track_bg_shader_frag = lf.read( "scripts/luaspfx/shaders/track.frag" )
      track_shader = lg.newShader( track_bg_shader_frag )

      buoy_gfx = lg.newImage( "gfx/spob/space/jumpbuoy.webp" )
      buoy_w, buoy_h = buoy_gfx:getDimensions()
   end

   local size = params.size or 500

   local s = spfx.new( math.huge, update, render, nil, nil, pos, nil, nil, size )
   local d  = s:data()
   d.size   = size
   d.col    = params.col or nil
   d.rot    = rot
   d.ready  = false
   d.seg1   = pos-vec2.newP(size*0.5,math.pi/2-rot)
   d.seg2   = pos+vec2.newP(size*0.5,math.pi/2-rot)
   d.ppos   = player.pos()
   d.activate = activate

   local obj = { s=s }
   setmetatable( obj, racetrack_mt )

   return obj
end

function racetrack:rm()
   self.s:rm()
end

function racetrack:data()
   return self.s:data()
end

function racetrack:setReady( state )
   local d = self:data()
   d.ready = state
   d.ppos = player.pos()
end

function racetrack:setCol( col )
   local d = self:data()
   d.col = col
end

return racetrack_new
