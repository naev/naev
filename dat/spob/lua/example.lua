--[[
   Example of a script for a Space Object. All functions are optional and can
   be used to override different core functionality.
--]]
local lg = require "love.graphics"

--[[
   @brief Run when spob is initialized during loading.

      @luatparam spb Spob being initialized.
--]]
function init( spb )
   mem.spob = spb
end

--[[
   @brief Run when system is getting loaded. Should return a texture that will
   be used by the GUI to visualize and determine the radius (if not
   overwritten).

      @luatreturn Texture A texture to be used as the planet image.
      @luatreturn[opt] number The value to use for the radius of the planet (for targetting, etc...)
--]]
function load ()
   if tex==nil then
      mem.tex = lg.newImage( "path/to/image.png" )
      mem.pos = mem.spob:pos()
      mem.tw, mem.th = tex:dim()
      mem.pos = mem.pos + vec2.new( -mem.tw/2, mem.th/2 )
   end
   return mem.tex.tex, (mem.tw+mem.th)/4
end

--[[
   @brief Run when the system is left and the planet stuff is not needed
   anymore. Should free all textures and things that aren't necessary anymore.
--]]
function unload ()
   mem.tex = nil -- Will be freed by garbage collector
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
   local x, y = gfx.screencoords( mem.pos, true ):get()
   mem.tex:draw( x, y, 0, 1/z )
end

--[[
   @brief Runs updates.

      @luatparam number dt Time increment since last update.
      @luatparam number real_dt Real time increment (in real world seconds).
--]]
function update( _dt, _real_dt )
end

--[[
   @brief Runs when hailed by the player.
--]]
function comm ()
end
