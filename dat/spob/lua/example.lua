--[[
   Example of a script for a Space Object. All functions are optional and can
   be used to override different core functionality.
--]]
local lg = require "love.graphics"
local tex, pos, tw, th

--[[
   @brief Run when system is getting loaded. Should return a texture that will
   be used by the GUI to visualize and determine the radius (if not
   overwritten).

      @luatreturn Texture A texture to be used as the planet image.
      @luatreturn[opt] number The value to use for the radius of the planet (for targetting, etc...)
--]]
function load( s )
   if tex==nil then
      tex = lg.newImage( "path/to/image.png" )
      pos = s:pos()
      tw, th = tex:getDimensions()
      pos = pos + vec2.new( -tw/2, th/2 )
   end
   return tex.tex, (tw+th)/4
end

--[[
   @brief Run when the system is left and the planet stuff is not needed
   anymore. Should free all textures and things that aren't necessary anymore.
--]]
function unload ()
   tex = nil
end

--[[
   @brief Checks to see if the player can land or not.

      @luatreturn boolean Whether or not the player can land.
      @luatreturn string Message to give the player regarding being able to land or not.
--]]
function can_land ()
   return true, _("Go ahead.")
end

--[[
   @brief Triggers when a pilot lands (assuming they can from can_land()).

      @luatparam Pilot p Pilot that is landing.
--]]
function land( _p )
   -- Do something
end

--[[
   @brief Will be used to render the planet instead of the graphic provided by
   load().
--]]
function render ()
   local z = camera.getZoom()
   local x, y = gfx.screencoords( pos, true ):get()
   tex:draw( x, y, 0, 1/z )
end

--[[
   @brief Runs updates.

      @luatparam number dt Time increment since last update.
      @luatparam number real_dt Real time increment (in real world seconds).
--]]
function update( _dt, _real_dt )
end
