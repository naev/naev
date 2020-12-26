--[[
-- Visual Novel API for Naev
--
-- Based on Love2D API
--]]
local utf8 = require 'utf8'
local love = require 'love'
local graphics = require 'love.graphics'
local window = require 'love.window'

local vn = {
   speed = 0.04,
   color = {1,1,1},

   -- Internal usage
   _default = {
      _characters = {},
      _states = {},
      _state = 0,
      _bufcol = { 1, 1, 1 },
      _buffer = "",
      _title = nil,
      _alpha = 1,
   }
}
-- Drawing
local lw, lh = window.getDesktopDimensions()
vn._default.textbox_font = graphics.newFont(16)
vn._default.textbox_w = 800
vn._default.textbox_h = 200
vn._default.textbox_x = (lw-vn._default.textbox_w)/2
vn._default.textbox_y = lh-230
vn._default.textbox_bg = {0, 0, 0, 1}
vn._default.namebox_font = graphics.newFont(20)
vn._default.namebox_w = -1 -- Autosize
vn._default.namebox_h = 20*2+vn._default.namebox_font:getHeight()
vn._default.namebox_x = vn._default.textbox_x
vn._default.namebox_y = vn._default.textbox_y - vn._default.namebox_h - 20
vn._default.namebox_bg = vn._default.textbox_bg


function vn._checkstarted()
   if vn._started then
      error( _("vn: can't modify states when running") )
   end
end

local function _set_col( col )
   local a = col[4] or 1
   graphics.setColor( col[1], col[2], col[3], a*vn._alpha )
end

local function _draw_bg( x, y, w, h, col, border_col )
   col = col or {0, 0, 0, 1}
   border_col = border_col or {0.5, 0.5, 0.5, 1}
   _set_col( border_col )
   graphics.rectangle( "fill", x, y, w, h )
   _set_col( col )
   graphics.rectangle( "fill", x+2, y+2, w-4, h-4 )
end


--[[
-- @brief Main drawing function.
--]]
function vn.draw()
   -- Draw characters
   -- TODO handle multiple characters and characters appearing in the middle
   -- of conversations
   for k,c in ipairs( vn._characters ) do
      if c.image ~= nil then
         local w, h = c.image:getDimensions()
         local lw, lh = love.graphics.getDimensions()
         local mw, mh = vn.textbox_w, vn.textbox_y
         local scale = math.min( mw/w, mh/h )
         local col
         if c.talking then
            col = { 1, 1, 1 }
         else
            col = { 0.8, 0.8, 0.8 }
         end
         _set_col( col )
         local x = (lw-w*scale)/2
         local y = mh-scale*h
         graphics.draw( c.image, x, y, 0, scale, scale )
      end
   end

   -- Textbox
   local font = vn.textbox_font
   -- Draw background
   local x, y, w, h = vn.textbox_x, vn.textbox_y, vn.textbox_w, vn.textbox_h
   local bw = 20
   local bh = 20
   _draw_bg( x, y, w, h, vn.textbox_bg )
   -- Draw text
   _set_col( vn._bufcol )
   graphics.printf( vn._buffer, font, x+bw, y+bw, vn.textbox_w-2*bw )

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
      _draw_bg( x, y, w, h, vn.namebox_bg )
      -- Draw text
      _set_col( vn._bufcol )
      graphics.print( vn._title, font, x+bw, y+bh )
   end

   -- Draw if necessary
   if vn.isDone() then return end
   local s = vn._states[ vn._state ]
   s:draw()
end


--[[
-- @brief Main updating function.
--    @param dt Update tick.
--]]
function vn.update(dt)
   -- Out of states
   if vn._state > #vn._states then
      love.event.quit()
      return
   end

   if vn.isDone() then return end

   if vn._state < 0 then
      vn._state = 1
   end

   local s = vn._states[ vn._state ]
   s:update( dt )
end


--[[
-- @brief Key press handler.
--    @param key Name of the key pressed.
--]]
function vn.keypressed( key )
   if key=="escape" then
      love.event.quit()
      return
   end

   if vn.isDone() then return end
   local s = vn._states[ vn._state ]
   s:keypressed( key )
end


--[[
-- @brief Mouse press handler.
--    @param mx X position of the click.
--    @param my Y position of the click.
--    @param button Button that was pressed.
--]]
function vn.mousepressed( mx, my, button )
   if vn.isDone() then return end
   local s = vn._states[ vn._state ]
   s:mousepressed( mx, my, button )
end


-- Helpers
--[[
-- @brief Makes the player say something.
--]]
function vn.me( what, nowait ) vn.say( "me", what, nowait ) end
--[[
-- @brief Makes the narrator say something.
--]]
function vn.na( what, nowait ) vn.say( "narrator", what, nowait ) end

--[[
-- State
--]]
vn.State = {}
vn.State_mt = { __index = vn.State }
local function _dummy() end
local function _finish(self) self.done = true end
local function _inbox( mx, my, x, y, w, h )
   return (mx>=x and mx<= mx+w and my>=y and my<=y+h)
end
function vn.State.new()
   local s = {}
   setmetatable( s, vn.State_mt )
   s._type = "State"
   s._init = _dummy
   s._draw = _dummy
   s._update = _dummy
   s._mousepressed = _dummy
   s._keypressed = _dummy
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
   self:_mousepressed( mx, my, button )
   vn._checkDone()
end
function vn.State:keypressed( key )
   self:_keypressed( key )
   vn._checkDone()
end
function vn.State:isDone() return self.done end
--[[
-- Scene
--]]
vn.StateScene ={}
function vn.StateScene.new( background )
   local s = vn.State.new()
   s._init = vn.StateScene._init
   s._type = "Scene"
   return s
end
function vn.StateScene:_init()
   -- Reset characters
   vn._characters = {
      vn._me,
      vn._na
   }
   _finish(self)
end
--[[
-- Character
--]]
vn.StateCharacter ={}
function vn.StateCharacter.new( character )
   local s = vn.State.new()
   s._init = vn.StateCharacter._init
   s._type = "Character"
   s.character = character
   return s
end
function vn.StateCharacter:_init()
   table.insert( vn._characters, self.character )
   _finish(self)
end
--[[
-- Say
--]]
vn.StateSay = {}
function vn.StateSay.new( who, what )
   local s = vn.State.new()
   s._init = vn.StateSay._init
   s._update = vn.StateSay._update
   s._mousepressed = vn.StateSay._finish
   s._keypressed = vn.StateSay._finish
   s._type = "Say"
   s.who = who
   s.what = what
   return s
end
function vn.StateSay:_init()
   if type(self.what)=="function" then
      self._textbuf = self.what()
   else
      self._textbuf = self.what
   end
   self._timer = vn.speed
   self._len = utf8.len( self._textbuf )
   self._pos = utf8.next( self._textbuf )
   self._text = ""
   local c = vn._getCharacter( self.who )
   vn._bufcol = c.color or vn._default._bufcol
   vn._buffer = self._text
   if c.hidetitle then
      vn._title = nil
   else
      vn._title = c.who
   end
   -- Reset talking
   for k,v in ipairs( vn._characters ) do
      v.talking = false
   end
   c.talking = true
end
function vn.StateSay:_update( dt )
   self._timer = self._timer - dt
   while self._timer < 0 do
      -- No more characters left -> done!
      if utf8.len(self._text) == self._len then
         _finish( self )
         return
      end
      self._pos = utf8.next( self._textbuf, self._pos )
      self._text = string.sub( self._textbuf, 1, self._pos )
      self._timer = self._timer + vn.speed
      vn._buffer = self._text
   end
end
function vn.StateSay:_finish()
   self._text = self._textbuf
   vn._buffer = self._text
   _finish( self )
end
--[[
-- Wait
--]]
vn.StateWait ={}
function vn.StateWait.new()
   local s = vn.State.new()
   s._init = vn.StateWait._init
   s._draw = vn.StateWait._draw
   s._mousepressed = _finish
   s._keypressed = _finish
   s._type = "Wait"
   return s
end
function vn.StateWait:_init()
   local x, y, w, h = vn.textbox_x, vn.textbox_y, vn.textbox_w, vn.textbox_h
   local font = vn.namebox_font
   self._font = font
   self._text = ">"
   self._w = font:getWidth( self._text )
   self._h = font:getHeight()
   self._x = x+w-10-self._w
   self._y = y+h-10-self._h
end
function vn.StateWait:_draw()
   _set_col( vn._bufcol )
   graphics.print( self._text, self._font, self._x, self._y )
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
   s.handler = handler
   s._choose = vn.StateMenu._choose
   return s
end
function vn.StateMenu:_init()
   -- Set up the graphics stuff
   local font = vn.namebox_font
   -- Border information
   local tb = 15 -- Inner border around text
   local b = 15 -- Outter border
   self._tb = tb
   self._b = b
   -- Get longest line
   local w = 0
   local h = 0
   self._elem = {}
   for k,v in ipairs(self.items) do
      local text = string.format("%d. %s", k, v[1])
      local sw, wrapped = font:getWrap( text, 900 )
      sw = sw + 2*tb
      local sh =  2*tb + font:getHeight() + font:getLineHeight() * (#wrapped-1)
      local elem = { text, 0, h, sw, sh }
      if w < sw then
         w = sw
      end
      h = h + sh + b
      self._elem[k] = elem
   end
   self._w = math.max( w, 300 )
   self._h = h-b
   self._x = (love.w-self._w)/2
   self._y = (love.h-self._h)/2-100
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
      local text, x, y, w, h = unpack(v)
      local col
      if _inbox( mx, my, gx+x, gy+y, w, h ) then
         col = {0.5, 0.5, 0.5}
      else
         col = {0.2, 0.2, 0.2}
      end
      _set_col( col )
      graphics.rectangle( "fill", gx+x, gy+y, w, h )
      _set_col( {1, 1, 1} )
      graphics.print( text, font, gx+x+tb, gy+y+tb )
   end
end
function vn.StateMenu:_mousepressed( mx, my, button )
   if button ~= 1 then
      return
   end
   local gx, gy = self._x, self._y
   local b = self._tb
   for k,v in ipairs(self._elem) do
      local text, x, y, w, h = unpack(v)
      if _inbox( mx, my, gx+x-b, gy+y-b, w+2*b, h+2*b ) then
         self:_choose(k)
         return
      end
   end
end
function vn.StateMenu:_keypressed( key )
   local n = tonumber(key)
   if n == nil then return end
   if n==0 then n = n + 10 end
   if n > #self.items then return end
   self:_choose(n)
end
function vn.StateMenu:_choose( n )
   self.handler( self.items[n][2] )
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
   s._init = _finish
   s._type = "Start"
   return s
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
function vn.StateEnd:_init()
   vn._state = #vn._states+1
end
--[[
-- Animation
--]]
vn.StateAnimation = {}
function vn.StateAnimation.new( seconds, func )
   local s = vn.State.new()
   s._init = vn.StateAnimation._init
   s._update = vn.StateAnimation._update
   s._type = "Animation"
   s._seconds = seconds
   s._func = func
   return s
end
function vn.StateAnimation:_init()
   self._accum = 0
   self._func( 0 )
end
function vn.StateAnimation:_update(dt)
   self._accum = self._accum + dt
   if self._accum > self._seconds then
      self._accum = self._seconds
      _finish(self)
   end
   self._func( self._accum / self._seconds )
end


--[[
-- Character
--]]
vn.Character = {}
--[[
-- @brief Makes a player say something.
--]]
function vn.Character:say( what, nowait ) return vn.say( self.who, what, nowait ) end
vn.Character_mt = { __index = vn.Character, __call = vn.Character.say }
--[[
-- @brief Creates a new character without adding it to the VN.
-- @note The character can be added with vn.newCharacter.
--    @param who Name of the character to add.
--    @param params Parameter table.
--    @return New character.
--]]
function vn.Character.new( who, params )
   local c = {}
   setmetatable( c, vn.Character_mt )
   params = params or {}
   c.who = who
   c.color = params.color or vn._default.color
   local image = params.image
   if image ~= nil then
      if type(image)=='string' or image:type()=="ImageData" then
         image = graphics.newImage( image )
      end
   end
   c.image = image
   c.hidetitle = params.hidetitle
   c.params = params
   return c
end
--[[
-- @brief Creates a new character.
--    @param who Name (or previously created character) to add.
--    @param params Parameter table.
--    @return New character.
--]]
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

--[[
-- @brief Starts a new scene.
--    @param background Background image to set or none if nil.
--]]
function vn.scene( background )
   vn._checkstarted()
   table.insert( vn._states, vn.StateScene.new( background ) )
end

--[[
-- @brief Has a character say something.
--
-- @note "me" and "narrator" are specila meta-characters.
--
--    @param who The name of the character that is saying something.
--    @param what What the character is saying.
--    @param nowait Whether or not to introduce a wait or just skip to the next text right away (defaults to false).
--]]
function vn.say( who, what, nowait )
   vn._checkstarted()
   table.insert( vn._states, vn.StateSay.new( who, what ) )
   if not nowait then
      table.insert( vn._states, vn.StateWait.new() )
   end
end

--[[
-- @brief Opens a menu the player can select from.
--    @param items Table of items to select from, they should be of the form "{text, label}" where "text" is what is displayed and "label" is what is passed to the handler.
--    @param handler Function to handle what happens when an item is selecetdd. Defaults to vn.jump.
--]]
function vn.menu( items, handler )
   vn._checkstarted()
   handler = handler or vn.jump
   table.insert( vn._states, vn.StateMenu.new( items, handler ) )
end

--[[
-- @brief Inserts a label. This does nothing but serve as a reference for vn.jump
--    @param label Name of the label to insert.
--]]
function vn.label( label )
   vn._checkstarted()
   table.insert( vn._states, vn.StateLabel.new( label ) )
end

--[[
-- @brief Inserts a jump. This skips to a certain label.
--    @param label Name of the label to jump to.
--]]
function vn.jump( label )
   if vn._started then
      vn._jump( label )
   end
   table.insert( vn._states, vn.StateJump.new( label ) )
end

--[[
-- @brief Finishes the VN.
--]]
function vn.done()
   vn._checkstarted()
   table.insert( vn._states, vn.StateEnd.new() )
end

--[[
-- @brief Inserts a fade.
--    @param seconds Number of seconds to fade.
--    @param fadestart Starting fade opacity.
--    @param fadeend Ending fade opacity.
--]]
function vn.fade( seconds, fadestart, fadeend )
   seconds = seconds or 0.2
   vn._checkstarted()
   local func = function( alpha )
      vn._alpha = fadestart + (fadeend-fadestart)*alpha
   end
   table.insert( vn._states, vn.StateAnimation.new( seconds, func ) )
end
--[[
-- @brief Wrapper to fade in.
--    @param seconds Number of seconds to fully fade in.
--]]
function vn.fadein( seconds ) vn.fade( seconds, 0, 1 ) end
--[[
-- @brief Wrapper to fade out.
--    @param seconds Number of seconds to fully fade out.
--]]
function vn.fadeout( seconds ) vn.fade( seconds, 1, 0 ) end
function vn.animation( seconds, func )
   vn._checkstarted()
   table.insert( vn._states, vn.StateAnimation.new( seconds, func ) )
end

--[[
-- @brief Custom states. Only use if you know what you are doing.
--]]
function vn.custom()
   local s = vn.State.new()
   table.insert( vn._states, s )
   return s
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


--[[
-- @brief Checks to see if the VN is done running or not.
--    @return true if it is done running, false otherwise
--]]
function vn.isDone()
   return vn._state > #vn._states
end


--[[
-- @brief Runs the visual novel environment.
--
-- @note You have to set up the states first.
-- @note This function doesn't return until the VN is done running.
--]]
function vn.run()
   if #vn._states == 0 then
      error( _("vn: run without any states") )
   end
   love._vn = true
   love.exec( 'scripts/vn' )
   love._vn = nil
   vn._started = false
end

--[[
-- @brief Clears the fundamental running variables. Run before starting a new VN instance.
--
-- @note Leaves customization to colors and positions untouched.
--]]
function vn.clear()
   local var = {
      "_state",
      "_bufcol",
      "_buffer",
      "_title",
      "_alpha"
   }
   for k,v in ipairs(var) do
      vn[v] = vn._default[v]
   end
   -- Have to create new tables
   vn._characters = {}
   vn._states = {}
end

--[[
-- @brief Fully resets the VN environment to default values.
--
-- @note This automatically does vn.clear() too.
--]]
function vn.reset()
   for k,v in pairs(vn._default) do
      vn[k] = v
   end
   vn.clear()
end

-- Default characters
vn._me = vn.Character.new( "me", { color={1, 1, 1}, hidetitle=true } )
vn._na = vn.Character.new( "narrator", { color={0.5, 0.5, 0.5}, hidetitle=true } )

-- Set defaults
vn.reset()

return vn
