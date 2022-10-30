--[[

   Extra functionality for the VN

--]]
local vn = require "vn"
local love_shaders = require "love_shaders"
local lg = require 'love.graphics'

local extras = {}

-- Used to restore values
local textbox_bg_alpha, textbox_font, textbox_h, textbox_w, textbox_x, textbox_y

--[[--
Converts the VN to behave like a notebook with hand-written text.

   @tparam[opt=_("Notebook")] string Name to give the "noteboo" vn character.
   @treturn vn.Character The new character to use for the notebook.
--]]
function extras.notebookStart( name )
   name = name or _("Notebook")
   local nw, nh = naev.gfx.dim()
   local paperbg = love_shaders.paper( nw, nh )
   local oldify = love_shaders.oldify()
   vn.func( function ()
      vn.setBackground( function ()
         vn.setColor( {1, 1, 1, 1} )
         lg.rectangle("fill", 0, 0, nw, nh )
         vn.setColor( {1, 1, 1, 0.3} )
         lg.setShader( oldify )
         paperbg:draw( 0, 0 )
         lg.setShader()
      end )
      -- Store old values
      textbox_bg_alpha = vn.textbox_bg_alpha
      textbox_h = vn.textbox_h
      textbox_w = vn.textbox_w
      textbox_x = vn.textbox_x
      textbox_y = vn.textbox_y
      textbox_font = vn.textbox_font
      -- New values
      vn.textbox_font = lg.newFont( _("fonts/CoveredByYourGrace-Regular.ttf"), 24 )
      vn.textbox_bg_alpha = 0
      vn.textbox_h = math.min(0.7*nh, 800 )
      vn.textbox_y = (nh-vn.textbox_h)/2
      vn.textbox_w = math.min( 0.8*nw, 1200 )
      vn.textbox_x = (nw-vn.textbox_w)/2
      vn.show_options = false
   end )
   vn.scene()
   local log = vn.newCharacter( name, { color={0, 0, 0}, hidetitle=true } )
   vn.transition()
   return log
end

function extras.notebookEnd ()
   vn.scene()
   vn.func( function ()
      vn.setBackground()
      vn.textbox_bg_alpha = textbox_bg_alpha
      vn.textbox_h = textbox_h
      vn.textbox_w = textbox_w
      vn.textbox_x = textbox_x
      vn.textbox_y = textbox_y
      vn.textbox_font = textbox_font
      vn.show_options = true
   end )
   vn.transition()
end

return extras
