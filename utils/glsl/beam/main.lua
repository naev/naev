--[[
-- Run with `love trail`
--]]


local pixelcode = love.filesystem.read( "beam.frag" )

local vertexcode = [[
#pragma language glsl3

vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]

function love.load()
   love.window.setTitle( "Naev Beam Demo" )
   -- Set up the shader
   shader   = love.graphics.newShader(pixelcode, vertexcode)
   -- We need an image for the shader to work so we create a 1x1 px white image.
   local idata = love.image.newImageData( 1, 1 )
   idata:setPixel( 0, 0, 1, 1, 1, 1 )
   img      = love.graphics.newImage( idata )
   -- Set the font
   love.graphics.setNewFont( 12 )
   -- Scaling
   love.window.setMode( 800, 1000 )
   scaling = 2

   -- Beams
   beamtypes = {
      {
         name = "beam_default",
         h = 14,
         colour = { 1, 1, 0, 1 },
         type = 0,
      },
      {
         name = "beam_unstable",
         h = 16,
         colour = { 1, 0.5, 0, 1 },
         type = 5,
      },
      {
         name = "beam_fuzzy",
         h = 14,
         colour = { 1, 0.2, 0, 1 },
         type = 6,
      },
      {
         name = "beam_wave",
         h = 14,
         colour = { 1, 0, 0, 1 },
         type = 1,
      },
      {
         name = "beam_arc",
         h = 16,
         colour = { 0.2, 0.5, 0.9, 1 },
         type = 2,
      },
      {
         name = "beam_helix",
         h = 16,
         colour = { 0.2, 0.9, 0.5, 1 },
         type = 3,
      },
      {
         name = "beam_organic",
         h = 16,
         colour = { 0.6, 0.6, 0.95, 1 },
         type = 4,
      },
      {
         name = "beam_reverse",
         h = 16,
         colour = { 1, 0, 1, 1 },
         type = 7,
      },
   }
   beamoutfits = {
      {
         name = "Orion Beam",
         h = 16,
         colour = { 0.2, 0.6, 0.9, 1 },
         type = 1,
      },
      {
         name = "Orion Lance",
         h = 13,
         colour = { 0.2, 0.6, 0.9, 1 },
         type = 6,
      },
      {
         name = "Particle Beam",
         h = 12,
         colour = { 1.0, 0.2, 0.0, 1 },
         type = 6,
      },
      {
         name = "Particle Lance",
         h = 10,
         colour = {1.0, 0.35, 0.0, 1 },
         type = 0,
      },
      {
         name = "Pulse Beam",
         h = 14,
         colour = { 0.6, 0.9, 0.2, 1 },
         type = 5,
      },
      {
         name = "Ragnarok Beam",
         h = 22,
         colour = { 0.8, 0.1, 0.4, 1 },
         type = 3,
      },
      {
         name = "Grave Beam",
         h = 20,
         colour = { 1.0, 0.3, 0.6, 1 },
         type = 3,
      },
      {
         name = "Grave Lance",
         h = 20,
         colour = { 0.9, 0.2, 0.5, 1 },
         type = 6,
      },
      {
         name = "Shattershield Lance",
         h = 14,
         colour = { 0.2, 0.5, 0.9, 1 },
         type = 2,
      },
      {
         name = "Valkyrie Beam",
         h = 26,
         colour = { 1.0, 0.6, 0.9, 1 },
         type = 3,
      },
      {
         name = "Antimatter Beam",
         h = 16,
         colour = { 1.0, 0.0, 1.0, 1 },
         type = 7,
      },
   }
   beams = beamtypes
   love.graphics.setBackgroundColor( 0.2, 0.2, 0.2, 1.0 )
end

function love.keypressed(key)
   if key=="1" then
      beams = beamtypes
   elseif key=="2" then
      beams = beamoutfits
   elseif key=="q" or key=="escape" then
      love.event.quit()
   end
end

function love.draw ()
   local x = 20
   local y = 20
   local lg = love.graphics
   local s = scaling
   love.graphics.scale( s, s )
   y = y/s
   lg.setShader()
   lg.setColor( 1, 1, 1, 1 )
   lg.print( "Press 1 to show types, press 2 to show outfits, q to quit.", x, y )
   y = y + 20

   local w = 350
   for k,b in ipairs(beams) do
      lg.setShader()
      lg.setColor( 1, 1, 1, 1 )
      love.graphics.print(b.name, x, y )
      y = y + 17
      local h = b.h
      lg.setColor( 1, 1, 1, 0.5 )
      lg.rectangle( "line", x-2, y-2, w+4, h+4 )
      lg.setShader(shader)
      shader:send( "type", b.type )
      shader:send( "r", 0 )
      shader:send( "dimensions", {w,h} )
      lg.setColor( b.colour )
      lg.draw( img, x, y, 0, w, h)
      y = y + h + 5
   end
end

function love.update( dt )
   global_dt = (global_dt or 0) + dt/1.5
   if shader:hasUniform("dt") then
      shader:send( "dt", global_dt )
   end
end
