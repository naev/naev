local luaspob = require "spob.lua.lib.spob"
local lg = require "love.graphics"
local lf = require "love.filesystem"
local li = require "love.image"
local love_shaders = require "love_shaders"

local pixelcode = lf.read( "spob/lua/shaders/goo.frag" )

local function update_canvas ()
   local oldcanvas = lg.getCanvas()
   lg.setCanvas( mem.cvs )
   lg.clear( 0, 0, 0, 0 )
   lg.setColour( 1, 1, 1, 1 )

   -- Draw the shader
   lg.setShader( mem.shader )
   lg.draw( mem.img, 0, 0, 0, mem.tw, mem.th )
   lg.setShader()

   lg.setCanvas( oldcanvas )
end

function init( spb )
   mem.spob = spb
end

function load( )
   -- We need an image for the shader to work.
   local idata = li.newImageData( 1, 1 )
   idata:setPixel( 0, 0, 1, 1, 1, 1 )
   mem.img = lg.newImage( idata )

   -- Position stuff
   mem.pos = mem.spob:pos()
   local size = 500
   mem.tw = size
   mem.th = size
   mem.radius = size*0.5

   -- The canvas
   mem.cvs  = lg.newCanvas( mem.tw, mem.th, {dpiscale=1} )

   -- Set up shader
   mem.shader = lg.newShader( pixelcode, love_shaders.vertexcode )
   mem.shader._dt = -1000 * rnd.rnd()
   mem.shader.update = function( self, dt )
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end

   update_canvas()
   return mem.cvs.t.tex, mem.radius
end

function unload ()
   mem.shader= nil
   mem.img   = nil
   mem.cvs   = nil
end

function can_land ()
   return true, _("Go ahead.")
end

function render ()
   update_canvas() -- We want to do this here or it gets slow in autonav
   local z = camera.getZoom()
   local x, y = gfx.screencoords( mem.pos ):get()
   z = 1/z
   mem.cvs:draw( x-mem.radius, y-mem.radius, 0, z, z )
end

function update( dt )
   mem.shader:update( dt )
end

comm = luaspob.comm
