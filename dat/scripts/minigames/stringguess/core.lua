local lg = require "love.graphics"
local la = require 'love.audio'
local love = require "love"
local love_shaders   = require "love_shaders"
local fmt = require "format"
local mg = {}

local colours = {
   dark        = {0x00/0xFF, 0x00/0xFF, 0x00/0xFF},
   bg          = {0x1C/0xFF, 0x30/0xFF, 0x4A/0xFF},
   highlight   = {0x04/0xFF, 0x6B/0xFF, 0x99/0xFF},
   mouseover   = {0x44/0xFF, 0xAB/0xFF, 0xD9/0xFF},
   ok          = {0x00/0xFF, 0xCF/0xFF, 0xFF/0xFF},
   --foobar      = {0xB3/0xFF, 0xEF/0xFF, 0xFF/0xFF},
   text        = {0xFF/0xFF, 0xFF/0xFF, 0xFF/0xFF},
   bad         = { 0.8, 0.2, 0.2 },
}

local function getpos( tbl, elm )
   for k,v in ipairs(tbl) do
      if v==elm then
         return k
      end
   end
   return false
end

local font, fonth, keyset, sol, guess, max_tries, tries, game, done, round, selected, attempts, alpha, bx, by, sol_length, mouseover
local bgshader, movekeys, standalone, headertext, headerfont
function mg.load ()
   local c = naev.cache()
   local params = c.stringguess.params
   standalone = c.stringguess.standalone
   max_tries = params.max_tries or 7
   keyset = params.keyset or {"B","E","K","N","O","V"} -- NAEV OK -> change A to B so it doesn't interfere with WASD keybinds
   sol_length = params.sol_length or 3
   headertext = params.header

   -- Get movement keys
   movekeys = {
      shoot = string.lower( naev.keyGet("primary") ),
      left = string.lower( naev.keyGet("left") ),
      right= string.lower( naev.keyGet("right") ),
      accel= string.lower( naev.keyGet("accel") ),
      reverse = string.lower( naev.keyGet("reverse") ),
   }

   -- Load audio if applicable
   if not mg.sfx then
      mg.sfx = {
         --goal     = la.newSource( 'snd/sounds/sokoban/goal.ogg' ),
         invalid  = la.newSource( 'snd/sounds/sokoban/invalid.ogg' ),
         level    = la.newSource( 'snd/sounds/sokoban/level.ogg' ),
      }
   end

   fonth = 16
   font = lg.newFont( fonth )

   local rndset = rnd.permutation( keyset )
   sol = {}
   for i=1,sol_length do
      table.insert( sol, rndset[i] )
   end

   guess = {}
   tries = max_tries
   game  = 0
   selected = 1
   round = true
   alpha = 0
   done = false
   mouseover = nil
   attempts = {}

   -- Window properties
   local lw, lh = love.window.getDesktopDimensions()
   lg.setBackgroundColour(0, 0, 0, 0)
   local ww, wh
   ww = 90 + 60*#sol+14 + 220 + 20 + 40*#sol+10+40
   wh = 25 + #keyset*40+10

   bx = (lw-ww)*0.5
   by = (lh-wh)*0.5
   if headertext then
      by = by - 40
   end
   headerfont = lg.newFont(24)
   bgshader = love_shaders.circuit()
end

local matches_exact, matches_fuzzy
local function finish_round ()
   matches_exact = 0
   matches_fuzzy = 0
   for i,v in ipairs(guess) do
      if v==sol[i] then
         matches_exact = matches_exact+1
      elseif inlist( sol, v ) then
         matches_fuzzy = matches_fuzzy+1
      end
   end

   tries = tries - 1
   if matches_exact == 3 then
      game = 1
      local c = naev.cache()
      c.stringguess.won = true
      mg.sfx.level:play()
   elseif tries <= 0 then
      game = -1
      mg.sfx.invalid:play()
   else
      table.insert( attempts, {
         guess = tcopy( guess ),
         matches_exact = matches_exact,
         matches_fuzzy = matches_fuzzy,
      } )
      mg.sfx.invalid:play()
   end
   round = false
end

local function inguess( k )
   for i=1,#sol do
      if guess[i] == k then
         return true
      end
   end
   return false
end

local function dopress( k )
   if inlist( keyset, k ) then
      if not round then
         selected = 2
         guess = { k }
         round = true
      elseif guess[selected]==k then
         for i=1,#sol do
            if not guess[i] then
               selected = i
               break
            end
         end
      elseif not inlist( guess, k ) then
         guess[selected] = k
         for i=1,#sol do
            if not guess[i] then
               selected = i
               break
            end
         end
         if #guess >= #sol then
            finish_round()
            return true
         end
      end
      return true
   end

   -- Next round if applicable
   if not round then
      guess = {}
      selected = 1
      round = true
      return true
   end
end

function mg.keypressed( key )
   if key == "escape" then
      done = true
   end

   if game ~= 0 then
      done = true
      return
   end

   -- Handle the press first before handling other keys
   if dopress( string.upper(key) ) then
      return
   end

   -- handle keys
   if key == movekeys.shoot then
      if #guess >= #sol then
         finish_round()
      end
   elseif key == movekeys.left then
      selected = math.max( selected-1, 1 )
   elseif key == movekeys.right then
      selected = math.min( selected+1, #sol )
   elseif key == movekeys.reverse then
      local p = getpos( keyset, guess[selected] ) or 0
      for i=p+1, #keyset do
         if not inguess( keyset[i] ) then
            guess[selected] = keyset[i]
            break
         end
      end
   elseif key == movekeys.accel then
      local p = getpos( keyset, guess[selected] ) or #sol+1
      for i=p-1,1,-1 do
         if not inguess( keyset[i] ) then
            guess[selected] = keyset[i]
            break
         end
      end
   end
end

local function mousepos( x, y )
   local offx, offy = 20, 25
   local s, b = 40, 10
   x = x - offx - b - bx
   y = y - offy - b - by

   -- First test x
   if x < 0 or x > s-b then
      return false
   end

   -- Figure out index
   local i = math.floor(y / s)+1
   y = y - (i-1)*s
   if y < 0 or y > s-b then
      return false
   end
   if i < 1 or i > #keyset then
      return false
   end
   return i
end

function mg.mousemoved( x, y )
   local i = mousepos( x, y )
   if i then
      mouseover = i
   else
      mouseover = nil
   end
end

function mg.mousepressed( x, y, _button )
   if game ~= 0 then
      done = true
      return
   end

   local i = mousepos( x, y )
   -- Apply click
   if i then
      dopress( keyset[i] )
   end
end

local function setcol( col )
   local r, g, b, a = table.unpack( col )
   a = a or 1
   lg.setColour( r, g, b, a*alpha )
end

local function drawglyph( g, f, x, y, w, h, col )
   col = col or colours.bg
   setcol( col )
   lg.rectangle( "fill", x, y, w, h )
   setcol( colours.text )
   local fh = f:getHeight()
   lg.printf( g, f, x, y+(h-fh)*0.5, w, "center" )
end

local function drawbox( x, y, w, h )
   setcol( colours.dark )
   lg.rectangle( "fill", x, y, w, h )
   setcol( colours.text )
   lg.rectangle( "line", x, y, w, h, 5 )
end

local function drawresult( exact, fuzzy, x, y, h )
   local str = ""
   for i=1,fuzzy do
      str = str .. "#o?#0"
   end
   for i=1,exact do
      str = str .. "#b!#0"
   end
   setcol{ 1, 0, 0 }
   lg.print( str, font, x, y+(h-fonth)*0.5 )
end

function mg.draw ()
   local x, y, s, b

   -- Fancy shader background
   local nw, nh = naev.gfx.dim()
   setcol{ 0.2, 0.2, 0.2, 0.85}
   lg.setShader( bgshader )
   love_shaders.img:draw( 0, 0, 0, nw, nh )
   lg.setShader()
   x, y = 0, 0

   if headertext then
      setcol{ 1, 1, 1 }
      lg.printf( headertext, headerfont, 0, by, nw, "center" )
      y = y + 60
   end

   -- Draw glyph bar
   s = 40
   b = 10
   setcol( colours.text )
   lg.printf( p_("stringguess", "Codes"), font, bx+x, by+y, s+40+b, "center" )
   y = y+25
   x = x+20
   drawbox( bx+x, by+y, s+b, s*#keyset+b )
   for k,v in ipairs(keyset) do
      local col
      if inlist( guess, v ) then
         col = colours.highlight
      elseif mouseover==k then
         col = colours.mouseover
      else
         col = nil
      end
      drawglyph( v, font, bx+x+b, by+y+k*s-s+b, s-b, s-b, col )
   end

   -- Draw the main interface
   x = 90
   y = 70
   s = 60
   b = 14
   setcol( colours.text )
   local txt = fmt.f(_("Input the code sequence ({tries} attempts left):"),{tries=tries})
   local txtw = font:getWidth( txt )
   local boxw = s*#sol+b
   local len = boxw + 220
   lg.print( txt, font, bx+x+(len-txtw)*0.5, by+y )

   x = x + (len-boxw)*0.5
   y = y+30
   drawbox( bx+x, by+y, boxw, s+b )
   for i=1,#sol do
      local v = guess[i] or ""
      local col
      if not round then
         if matches_exact >= #sol then
            col = colours.ok
         else
            col = colours.bad
         end
      else
         if i==selected then
            col = colours.highlight
         else
            col = nil
         end
      end
      drawglyph( v, font, bx+x+i*s-s+b, by+y+b, s-b, s-b, col )
   end

   x = 140
   y = y + 20
   txt = _([[#nHelp:#0
Guess the sequence of codes
#o?#0 correct code, wrong position
#b!#0 correct code and position]])
   lg.printf( txt, font, bx+x, by+y+s+b+10, len )

   -- Display attempts
   x = 90 + len + 20
   y = 0
   if headertext then
      y = y + 60
   end
   s = 40
   b = 10
   boxw = s*#sol+b+40
   setcol( colours.text )
   lg.printf( p_("stringguess", "Attempts"), font, bx+x, by+y, boxw, "center" )
   y = y+25
   x = x
   drawbox( bx+x, by+y, boxw, s*(max_tries-1)+b )
   for i,t in ipairs(attempts) do
      for j,v in ipairs(t.guess) do
         drawglyph( v, font, bx+x+j*s-s+b, by+y+b, s-b, s-b )
      end
      drawresult( t.matches_exact, t.matches_fuzzy, bx+x+boxw-40, by+y, s+b )
      y = y+s
   end
end

function mg.update( dt )
   local spd = 4
   if done then
      alpha = alpha - spd*dt
      if alpha < 0 then
         if standalone then
            love.event.quit()
         end
         return true
      end
   else
      alpha = math.min( 1, alpha + spd*dt )
   end
   bgshader:update(dt)
   return false
end

return mg
