--[[--
   Visual Novel API for Naev

   Based on Love2D API
   @module vn
]]
local utf8        = require 'utf8'
local love        = require 'love'
local love_math   = require 'love.math'
local graphics    = require 'love.graphics'
local window      = require 'love.window'
local filesystem  = require 'love.filesystem'
local audio       = require 'love.audio'
local lmusic      = require 'lmusic'
local transitions = require 'vn.transitions'
local log         = require "vn.log"
local sdf         = require "vn.sdf"
local opt         = require "vn.options"
local luaspfx     = require "luaspfx"
local luatk       = require "luatk"
local fmt         = require "format"

local vn = {
   speed = var.peek("vn_textspeed") or 0.025,
   autoscroll = (var.peek("vn_autoscroll")==true),
   nobounce = (var.peek("vn_nobounce")==true),

   -- Internal usage
   _default = {
      _characters = {},
      _states = {},
      _state = 0,
      _bufcol = { 0.95, 0.95, 0.95 },
      _buffer = "",
      _title = nil,
      _globalalpha = 1,
      --_soundTalk = audio.newSource( "snd/sounds/ui/letter0.wav" ),
      _pitchValues = {0.7, 0.8, 1.0, 1.2, 1.3},
      _buffer_y = 0,
   },
   transitions = transitions,
   _sfx = {
      bingo = audio.newSource( 'snd/sounds/jingles/success.ogg' ),
      eerie = audio.newSource( 'snd/sounds/jingles/eerie.ogg' ),
      money = audio.newSource( 'snd/sounds/jingles/money.ogg' ),
      victory = audio.newSource( 'snd/sounds/jingles/victory.ogg' ),
      ui = {
         option = audio.newSource( 'snd/sounds/ui/happy.wav' ),
      },
   },
}
-- Drawing
local function _setdefaults()
   local lw, lh = window.getDesktopDimensions()
   local mw, mh = 1280, 720
   local ox, oy = (lw-mw)/2, (lh-mh)/2
   vn._default.display_w = mw
   vn._default.display_h = mh

   vn._default.textbox_font = graphics.newFont(16)
   vn._default.textbox_font:setOutline( 0.5 )
   vn._default.textbox_w = 800
   local fonth = vn._default.textbox_font:getLineHeight()
   vn._default.textbox_h = math.floor(200 / fonth) * fonth + 20*2
   vn._default.textbox_x = ox + (mw-vn._default.textbox_w)/2
   vn._default.textbox_y = oy + mh-30-vn._default.textbox_h
   vn._default.textbox_bg = {0, 0, 0, 1}
   vn._default.textbox_bg_alpha = 1
   vn._default.textbox_text_alpha = 1
   vn._default.namebox_font = graphics.newFont(20)
   vn._default.namebox_w = -1 -- Autosize
   vn._default.namebox_h = 20*2+vn._default.namebox_font:getHeight()
   vn._default.namebox_x = vn._default.textbox_x
   vn._default.namebox_y = vn._default.textbox_y - vn._default.namebox_h - 20
   vn._default.namebox_bg = vn._default.textbox_bg
   vn._default.namebox_alpha = 1
   vn._default.menu_x = nil
   vn._default.menu_y = nil
   vn._default._postshader = nil
   vn._default._draw_fg = nil
   vn._default._draw_mg = nil
   vn._default._draw_bg = nil
   vn._default._updatefunc = nil
   -- Options
   vn._default.options_x = vn._default.textbox_x + vn._default.textbox_w + 20
   vn._default.options_y = vn._default.textbox_y
   vn._default.options_w = 30
   vn._default.options_h = 30
   vn._default.show_options = true
   vn._default._options_over = false
   vn._default._show_options = false
   -- These are implicitly dependent on lw, lh, so should be recalculated with the above.
   vn._canvas     = graphics.newCanvas()
   vn._prevcanvas = graphics.newCanvas()
   vn._curcanvas  = graphics.newCanvas()
   -- Empty canvas used for some transitions
   vn._emptycanvas = graphics.newCanvas()
   local oldcanvas = graphics.getCanvas()
   graphics.setCanvas( vn._emptycanvas )
   graphics.clear( 0, 0, 0, 0 )
   graphics.setCanvas( oldcanvas )
   -- Music stuff
   vn._handle_music = false
   -- Logging
   vn._show_log = false
   log.reset()
end

--[[--
Checks to see if the VN has started and errors if it does.

@local
--]]
function vn._checkstarted()
   if vn._started then
      error( _("vn: can't modify states when running") )
   end
end

--[[--
Sets the current drawing colour of the VN.

   @tparam tab col Colour table consisting of 3 or 4 number values.
   @tparam[opt=1] number alpha Additional alpha to multiply by.
--]]
function vn.setColour( col, alpha )
   local a = col[4] or 1
   alpha = alpha or 1
   a = a*alpha
   graphics.setColour( col[1], col[2], col[3], a*vn._globalalpha )
end

local function _draw_bg( x, y, w, h, col, border_col, alpha )
   col = col or {0, 0, 0, 1}
   border_col = border_col or {0.5, 0.5, 0.5, 1}
   vn.setColour( border_col, alpha )
   graphics.rectangle( "fill", x, y, w, h )
   vn.setColour( col, alpha )
   graphics.rectangle( "fill", x+2, y+2, w-4, h-4 )
end

local function _draw_character( c )
   if c.image == nil then return end
   local w, h = c.image:getDimensions()
   local isportrait = (w>=h)
   if c.params.isportrait ~= nil then
      isportrait = c.params.isportrait
   end
   local lw, lh = love.graphics.getDimensions()
   local scale, x, y
   if isportrait then
      scale = math.min( vn.textbox_w/w, (vn.textbox_y-(lh-vn.display_h)/2)/h )
      x = (lw-vn.display_w)/2 + c.offset*vn.display_w - w*scale/2
      y = vn.textbox_y-scale*h
   else
      scale = vn.display_h/h
      x = (lw-vn.display_w)/2 + c.offset*vn.display_w - w*scale/2
      y = (lh-vn.display_h)/2
   end
   y = y + (c.offy or 0)
   local col
   if c.talking then
      col = { 1, 1, 1 }
   else
      col = { 0.95, 0.95, 0.95 }
   end
   local flip
   if c.params.flip~=nil then
      if c.params.flip then
         flip = -1
         x = x + scale*w
      else
         flip = 1
      end
   else
      if c.offset > 0.5 then
         flip = -1
         x = x + scale*w
      else
         flip = 1
      end
   end
   local r = c.rotation or 0
   -- TODO why does rotation not work with shaders??
   if c.shader and c.shader.prerender then
      c.shader:prerender( c.image )
   end
   vn.setColour( col, c.alpha )
   graphics.setShader( c.shader )
   graphics.draw( c.image, x, y, r, flip*scale, scale )
   graphics.setShader()
end

--[[
-- Main drawing function.
--]]
local function _draw()
   local prevcanvas
   if vn._postshader then
      prevcanvas = graphics.getCanvas()
      graphics.setCanvas( vn._canvas )
      graphics.clear( 0, 0, 0, 0 )
   end

   -- Draw background
   if vn._draw_bg then
      vn._draw_bg()
   end

   local lw, lh = window.getDesktopDimensions()
   graphics.setScissor( (lw-vn.display_w)/2, (lh-vn.display_h)/2, vn.display_w, vn.display_h )
   -- Draw characters
   for k,c in ipairs( vn._characters ) do
      if not c.talking then
         _draw_character( c )
      end
   end
   for k,c in ipairs( vn._characters ) do
      if c.talking then
         _draw_character( c )
      end
   end
   graphics.setScissor()

   -- Textbox
   local font = vn.textbox_font
   -- Draw background
   local x, y, w, h = vn.textbox_x, vn.textbox_y, vn.textbox_w, vn.textbox_h
   local bw = 20
   local bh = 20
   _draw_bg( x, y, w, h, vn.textbox_bg, nil, vn.textbox_bg_alpha )
   -- Draw text
   vn.setColour( vn._bufcol, vn.textbox_text_alpha )
   -- We pad a bit here so that the top doesn't get cut off from certain
   -- characters that extend above the font height
   local padh = font:getLineHeight()-font:getHeight()
   graphics.setScissor( x, y+bh-padh, w, h-2*bh+padh )
   -- We're actually printing the entire text and using scissors to cut it out
   -- TODO only show the visible text while not trying to render it
   y = y + vn._buffer_y
   graphics.printf( vn._buffer, font, x+bw, y+bh, w-3*bw )
   graphics.setScissor()

   -- Namebox
   if vn._title ~= nil and utf8.len(vn._title)>0 then
      font = vn.namebox_font
      bw = 20
      bh = 20
      x = vn.namebox_x
      y = vn.namebox_y
      w = vn.namebox_w
      h = vn.namebox_h
      if w < 0 then
         w = font:getWidth( vn._title )+2*bw
      end
      _draw_bg( x, y, w, h, vn.namebox_bg, nil, vn.namebox_alpha )
      -- Draw text
      vn.setColour( vn._bufcol, vn.namebox_alpha )
      graphics.print( vn._title, font, x+bw, y+bh )
   end

   -- Custom Namebox
   if vn._draw_mg then
      vn._draw_mg()
   end

   -- Options
   if vn.show_options then
      x = vn.options_x
      y = vn.options_y
      w = vn.options_w
      h = vn.options_h
      local colbg
      if vn._options_over then
         colbg = {0.3, 0.3, 0.3 }
      else
         colbg = vn.textbox_bg
      end
      _draw_bg( x, y, w, h, colbg, nil, 1 )
      graphics.setShader( sdf.gear )
      vn.setColour( {0.95,0.95,0.95} )
      sdf.img:draw( x+5, y+5, 0, w-10, h-10 )
      graphics.setShader()
   end

   -- Draw current state if necessary
   if not vn.isDone() then
      local s = vn._states[ vn._state ]
      s:draw()
   end

   -- Draw foreground
   if vn._draw_fg then
      vn._draw_fg()
   end

   if vn._postshader then
      -- Draw canvas
      graphics.setCanvas( prevcanvas )
      graphics.setShader( vn._postshader )
      vn.setColour( {1, 1, 1, 1} )
      graphics.setBlendMode( "alpha", "premultiplied" )
      vn._canvas:draw( 0, 0 )
      graphics.setBlendMode( "alpha" )
      graphics.setShader()
   end
end
--[[--
Drawing function for the VN. Has to be called each loop in "love.draw".
--]]
function vn.draw()
   local s = vn._states[ vn._state ]
   if s and s.drawoverride then
      s:drawoverride()
   else
      _draw()
   end

   -- Draw on top
   if vn._show_log then
      log.draw()
   end

   if vn._show_options then
      opt.draw()
   end
end
local function _draw_to_canvas( canvas )
   local oldcanvas = graphics.getCanvas()
   graphics.setCanvas( canvas )
   graphics.clear( 0, 0, 0, 0 )
   _draw()
   graphics.setCanvas( oldcanvas )
end

--[[--
Main updating function. Has to be called each loop in "love.update"
   @tparam number dt Update tick in seconds.
]]
function vn.update( dt )
   lmusic.update( dt )

   if vn._show_options then
      vn._show_options = opt.update( dt )
      if vn._show_options then
         vn.speed = var.peek("vn_textspeed") or 0.025
      end
      return
   end

   -- Basically skip the entire VN framework
   if vn._show_log then
      vn._show_log = log.update( dt )
      return
   end

   -- Out of states
   if vn._state > #vn._states then
      love.event.quit()
      return
   end

   if vn.isDone() then return end

   if vn._state < 0 then
      vn._state = 1
   end

   for k,c in ipairs( vn._characters ) do
      if c.shader and c.shader.update then
         c.shader:update(dt)
      end
   end

   -- Update shader if necessary
   if vn._postshader and vn._postshader.update then
      vn._postshader:update( dt )
   end

   -- Custom update function
   if vn._updatefunc then
      vn._updatefunc(dt)
   end

   local s = vn._states[ vn._state ]
   s:update( dt )
end

--[[--
Handles resizing of the windows.
   @tparam number _w New width.
   @tparam number _h New height.
--]]
function vn.resize( _w, _h )
   -- TODO fix scissoring breaking for the characters
end

--[[--
Key press handler.
   @tparam string key Name of the key pressed.
   @tparam boolean isrepeat Whether or not the string is repeating.
--]]
function vn.keypressed( key, isrepeat )
   if love._vn_keyrepeat_check and isrepeat then
      return false
   end
   love._vn_keyrepeat_check = false

   local tkopen = luatk.isOpen()
   if not tkopen and string.lower(naev.keyGet( "menu" )) == key then
      naev.menuSmall()
      return true
   end

   if vn._show_options then
      opt.keypressed( key )
      return true
   end

   -- Skip modifier keys
   local blacklist = {
      "left alt",
      "left shift",
      "left ctrl",
      "left gui",
      "right alt",
      "right ctrl",
      "right shift",
      "right gui",
   }
   for k,v in ipairs(blacklist) do
      if key == v then
         return false
      end
   end

   if not vn._show_log then
      if not tkopen and (key=="tab" or key=="escape") then
         vn._show_log = not vn._show_log
         log.open( vn.textbox_font )
         return true
      end
   else
      return log.keypress( key )
   end

   if vn.isDone() then return end
   local s = vn._states[ vn._state ]
   --return s:keypressed( key )
   -- Always eat all keys for now
   s:keypressed( key )
   return true
end

local function _inbox( mx, my, x, y, w, h )
   return (mx>=x and mx<=x+w and my>=y and my<=y+h)
end

--[[--
Mouse move handler.
--]]
function vn.mousemoved( mx, my, dx, dy )
   if vn._show_options then
      opt.mousemoved( mx, my )
      return true
   end

   if vn.show_options and _inbox( mx, my, vn.options_x, vn.options_y, vn.options_w, vn.options_h ) then
      vn._options_over = true
      return true
   else
      vn._options_over = false
   end

   if vn.isDone() then return false end
   local s = vn._states[ vn._state ]
   return s:mousemoved( mx, my, dx, dy )
end

--[[--
Mouse press handler.
   @tparam number mx X position of the click.
   @tparam number my Y position of the click.
   @tparam number button Button that was pressed.
--]]
function vn.mousepressed( mx, my, button )
   if vn._show_options then
      opt.mousepressed( mx, my, button )
      return true
   end

   if vn._show_log then
      log.mousepressed( mx, my, button )
      return true
   end

   if vn.show_options and _inbox( mx, my, vn.options_x, vn.options_y, vn.options_w, vn.options_h ) then
      opt.open( vn )
      vn._show_options = true
      return
   end

   if vn.isDone() then return false end
   local s = vn._states[ vn._state ]
   return s:mousepressed( mx, my, button )
end

--[[--
Mouse released handler.
   @tparam number mx X position of the click.
   @tparam number my Y position of the click.
   @tparam number button Button that was pressed.
--]]
function vn.mousereleased( mx, my, button )
   if vn._show_options then
      opt.mousereleased( mx, my, button )
      return true
   end

   if vn.isDone() then return false end
   local s = vn._states[ vn._state ]
   return s:mousereleased( mx, my, button )
end

--[[--
Mouse wheel handler.
--]]
function vn.wheelmoved( dx, dy )
   if vn.isDone() then return false end
   local s = vn._states[ vn._state ]
   return s:wheelmoved( dx, dy )
end

--[[--
Text input handler.
--]]
function vn.textinput( str )
   if vn.isDone() then return false end
   local s = vn._states[ vn._state ]
   return s:textinput( str )
end

-- Helpers
--[[--
Makes the player say something.

   @tparam string what What is being said.
   @tparam bool noclear Whether or not to clear the text buffer.
   @tparam bool nowait Whether or not to wait for player input when said.
]]
function vn.me( what, noclear, nowait ) vn.say( "You", what, noclear, nowait ) end
--[[--
Makes the narrator say something.

   @tparam string what What is being said.
   @tparam bool noclear Whether or not to clear the text buffer.
   @tparam bool nowait Whether or not to wait for player input when said.
]]
function vn.na( what, noclear, nowait ) vn.say( "Narrator", what, noclear, nowait ) end

--[[
-- State
--]]
vn.State = {}
vn.State_mt = { __index = vn.State }
local function _dummy() end
local function _finish(self) self.done = true end
function vn.State.new()
   local s = {}
   setmetatable( s, vn.State_mt )
   s._type = "State"
   s._init = _dummy
   s._draw = _dummy
   s._update = _dummy
   s._mousepressed = _dummy
   s._mousereleased = _dummy
   s._mousemoved = _dummy
   s._wheelmoved = _dummy
   s._keypressed = _dummy
   s._textinput = _dummy
   s.done = false
   return s
end
function vn.State:type() return self._type end
function vn.State:init()
   self.done = false
   self:_init()
end
function vn.State:draw()
   self:_draw()
   vn._checkDone()
end
function vn.State:update( dt )
   self:_update( dt )
   vn._checkDone()
end
function vn.State:mousepressed( mx, my, button )
   local ret = self:_mousepressed( mx, my, button )
   vn._checkDone()
   return ret
end
function vn.State:mousereleased( mx, my, button )
   local ret = self:_mousereleased( mx, my, button )
   vn._checkDone()
   return ret
end
function vn.State:mousemoved( mx, my, dx, dy )
   local ret = self:_mousemoved( mx, my, dx, dy )
   vn._checkDone()
   return ret
end
function vn.State:wheelmoved( dx, dy )
   local ret = self:_wheelmoved( dx, dy )
   vn._checkDone()
   return ret
end
function vn.State:keypressed( key )
   local ret = self:_keypressed( key )
   vn._checkDone()
   return ret
end
function vn.State:textinput( key )
   local ret = self:_textinput( key )
   vn._checkDone()
   return ret
end
function vn.State:isDone() return self.done end
local __needs_transition
--[[
-- Scene
--]]
vn.StateScene ={}
function vn.StateScene.new()
   local s = vn.State.new()
   s._init = vn.StateScene._init
   s._type = "Scene"
   return s
end
function vn.StateScene:_init()
   __needs_transition = true

   -- Render previous scene to an image
   local canvas = vn._prevcanvas
   _draw_to_canvas( canvas )
   vn._prevscene = canvas

   -- Reset characters
   vn._characters = {
      vn._me,
      vn._na
   }

   -- Set alpha to max (since transitions will be used in general)
   vn._globalalpha = 1

   -- Clear stuff
   vn._buffer = ""
   vn._buffer_y = 0
   vn._title = nil

   _finish(self)
end
--[[
-- Character
--]]
vn.StateCharacter ={}
function vn.StateCharacter.new( character, remove )
   local s = vn.State.new()
   s._init = vn.StateCharacter._init
   s._type = "Character"
   s.character = character
   s.remove = remove or false
   return s
end
local function _getpos( pos )
   pos = pos or "center"
   if type(pos)=="number" then
      return pos
   elseif pos == "center" then
      return 0.5
   elseif pos == "left" then
      return 0.25
   elseif pos == "right" then
      return 0.75
   elseif pos == "farleft" then
      return 0.15
   elseif pos == "farright" then
      return 0.85
   end
   return 0.5
end
function vn.StateCharacter:_init()
   if self.remove then
      local found = false
      for k,c in ipairs(vn._characters) do
         if c==self.character then
            table.remove( vn._characters, k )
            found = true
            break
         end
      end
      if not found then
         error(_("vn: character not found to remove!"))
      end
   else
      local c = self.character
      table.insert( vn._characters, c )
      c.alpha = 1
      c.displayname = c.who -- reset name
      self.character.offset = _getpos( self.character.pos )
   end
   _finish(self)
end
--[[
-- Say
--]]
vn.StateSay = {}
function vn.StateSay.new( who, what, noclear )
   local s = vn.State.new()
   s._init = vn.StateSay._init
   s._update = vn.StateSay._update
   s._mousepressed = vn.StateSay._finish
   s._keypressed = vn.StateSay._finish
   s._type = "Say"
   s.who = who
   s._what = what
   s._noclear = noclear
   return s
end
function vn.StateSay:_init()
   if __needs_transition then
      warn(_("vn: vn.say being used after vn.scene without a vn.transition!"))
   end

   -- Get the main text
   if type(self._what)=="function" then
      self.what = self._what()
   else
      self.what = self._what
   end
   self._textbuf = self.what
   -- Parse for line breaks and insert newlines
   local font = vn.textbox_font
   local _maxw, wrappedtext = font:getWrap( self._textbuf, vn.textbox_w-60 )
   self._textbuf = table.concat( wrappedtext, "\n" )
   -- Set up initial buffer
   self._timer = vn.speed
   if self._noclear then
      self._pos = utf8.len( vn._buffer )
      self._textbuf = vn._buffer .. self._textbuf
      self._text = vn._buffer
      vn._buffer = self._text
   else
      self._pos = utf8.next( self._textbuf )
      self._text = ""
      -- Initialize scroll
      vn._buffer_y = 0
   end
   self._len = utf8.len( self._textbuf )
   local c = vn._getCharacter( self.who )
   vn._bufcol = c.colour or vn._default._bufcol
   if c.hidetitle then
      vn._title = nil
   else
      vn._title = c.displayname or c.who
   end
   local wastalking = c.talking
   -- Reset talking
   for k,v in ipairs( vn._characters ) do
      v.talking = false
   end
   c.talking = true
   if not vn.nobounce and c.talking and not wastalking then
      self.bounce = 0
      c.offy = 0
   else
      self.bounce = nil
   end
end
function vn.StateSay:_update( dt )
   if self.bounce ~= nil then
      local c = vn._getCharacter( self.who )
      self.bounce = self.bounce + dt
      c.offy = math.sin( self.bounce * 5 * math.pi ) * math.max(0, (1-self.bounce)^2) * 10
      if self.bounce > 1 then
         self.bounce = nil
         c.offy = 0
      end
   end
   self._timer = self._timer - dt
   while self._timer < 0 do
      -- No more characters left -> done!
      if utf8.len(self._text) == self._len then
         vn.StateSay._finish( self )
         return
      end
      self._pos = utf8.next( self._textbuf, self._pos )
      self._text = utf8.sub( self._textbuf, 1, self._pos )
      self._timer = self._timer + vn.speed
      vn._buffer = self._text

      -- play sound
      if vn._soundTalk then
         local p = vn._pitchValues[ rnd.rnd(1,#vn._pitchValues) ]
         vn._soundTalk:setPitch( p )
         vn._soundTalk:play()
      end

      -- Checks to see if we should scroll down
      local bh = 20
      local font = vn.textbox_font
      local _maxw, wrappedtext = font:getWrap( self._text, vn.textbox_w-60 )
      local lh = font:getLineHeight()
      if (lh * #wrappedtext + bh + vn._buffer_y > vn.textbox_h) then
         if vn.autoscroll then
            vn._buffer_y = vn._buffer_y - lh
         else
            vn.StateSay._finish( self )
         end
      end
   end
end
function vn.StateSay:_finish()
   self._text = self._textbuf
   vn._buffer = self._text

   local c = vn._getCharacter( self.who )
   c.offy = nil
   log.add{
      who   = c.displayname or c.who,
      what  = self.what, -- Avoids newlines
      colour= c.colour or vn._default._bufcol,
   }

   _finish( self )
end
--[[
-- Wait
--]]
vn.StateWait = {}
function vn.StateWait.new()
   local s = vn.State.new()
   s._init = vn.StateWait._init
   s._draw = vn.StateWait._draw
   s._mousepressed = vn.StateWait._mousepressed
   s._wheelmoved = vn.StateWait._wheelmoved
   s._keypressed = vn.StateWait._keypressed
   s._type = "Wait"
   return s
end
local function _check_scroll( lines )
   local bh = 20
   local font = vn.textbox_font
   local lh = font:getLineHeight()
   return (lh * #lines + bh + vn._buffer_y > vn.textbox_h)
end
function vn.StateWait:_init()
   local x, y, w, h = vn.textbox_x, vn.textbox_y, vn.textbox_w, vn.textbox_h
   self._w = 25
   self._h = 25
   self._x = x+w-10-self._w
   self._y = y+h-10-self._h

   local bw = 20
   local font = vn.textbox_font
   local _maxw, wrappedtext = font:getWrap( vn._buffer, vn.textbox_w-2*bw )
   self._lines = wrappedtext
   self._lh = font:getLineHeight()

   self._scrolled = _check_scroll( self._lines )
end
function vn.StateWait:_draw()
   --vn.setColour( vn._bufcol )
   vn.setColour( {0,1,1,1} )
   if vn._buffer_y < 0 then
      graphics.setShader( sdf.arrow )
      graphics.draw( sdf.img, self._x-10, vn.textbox_y+30, -math.pi/2, 45, 15  )
   end
   if _check_scroll( self._lines ) then
      graphics.setShader( sdf.arrow )
      graphics.draw( sdf.img, self._x-10, vn.textbox_y+vn.textbox_h-40, math.pi/2, 45, 15 )
   elseif not self._scrolled then
      graphics.setShader( sdf.cont )
      graphics.draw( sdf.img, self._x, self._y, 0, self._w, self._h )
   end
   graphics.setShader()
end
local function wait_scrollorfinish( self )
   if _check_scroll( self._lines ) then
      vn._buffer_y = vn._buffer_y - self._lh
   elseif self._scrolled then
      self._scrolled = false
   else
      _finish(self)
   end
end
function vn.StateWait:_mousepressed( _mx, _my, _button )
   wait_scrollorfinish( self )
end
function vn.StateWait:_wheelmoved( _dx, dy )
   if dy > 0 then -- upwards movement
      vn._buffer_y = vn._buffer_y + vn.textbox_font:getLineHeight()
      vn._buffer_y = math.min( 0, vn._buffer_y )
      self._scrolled = _check_scroll( self._lines )
      return true
   elseif dy < 0 then -- downwards movement
      vn._buffer_y = vn._buffer_y - vn.textbox_font:getLineHeight()
      vn._buffer_y = math.max( vn._buffer_y, (vn.textbox_h - 40) - vn.textbox_font:getLineHeight() * (#self._lines) )
      self._scrolled = _check_scroll( self._lines )
      -- we don't check for finish, have to click for that
      return true
   end
   return false
end
function vn.StateWait:_keypressed( key )
   if key=="up" then
      if vn._buffer_y < 0 then
         vn._buffer_y = vn._buffer_y + vn.textbox_font:getLineHeight()
      end
      vn._buffer_y = math.min( 0, vn._buffer_y )
      self._scrolled = _check_scroll( self._lines )
      return true
   elseif key=="pageup" then
      if vn._buffer_y < 0 then
         local fonth = vn.textbox_font:getLineHeight()
         local h = (math.floor( vn.textbox_h / fonth )-1) * fonth
         vn._buffer_y = math.min( 0, vn._buffer_y+h )
      end
      self._scrolled = _check_scroll( self._lines )
      return true
   elseif key=="pagedown" then
      local fonth = vn.textbox_font:getLineHeight()
      local h = (math.floor( vn.textbox_h / fonth )-2) * fonth -- wait_scrollorfinish adds an extra line movement
      vn._buffer_y = math.max( vn._buffer_y-h, (vn.textbox_h - 40) - vn.textbox_font:getLineHeight() * (#self._lines) )
      wait_scrollorfinish( self )
      return true
   elseif key=="home" then
      vn._buffer_y = 0
      self._scrolled = _check_scroll( self._lines )
      return true
   elseif key=="end" then
      vn._buffer_y = (vn.textbox_h - 40) - vn.textbox_font:getLineHeight() * (#self._lines)
      wait_scrollorfinish( self )
      return true
   end

   local whitelist = {
      "enter",
      "space",
      "right",
      "down",
      "escape",
   }
   local found = false
   for k,v in ipairs(whitelist) do
      if key == v then
         found = true
         break
      end
   end
   if not found then
      return false
   end

   wait_scrollorfinish( self )
   return true
end
--[[
-- Menu
--]]
vn.StateMenu = {}
function vn.StateMenu.new( items, handler )
   local s = vn.State.new()
   s._init = vn.StateMenu._init
   s._draw = vn.StateMenu._draw
   s._mousepressed = vn.StateMenu._mousepressed
   s._keypressed = vn.StateMenu._keypressed
   s._type = "Menu"
   s.items = items
   s._items = nil
   s.handler = handler
   s._choose = vn.StateMenu._choose
   return s
end
function vn.StateMenu:_init()
   -- Check to see if function
   if type(self.items)=="function" then
      self._items = self.items()
   else
      self._items = self.items
   end
   if __debugging then
      if self._items == nil or #self._items==0 then
         error(_("vn: menu has no options!"))
      end
   end
   -- Set up the graphics stuff
   local font = vn.namebox_font
   -- Border information
   local tb = 15 -- Inner border around text
   local b = 15 -- Outter border
   self._tb = tb
   self._b = b
   -- Get longest line
   local wmin = 300
   local wmax = 900
   local w = 0
   local h = 0
   self._elem = {}
   for k,v in ipairs(self._items) do
      local text = string.format("#w%d#0. %s", k, v[1])
      local sw, wrapped = font:getWrap( text, wmax )
      sw = sw + 2*tb
      local sh =  2*tb + font:getHeight() + font:getLineHeight() * (#wrapped-1)
      local elem = { text, 0, h, sw, sh }
      if w < sw then
         w = sw
      end
      h = h + sh + b
      self._elem[k] = elem
   end
   self._w = math.max( w, wmin ) -- enforce minimum size
   self._h = h-b
   self._x = (love.w-self._w)/2
   self._y = (love.h-self._h)/2-100
   if vn.menu_x then
      if vn.menu_x > 0 then
         self._x = vn.menu_x
      else
         self._x = love.w - self._w + vn.menu_x
      end
   end
   if vn.menu_y then
      if vn.menu_y > 0 then
         self._y = vn.menu_y
      else
         self._y = love.w - self._w + vn.menu_y
      end
   end
   -- Make all boxes max width
   for k,v in ipairs(self._elem) do
      v[4] = self._w
   end
end
function vn.StateMenu:_draw()
   local font = vn.namebox_font
   local gx, gy, gw, gh = self._x, self._y, self._w, self._h
   local b = self._b
   _draw_bg( gx-b, gy-b, gw+2*b, gh+2*b )
   local tb = self._tb
   local mx, my = love.mouse.getX(), love.mouse.getY()
   for k,v in ipairs(self._elem) do
      local text, x, y, w, h = table.unpack(v)
      local col
      if _inbox( mx, my, gx+x, gy+y, w, h ) then
         -- Mouse over
         col = {0.3, 0.3, 0.3}
      else
         col = {0.2, 0.2, 0.2}
      end
      vn.setColour( col )
      graphics.rectangle( "fill", gx+x, gy+y, w, h )
      vn.setColour( {0.7, 0.7, 0.7} )
      graphics.printf( text, font, gx+x+tb, gy+y+tb, w-tb*2 )
   end
end
function vn.StateMenu:_mousepressed( mx, my, button )
   if button ~= 1 then
      return
   end
   local gx, gy = self._x, self._y
   local b = self._tb
   for k,v in ipairs(self._elem) do
      local _text, x, y, w, h = table.unpack(v)
      if _inbox( mx, my, gx+x-b, gy+y-b, w+2*b, h+2*b ) then
         self:_choose(k)
         return true
      end
   end
end
function vn.StateMenu:_keypressed( key )
   local n = tonumber(key)
   if n == nil then return false end
   if n==0 then n = n + 10 end
   if n > #self._items then return false end
   self:_choose(n)
   return true
end
function vn.StateMenu:_choose( n )
   vn._sfx.ui.option:play()
   self.handler( self._items[n][2] )
   log.add{
      who   = _("[CHOICE]"),
      what  = self._items[n][1],
      colour= vn._default._bufcol,
   }
   _finish( self )
end
--[[
-- Label
--]]
vn.StateLabel ={}
function vn.StateLabel.new( label )
   local s = vn.State.new()
   s._init = _finish
   s._type = "Label"
   s.label = label
   return s
end
--[[
-- Jump
--]]
vn.StateJump ={}
function vn.StateJump.new( label )
   local s = vn.State.new()
   s._init = vn.StateJump._init
   s._type = "Jump"
   s.label = label
   return s
end
function vn.StateJump:_init()
   vn._jump( self.label )
   _finish(self)
end
--[[
-- Start
--]]
vn.StateStart ={}
function vn.StateStart.new()
   local s = vn.State.new()
   s._init = vn.StateStart._init
   s._type = "Start"
   return s
end
function vn.StateStart:_init()
   local oldcanvas = graphics.getCanvas()
   graphics.setCanvas( vn._prevcanvas )
   graphics.clear( 0, 0, 0, 0 )
   graphics.setCanvas( oldcanvas )
   vn._globalalpha = 0
   _finish(self)
end
--[[
-- End
--]]
vn.StateEnd ={}
function vn.StateEnd.new()
   local s = vn.State.new()
   s._init = vn.StateEnd._init
   s._type = "End"
   return s
end
function vn.StateEnd._init( _self )
   vn._state = #vn._states+1
end
--[[
-- Animation
--]]
vn.StateAnimation = {}
function vn.StateAnimation.new( seconds, func, drawfunc, transition, initfunc, drawoverride )
   transition = transition or "ease"
   local s = vn.State.new()
   s._init = vn.StateAnimation._init
   s._update = vn.StateAnimation._update
   s._draw = vn.StateAnimation._draw
   if drawoverride then
      s.drawoverride = vn.StateAnimation._drawoverride
      s._drawoverride = drawoverride
   end
   s._type = "Animation"
   s._seconds = seconds
   s._func = func
   s._drawfunc = drawfunc
   s._initfunc = initfunc
   -- These transitions are taken from CSS spec
   if type(transition)=='table' then
      s._x2, s._y2, s._x3, s._y3 = table.unpack(transition)
   elseif transition=="ease" then
      s._x2, s._y2, s._x3, s._y3 = 0.25, 0.1, 0.25, 1
   elseif transition=="ease-in" then
      s._x2, s._y2, s._x3, s._y3 = 0.42, 0, 1, 1
   elseif transition=="ease-out" then
      s._x2, s._y2, s._x3, s._y3 = 0, 0, 0.58, 1
   elseif transition=="ease-in-out" then
      s._x2, s._y2, s._x3, s._y3 = 0.42, 0, 0.58, 0
   elseif transition=="linear" then
      s._x2, s._y2, s._x3, s._y3 = 0, 0, 1, 1
   end
   return s
end
function vn.StateAnimation:_init()
   if self._initfunc then
      self._params = self._initfunc()
   end
   self._accum = 0
   if self._func then
      self._func( 0, 0, self._params )
   end
   if self._drawfunc then
      self._drawfunc( 0, self._params )
   end
end
-- quadratic bezier curve
local function _bezier( u, x2, x3 )
   -- We assume x1,y1 is always 0,0 and x4,y4 is always 1,1
   local c4 = math.pow(1-u,3)
   local c3 = 3*u*math.pow(1-u,2)
   local c2 = 3*math.pow(u,2)*(1-u)
   --local c1 = math.pow(u,3)
   return c4+c3*x3+c2*x2
end
-- simple binary search, should work for monotonic functions
local function _bezier_yatx( xtarget, x2, x3, y2, y3 )
   if xtarget==0 then
      return 0
   elseif xtarget==1 then
      return 1
   end
   local lower = 0
   local upper = 1
   local middle = 0.5
   local xtol = 0.001 -- usually finishes in a few iterations
   local x = _bezier( middle, x2, x3 )
   while math.abs(xtarget-x) > xtol do
      if xtarget < x then
         lower = middle
      else
         upper = middle
      end
      middle = (upper+lower)/2
      x = _bezier( middle, x2, x3 )
   end
   return _bezier( middle, y2, y3 )
end
local function _animation_alpha( self )
   return _bezier_yatx( self._accum / self._seconds, self._x2, self._x3, self._y2, self._y3 )
end
function vn.StateAnimation:_update(dt)
   self._accum = self._accum + dt
   if self._accum > self._seconds then
      self._accum = self._seconds
      _finish(self)
   end
   if self._func then
      self._func( _animation_alpha(self), dt, self._params )
   end
end
function vn.StateAnimation:_draw( _dt )
   if self._drawfunc then
      self._drawfunc( _animation_alpha(self), self._params )
   end
end
function vn.StateAnimation:_drawoverride( _dt )
   self._drawoverride( _animation_alpha(self), self._params )
end

--[[--
A visual novel character.
--]]
vn.Character = {}
--[[--
Makes a character say something.
   @tparam string what What is being said.
   @tparam bool noclear Whether or not to clear the text buffer.
   @tparam bool nowait Whether or not to wait for player input when said.
--]]
function vn.Character:say( what, noclear, nowait ) return vn.say( self.who, what, noclear, nowait ) end
vn.Character_mt = { __index = vn.Character, __call = vn.Character.say }
--[[--
Creates a new character without adding it to the VN.
<em>Note</em> The character can be added with vn.newCharacter.
   @tparam string who Name of the character to add.
   @tparam tab params Parameter table.
   @treturn Character New character.
--]]
function vn.Character.new( who, params )
   local c = {}
   setmetatable( c, vn.Character_mt )
   params = params or {}
   c.who = who
   c.colour = params.colour or vn._default.colour
   local pimage = params.image
   if pimage ~= nil then
      local img
      if type(pimage)=='string' then
         local searchpath = {
            "",
            "gfx/vn/characters/",
         }
         for k,s in ipairs(searchpath) do
            local info = filesystem.getInfo( s..pimage )
            if info ~= nil then
               img = graphics.newImage( s..pimage )
               if img ~= nil then
                  break
               end
            end
         end
         if img == nil then
            error(string.format(_("vn: character image '%s' not found!"),pimage))
         end
         local iw, ih = img:getDimensions()
         if iw <= 256 or ih <= 256 then
            img:setFilter( "linear", "nearest" )
         end
      elseif pimage._type=="ImageData" then
         img = graphics.newImage( pimage )
      else
         img = pimage
      end
      c.image = img
   else
      c.image = nil
   end
   c.shader = params.shader
   c.hidetitle = params.hidetitle
   c.pos = params.pos
   c.rotation = params.rotation
   c.params = params
   return c
end
--[[--
Renames a character on the fly.
   @tparam string newname New name to give the character.
--]]
function vn.Character:rename( newname )
   vn.func( function ( _state )
      self.displayname = newname
   end )
end
--[[--
Creates a new character.
   @tparam string who Name (or previously created character) to add.
   @tparam tab params Parameter table.
   @treturn Character New character.
]]
function vn.newCharacter( who, params )
   local c
   if type(who)=="string" then
      c = vn.Character.new( who, params )
   else
      c = who
   end
   table.insert( vn._states, vn.StateCharacter.new( c ) )
   return c
end

local function _appear_setup( c, shader )
   -- Create new drawables
   vn.func( function ()
      shader:send( "texprev", vn._emptycanvas )
      for k,v in ipairs(c) do
         if v.image then
            local d = graphics.Drawable.new()
            d.image = v.image
            d.getDimensions = function ( self, ... )
               return self.image:getDimensions(...)
            end
            d.draw = function ( self, ... )
               local oldcanvas = graphics.getCanvas()
               graphics.setCanvas( vn._curcanvas )
               graphics.clear( 0, 0, 0, 0 )
               self.image:draw( ... )
               graphics.setCanvas( oldcanvas )

               local oldshader = graphics.getShader()
               graphics.setShader( shader )
               vn.setColour( {1, 1, 1, 1} )
               graphics.setBlendMode( "alpha", "premultiplied" )
               vn._curcanvas:draw( 0, 0 )
               graphics.setBlendMode( "alpha" )
               graphics.setShader( oldshader )
            end
            v.image = d
         else
            warn(fmt.f(_("vn: Appearing character '{c}' without an image!"),{c=(v.displayname or v.who)}))
         end
      end
   end )
end
local function _appear_cleanup( c )
   -- Undo new drawables
   vn.func( function ()
      for k,v in ipairs(c) do
         if v.image then
            v.image = v.image.image
         end
      end
   end )
end

--[[--
Makes a character appear in the VN.
   @see vn.transition
   @see vn.appear
   @tparam Character|table c Character or table of Characters to make appear.
   @tparam[opt="fade"] string name Name of the transition effect to use (see vn.transition)
   @tparam number seconds Seconds to do the transition.
   @tparam[opt="linear"] string transition Name of the CSS transition to use.
--]]
function vn.appear( c, name, seconds, transition )
   local shader
   shader, seconds = transitions.get( name, seconds, transition )
   if getmetatable(c)==vn.Character_mt then
      c = {c}
   end

   -- New character
   for k,v in ipairs(c) do
      vn.newCharacter(v)
   end

   _appear_setup( c, shader )
   -- Create the transition
   vn.animation( seconds, function( progress, dt )
      shader:send( "progress", progress )
      if shader.update then
         shader:update( dt )
      end
   end )
   _appear_cleanup( c )
end
--[[--
Makes a character disappear in the VN.

The way it works is that the transition is played backwards, so if you want the character to slide left, use "slideright"!

@usage vn.disappear( c, "fade", 3 )

   @see vn.transition
   @see vn.appear
   @tparam Character c Character to make disappear.
   @tparam[opt="fade"] string name Name of the transition effect to use (see vn.transition)
   @tparam number seconds Seconds to do the transition.
   @tparam[opt="linear"] string transition Name of the CSS transition to use.
--]]
function vn.disappear( c, name, seconds, transition )
   local shader
   shader, seconds = transitions.get( name, seconds, transition )
   if getmetatable(c)==vn.Character_mt then
      c = {c}
   end
   _appear_setup( c, shader )
   vn.animation( seconds, function( progress, dt )
      shader:send( "progress", 1-progress )
      if shader.update then
         shader:update( dt )
      end
   end )
   _appear_cleanup( c )
   for k,v in ipairs(c) do
      table.insert( vn._states, vn.StateCharacter.new( v, true ) )
   end
end

--[[--
Moves a character to another position.

   @see vn.animation
   @tparam Character c Character to move.
   @tparam[opt="center"] string|number pos Position to move to. Can be either a [0,1] value, "center", "left", "right", "farleft", or "farright".
--]]
function vn.move( c, pos )
   local function runinit ()
      local cpos = c.offset
      local tpos = _getpos( pos )
      return { cpos, tpos }
   end
   vn.animation( 1, function( alpha, _dt, params )
      local cpos, tpos = table.unpack(params)
      c.offset = tpos*alpha + cpos*(1-alpha)
   end, nil, "ease-in-out", runinit )
end

--[[--
Starts a new scene.
--]]
function vn.scene()
   vn._checkstarted()
   table.insert( vn._states, vn.StateScene.new() )
end

--[[--
Has a character say something.

<em>Note</em> "You" and "Narrator" are special meta-characters.

   @tparam string who The name of the character that is saying something.
   @tparam string what What the character is saying.
   @tparam[opt=false] bool noclear Whether or not to clear the text buffer.
   @tparam[opt=false] bool nowait Whether or not to introduce a wait or just skip to the next text right away (defaults to false).
]]
function vn.say( who, what, noclear, nowait )
   vn._checkstarted()
   if type(what)~="table" then
      what = {what}
   end
   for k,s in ipairs(what) do
      table.insert( vn._states, vn.StateSay.new( who, s, noclear ) )
      if not nowait then
         table.insert( vn._states, vn.StateWait.new() )
      end
   end
end

--[[--
Opens a menu the player can select from.
   @tparam table items Table of items to select from, they should be of the form "{text, label}" where "text" is what is displayed and "label" is what is passed to the handler.
   @tparam[opt=vn.jump] function handler Function to handle what happens when an item is selecetdd. Defaults to vn.jump.
--]]
function vn.menu( items, handler )
   vn._checkstarted()
   handler = handler or vn.jump
   table.insert( vn._states, vn.StateMenu.new( items, handler ) )
end

--[[--
Inserts a label. This does nothing but serve as a reference for vn.jump
   @see vn.jump
   @tparam string label Name of the label to insert.
--]]
function vn.label( label )
   vn._checkstarted()
   table.insert( vn._states, vn.StateLabel.new( label ) )
end

--[[--
Inserts a jump. This skips to a certain label.
   @see vn.label
   @tparam string label Name of the label to jump to.
--]]
function vn.jump( label )
   if vn._started then
      vn._jump( label )
   end
   table.insert( vn._states, vn.StateJump.new( label ) )
end

--[[--
Plays music. This will stop all other playing music unless dontstop is set to true.

This gets automatically reset when the VN finishes.

   @see lmusic.play
   @tparam string|nil filename Name of the music to play. If nil, it tries to restore the music again.
   @tparam tab params Same as lmusic.play()
   @tparam boolean dontstop Don't stop other music.
--]]
function vn.music( filename, params, dontstop )
   vn._checkstarted()
   local m = {}
   if filename == nil then
      vn.func( function ()
         music.play()
         lmusic.stop()
      end )
   else
      vn.func( function ()
         if not dontstop then
            music.stop()
            lmusic.stop()
         end
         vn._handle_music = true
         m.m = lmusic.play( filename, params )
      end )
   end
   return m
end

--[[--
Stops certain playing music.

   @tparam string|nil filename Name of the music to stop. If nil, it tries to stop all playing music.
--]]
function vn.musicStop( filename )
   vn.func( function ()
      if type(filename)=="table" and filename.m then
         lmusic.stop( filename.m )
      else
         lmusic.stop( filename )
      end
   end )
end

--[[--
Sets the pitch for playing music.

   @tparam table|string|nil data Name of the music to change pitch of or all if nil. Passing the table returned from vn.music can also be done to specify the music.
   @tparam number p Pitch to set.
--]]
function vn.musicPitch( data, p )
   vn.func( function ()
      if type(data)=="table" and data.m then
         lmusic.setPitch( data.m, p )
      else
         lmusic.setPitch( data, p )
      end
   end )
end

--[[--
Sets the volume for playing music.

   @tparam table|string|nil data Name of the music to change volume of or all if nil. Passing the table returned from vn.music can also be done to specify the music.
   @tparam number p Volume to set.
--]]
function vn.musicVolume( data, p )
   vn.func( function ()
      if type(data)=="table" and data.m then
         lmusic.setVolume( data.m, p )
      else
         lmusic.setVolume( data, p )
      end
   end )
end

--[[--
Finishes the VN.
   @see vn.transition
   @param ... Uses the parameters as vn.transition.
--]]
function vn.done( ... )
   vn._checkstarted()
   vn.scene()
   vn.func( function ()
      vn._globalalpha = 0
      vn._draw_fg = nil
      vn._draw_mg = nil
      vn._draw_bg = nil
      vn._postshader = nil
      vn._update = nil
      if vn._handle_music then
         lmusic.stop()
         if not music.isPlaying() then
            music.play()
         end
      end
   end )
   vn.transition( ... )
   table.insert( vn._states, vn.StateEnd.new() )
end

--[[--
Allows doing arbitrary animations.
   @tparam number seconds Seconds to perform the animation
   @tparam func func Function to call when progress is being done.
   @tparam func drawfunc Function to call when drawing.
   @tparam string|tab transition A CSS transition to use. Can be one of "ease", "ease-in", "ease-out", "ease-in-out", "linear", or a table defining the bezier curve parameters.
   @tparam func initfunc Function run once at the beginning.
   @tparam func drawoverride Function that overrides the drawing function for the VN.
--]]
function vn.animation( seconds, func, drawfunc, transition, initfunc, drawoverride )
   vn._checkstarted()
   table.insert( vn._states, vn.StateAnimation.new( seconds, func, drawfunc, transition, initfunc, drawoverride ) )
end

--[[--
Creates a transition state.
   @see vn.animation
   @tparam[opt="fade"] string name Name of the transition to use.
   @tparam[opt] number seconds Seconds for the transition to last. The default value depends on the type of transition.
   @tparam[opt="linear"] string transition The name of the CSS transition to use. See vn.animation.
--]]
function vn.transition( name, seconds, transition )
   vn._checkstarted()
   local shader
   shader, seconds, transition = transitions.get( name, seconds, transition )

   vn.animation( seconds,
      function (progress, dt) -- progress
         shader:send( "progress", progress )
         if shader.update then
            shader:update( dt )
         end
      end, nil, -- no draw function
      transition, function () -- init
         __needs_transition = false
         shader:send( "texprev", vn._prevscene )
         if shader:hasUniform( "u_r" ) then
            shader:send( "u_r", love_math.random() )
         end
      end, function () -- drawoverride
         local canvas = vn._curcanvas
         _draw_to_canvas( canvas )

         local oldshader = graphics.getShader()
         graphics.setShader( shader )
         graphics.setBlendMode( "alpha", "premultiplied" )
         canvas:draw( 0, 0 )
         graphics.setBlendMode( "alpha" )
         graphics.setShader( oldshader )
      end )
end

--[[--
Runs a specified function and continues execution.

   @tparam func func Function to run.
--]]
function vn.func( func )
   vn._checkstarted()
   local s = vn.State.new()
   s._init = function (self)
      func()
      _finish(self)
   end
   table.insert( vn._states, s )
end

--[[--
Plays a sound.

   @tparam Audio sfx Sound to play.
   @tparam[opt] table params Parameter values such as "effect" and "pitch". See audio package docs.
--]]
function vn.sfx( sfx, params )
   params = params or {}
   if vn._started then
      luaspfx.sfx( false, nil, sfx, params )
      return
   end
   local s = vn.State.new()
   s._init = function (state)
      local _sfx = sfx:clone()
      s._sfx = _sfx
      luaspfx.sfx( false, nil, _sfx, params )
      _finish(state)
   end
   table.insert( vn._states, s )
end
--[[--
Plays a money sound.
--]]
function vn.sfxMoney( params )
   return vn.sfx( vn._sfx.money, params )
end
--[[--
Plays a victory sound.
--]]
function vn.sfxVictory( params )
   return vn.sfx( vn._sfx.victory, params )
end
--[[--
Plays a bingo sound.
--]]
function vn.sfxBingo( params )
   return vn.sfx( vn._sfx.bingo, params )
end
--[[--
Plays an eerie sound.
--]]
function vn.sfxEerie( params )
    return vn.sfx( vn._sfx.eerie, params )
end

--[[--
Custom states. Only use if you know what you are doing.
--]]
function vn.custom()
   vn._checkstarted()
   local s = vn.State.new()
   table.insert( vn._states, s )
   return s
end

--[[--
Sets the shader to be used for post-processing the VN.

   @see love_shaders
   @tparam Shader shader Shader to use for post-processing or nil to disable.
--]]
function vn.setShader( shader )
   vn._postshader = shader
end

--[[--
Sets the background drawing function. Drawn behind the VN stuff.

   @tparam func drawfunc Function to call to draw the background or nil to disable.
--]]
function vn.setBackground( drawfunc )
   vn._draw_bg = drawfunc
end

--[[--
Sets the foreground drawing function. Drawn in front the VN stuff.

   @tparam func drawfunc Function to call to draw the foreground or nil to disable.
--]]
function vn.setForeground( drawfunc )
   vn._draw_fg = drawfunc
end

--[[--
Sets the middle drawing function. Drawn in right in front of the normal "namebox".

   @tparam func drawfunc Function to call to draw the middle or nil to disable.
--]]
function vn.setMiddle( drawfunc )
   vn._draw_mg = drawfunc
end

--[[--
Sets a custom update function. This gets run every frame.

   @tparam func updatefunc Update function to run every frame.
--]]
function vn.setUpdateFunc( updatefunc )
   vn._updatefunc = updatefunc
end

function vn._jump( label )
   for k,v in ipairs(vn._states) do
      if v:type() == "Label" and v.label == label then
         vn._state = k
         local s = vn._states[ vn._state ]
         s:init()
         vn._checkDone()
         return true
      end
   end
   error( string.format(_("vn: unable to find label '%s'"), label ) )
   return false
end

function vn._getCharacter( who )
   for k,v in ipairs(vn._characters) do
      if v.who == who then
         return v
      end
   end
   error( string.format(_("vn: character '%s' not found!"), who) )
end

function vn._checkDone()
   if vn.isDone() then return end

   local s = vn._states[ vn._state ]
   if s:isDone() then
      vn._state = vn._state+1
      if vn._state > #vn._states then
         return
      end
      s = vn._states[ vn._state ]
      s:init()
      vn._checkDone() -- Recursive :D
   end
end

--[[--
Checks to see if the VN is done running or not.
   @treturn bool true if it is done running, false otherwise
--]]
function vn.isDone()
   return vn._state > #vn._states
end

--[[--
Runs the visual novel environment.

<em>Note</em> You have to set up the states first.
<em>Note</em> This function doesn't return until the VN is done running.
--]]
function vn.run()
   if #vn._states == 0 then
      error( _("vn: run without any states") )
   end
   -- Check for duplicate labels
   if __debugging then
      local jumps = {}
      local labels = {}
      local menu_jumps = {}
      for k,s in ipairs( vn._states ) do
         if s._type=="Label" then
            table.insert( labels, s.label )
         elseif s._type=="Jump" then
            table.insert( jumps, s.label )
         elseif s._type=="Menu" then
            if type(s.items)=="table" then
               for i,m in ipairs(s.items) do
                  table.insert( menu_jumps, m[2] )
               end
            end
         end
      end
      table.sort( labels )
      for k,v in ipairs(labels) do
         if v==labels[k+1] then
            warn(fmt.f(_("vn: Duplicate label '{lbl}'!"),{lbl=v}))
         end
      end
      for k,v in ipairs(jumps) do
         if not inlist( labels, v ) then
            warn(fmt.f(_("vn: Jump to non-existent label '{lbl}'!"),{lbl=v}))
         end
      end
      for k,v in ipairs(menu_jumps) do
         if not inlist( labels, v ) then
            warn(fmt.f(_("vn: Manu has jump to non-existent label '{lbl}'!"),{lbl=v}))
         end
      end
   end
   love._vn = true
   love._vn_keyrepeat_check = true
   love.exec( 'scripts/vn' )
   love._vn = nil
   vn._started = false
   -- Destroy remaining lmusic stuff if necessary
   if vn._handle_music then
      lmusic.clear()
   end
end

--[[--
Clears the fundamental running variables. Run before starting a new VN instance.

<em>Note</em> Leaves customization to colours and positions untouched.
--]]
function vn.clear()
   local var = {
      "_state",
      "_bufcol",
      "_buffer",
      "_title",
      "_globalalpha",
      "_postshader",
      "_draw_fg",
      "_draw_bg",
      "_updatefunc",
   }

   _setdefaults()
   for k,v in ipairs(var) do
      vn[v] = vn._default[v]
   end
   -- Have to create new tables. Reset canvases in case the game was resized.
   vn._characters = {}
   vn._states = {}
end

--[[--
Fully resets the VN environment to default values.

<em>Note</em> This automatically does vn.clear() too.
--]]
function vn.reset()
   for k,v in pairs(vn._default) do
      vn[k] = v
   end
   vn.clear()
end

-- Default characters
vn._me = vn.Character.new( "You", { colour={1, 1, 1}, hidetitle=true } )
vn._na = vn.Character.new( "Narrator", { colour={0.5, 0.5, 0.5}, hidetitle=true } )

-- Set defaults
_setdefaults()
vn.reset()

return vn
