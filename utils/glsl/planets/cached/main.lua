local pixelcode_noise = love.filesystem.read( "noise.glsl" )

local vertexcode = [[
#pragma language glsl3
vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]

local planet_types
local global_dt, idata, img, shader, shader_name, csv_diffuse, csv_height, csv_normal

local function set_shader( num )
   local s = planet_types[num+1]
   if not s then
      return
   end
   local w, h = love.graphics.getDimensions()
   local shader_type = s.height
   shader_name = s.name

   -- Height map.
   csv_height = love.graphics.newCanvas( 2*w, h, {dpiscale=1} )
   love.graphics.setCanvas( csv_height )
   shader = love.graphics.newShader( pixelcode_noise .. love.filesystem.read(shader_type), vertexcode)
   shader:send( "dimensions", {w,h} )
   if shader:hasUniform("u_seed") then
      shader:send( "u_seed", math.floor(1e3*global_dt) )
   end
   if shader:hasUniform("u_resolution") then
      shader:send( "u_resolution", {2*w, h} )
   end
   love.graphics.setShader( shader )
   love.graphics.draw( img, 0, 0, 0, 2*w, h )
   love.graphics.setShader()
   love.graphics.setCanvas()

   -- Normal map. Sends the height map.
   shader_type = s.normal
   if shader_type then
      csv_normal = love.graphics.newCanvas( 2*w, h, {dpiscale=1} )
      love.graphics.setCanvas( csv_normal )
      shader = love.graphics.newShader( pixelcode_noise .. love.filesystem.read(shader_type), vertexcode)
      shader:send( "dimensions", {w,h} )
      if shader:hasUniform("u_seed") then
         shader:send( "u_seed", math.floor(1e3*global_dt) )
      end
      if shader:hasUniform("u_resolution") then
         shader:send( "u_resolution", {2*w, h} )
      end
      if shader:hasUniform("height") then
         shader:send( "height", csv_height )
      end
      love.graphics.setShader( shader )
      love.graphics.draw( img, 0, 0, 0, 2*w, h )
      love.graphics.setShader()
      love.graphics.setCanvas()
   end

   -- Diffuse texture. Sends the height map and normal map if necessary.
   shader_type = s.diffuse
   if shader_type then
      csv_diffuse = love.graphics.newCanvas( 2*w, h, {dpiscale=1} )
      love.graphics.setCanvas( csv_diffuse )
      shader = love.graphics.newShader( pixelcode_noise .. love.filesystem.read(shader_type), vertexcode)
      shader:send( "dimensions", {w,h} )
      if shader:hasUniform("u_seed") then
         shader:send( "u_seed", math.floor(1e3*global_dt) )
      end
      if shader:hasUniform("u_resolution") then
         shader:send( "u_resolution", {2*w, h} )
      end
      if shader:hasUniform("height") then
         shader:send( "height", csv_height )
      end
      if shader:hasUniform("normal") then
         shader:send( "normal", csv_normal )
      end
      love.graphics.setShader( shader )
      love.graphics.draw( img, 0, 0, 0, 2*w, h )
      love.graphics.setShader()
      love.graphics.setCanvas()
   end

   -- Load Main shader.
   shader_type = s.main
   shader = love.graphics.newShader( pixelcode_noise .. love.filesystem.read(shader_type), vertexcode)
   if shader:hasUniform("u_seed") then
      shader:send( "u_seed", math.floor(1e3*global_dt) )
   end

   global_dt = 0
end

function love.load()
   local w = 600
   local h = 600
   local prog_name = "Naev Planet Demo"
   love.window.setTitle( prog_name )
   love.filesystem.setIdentity( prog_name );
   love.window.setMode( 2*w, h, {resizable = true} )
   global_dt = 0
   -- We need an image for the shader to work so we create a 1x1 px white image.
   idata = love.image.newImageData( 1, 1 )
   idata:setPixel( 0, 0, 1, 1, 1, 1 )
   img = love.graphics.newImage( idata )

   -- Define shader data.
   planet_types = {
      {
         name = "destroyed city",
         main = "destroyed_city.frag",
         height = "destroyed_city_height.frag",
         normal = "normal_from_height.frag",
         diffuse = "destroyed_city_diffuse.frag",
      },
      {
         name = "burned",
         main = "burned.frag",
         height = "burned_height.frag",
         normal = "normal_from_height.frag",
         diffuse = "burned_diffuse.frag",
      },
      {
         name = "craters",
         main = "craters.frag",
         height = "craters_height.frag",
         normal = "normal_from_height.frag",
         diffuse = "craters_diffuse.frag",
      },
      {
         name = "frozen",
         main = "frozen.frag",
         height = "frozen_height.frag",
         normal = "normal_from_height.frag",
         diffuse = "frozen_diffuse.frag",
      },
      {
         name = "fractured",
         main = "fractured.frag",
         height = "fractured_height.frag",
         normal = "normal_from_height.frag",
         diffuse = "fractured_diffuse.frag",
      },
   }

   -- Set up the shader
   set_shader( 0 )
   -- Set the font
   love.graphics.setNewFont( 24 )
end

function love.keypressed(key)
   local num = tonumber(key)
   if num~=nil then
      set_shader( num )
   elseif key=="q" or key=="escape" then
      love.event.quit()
   elseif key=="s" then
      local scr_name = ( "Screenshot_" .. os.time() .. ".png" )
      love.graphics.captureScreenshot( scr_name )
      print( "Captured screenshot " .. scr_name )
   end
end

function love.draw ()
   local lg = love.graphics
   local w, h = love.graphics.getDimensions()

   lg.setCanvas( )
   lg.setColor( 0, 0, 1, 1 )
   lg.setShader(shader)
   lg.draw( img, 0, 0, 0, w, h )
   lg.setShader()

   if global_dt < 1 then
      lg.setColor( 1, 1, 1, 1-global_dt )
      lg.printf( shader_name, 0, h/2-12, w, "center" )
      shader:send( "dimensions", {w,h} )
      if shader:hasUniform("diffuse") then
         shader:send( "diffuse", csv_diffuse )
      end
      if shader:hasUniform("height") then
         shader:send( "height", csv_height )
      end
      if shader:hasUniform("normal") then
         shader:send( "normal", csv_normal )
      end
   end
end

function love.update( dt )
   global_dt = (global_dt or 0) + dt
   if shader:hasUniform("u_time") then
      shader:send( "u_time", global_dt )
   end
end
