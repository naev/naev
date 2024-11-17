--[[

   Extra functionality for the VN

--]]
local vn = require "vn"
local love_shaders = require "love_shaders"
local lg = require 'love.graphics'

local extras = {}

-- Used to restore values
local textbox_bg_alpha, textbox_font, textbox_h, textbox_w, textbox_x, textbox_y, characters

local function fullscreenStart( func, params )
   params = params or {}
   local nw, nh = naev.gfx.dim()
   vn.func( function ()
      -- Store old values
      characters = vn._characters
      textbox_bg_alpha = vn.textbox_bg_alpha
      textbox_h = vn.textbox_h
      textbox_w = vn.textbox_w
      textbox_x = vn.textbox_x
      textbox_y = vn.textbox_y
      textbox_font = vn.textbox_font
   end )
   vn.scene()
   vn.func( function ()
      if func then
         func()
      end
      -- New values
      if params.font then
         vn.textbox_font = params.font
      end
      vn.textbox_bg_alpha = 0
      vn.textbox_h = math.min(0.7*nh, 800 )
      vn.textbox_y = (nh-vn.textbox_h)/2
      vn.textbox_w = math.min( 0.8*nw, 1200 )
      vn.textbox_x = (nw-vn.textbox_w)/2
      vn.show_options = false
   end )
   local name = params.name or _("Notebook")
   local colour = params.textcolour or {1, 1, 1}
   local log = vn.newCharacter( name, { colour=colour, hidetitle=true } )
   vn.transition( params.transition )
   return log
end

local function fullscreenEnd( params )
   params = params or {}
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
      if params.done then
         vn.textbox_bg_alpha = 0
         vn.show_options = false
      else
         if params.characters then
            for k,c in ipairs(params.characters) do
               table.insert( vn._characters, c )
            end
         else
            vn._characters = characters
         end
      end
   end )
   if not params.notransition then
      vn.transition( params.transition, params.transition_length )
   end
end

--[[--
Converts the VN to behave like a notebook with hand-written text.

   @tparam[opt=_("Notebook")] string Name to give the "notebook" vn character.
   @treturn vn.Character The new character to use for the notebook.
--]]
function extras.notebookStart( name, params )
   params = params or {}
   local nw, nh = naev.gfx.dim()
   local paperbg = love_shaders.paper( nw, nh )
   local oldify = love_shaders.oldify()
   return fullscreenStart( function ()
      vn.setBackground( function ()
         vn.setColour( {1, 1, 1, 1} )
         lg.rectangle("fill", 0, 0, nw, nh )
         vn.setColour( {1, 1, 1, 0.3} )
         lg.setShader( oldify )
         paperbg:draw( 0, 0 )
         lg.setShader()
      end )
   end, {
      name = name or _("Notebook"),
      textcolour = {0, 0, 0},
      font = lg.newFont( _("fonts/CoveredByYourGrace-Regular.ttf"), 24 ),
      transition = params.transition,
   } )
end
extras.notebookEnd = fullscreenEnd

--[[--
Converts the VN to behave like a grainy flashback.

   @tparam[opt=_("Notebook")] string Name to give the "notebook" vn character.
   @treturn vn.Character The new character to use for the notebook.
--]]
function extras.flashbackTextStart( name, params )
   params = params or {}
   local nw, nh = naev.gfx.dim()
   return fullscreenStart( function ()
      --ft_oldify.shader:addPPShader( "final" )
      vn.setBackground( function ()
         vn.setColour( {0, 0, 0, 1} )
         lg.rectangle("fill", 0, 0, nw, nh )
      end )
   end, {
      name = name or _("Flashback"),
      textcolour = {0.8, 0.8, 0.8},
      font = lg.newFont( 18 ),
      transition = params.transition,
   } )
end
extras.flashbackTextEnd = fullscreenEnd

function extras.alarmStart ()
   vn.music( "snd/sounds/loops/alarm.ogg" ) -- blaring alarm
   vn.func( function ()
      local shd = love_shaders.tint{ colour={1.0, 0.0, 0.0} }
      local t = 0
      local v = math.sin(t)
      vn.setShader( shd )
      vn.setBackground( function ()
         local nw, nh = gfx.dim()
         lg.setColour{ 1, 0, 0, 0.03-0.03*v+0.02*math.min(t,1) }
         lg.rectangle( "fill", 0, 0, nw, nh )
      end )
      local function up( dt )
         t = t + dt
         v = math.cos( t * (math.pi*2) / 1.375)
         shd:send( "strength", 0.8+0.2*v-0.2*math.min(t,1) )
      end
      vn.setUpdateFunc( up )
      up(0)
   end )
end

function extras.alarmEnd ()
   vn.music()
   vn.func( function ()
      vn.setShader()
      vn.setBackground()
      vn.setUpdateFunc()
   end )
end

return extras
