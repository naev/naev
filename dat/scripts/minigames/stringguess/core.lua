local lg = require "love.graphics"
local le = require "love.event"
local la = require 'love.audio'
local love = require "love"
local fmt = require "format"
local mg = {}

local colours = {
   dark        = {0x00/0xFF, 0x00/0xFF, 0x00/0xFF},
   bg          = {0x1C/0xFF, 0x30/0xFF, 0x4A/0xFF},
   highlight   = {0x04/0xFF, 0x6B/0xFF, 0x99/0xFF},
   ok          = {0x00/0xFF, 0xCF/0xFF, 0xFF/0xFF},
   foobar      = {0xB3/0xFF, 0xEF/0xFF, 0xFF/0xFF},
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

local font, fonth, keyset, sol, guess, max_tries, tries, game, done, round, selected, attempts, alpha, bx, by
function mg.load ()
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

   keyset = {"A","E","K","N","O","V"} -- NAEV OK
   local rndset = rnd.permutation( keyset )
   sol = {}
   for i=1,3 do
      table.insert( sol, rndset[i] )
   end

   guess = {}
   max_tries = 7
   tries = max_tries
   game  = 0
   selected = 1
   round = true
   alpha = 0
   done = false
   attempts = {}

   -- Window properties
   local lw, lh = love.window.getDesktopDimensions()
   lg.setBackgroundColor(0, 0, 0, 0)
   local ww, wh
   ww = 90 + 60*#sol+14 +  40 + 20 + 40*#sol+10+40
   wh = 25 + #keyset*40+10

   bx = (lw-ww)*0.5
   by = (lh-wh)*0.5
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

function mg.keypressed( key )
   if key == "escape" then
      done = true
   end

   if game ~= 0 then
      return
   end

   local k = string.upper(key)
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
            return
         end
      end
   end

   -- Next round if applicable
   if not round then
      guess = {}
      selected = 1
      round = true
      return
   end

   -- handle keys
   if key == "space" then
      if #guess >= #sol then
         finish_round()
      end
   elseif key == "left" then
      selected = math.max( selected-1, 1 )
   elseif key == "right" then
      selected = math.min( selected+1, #sol )
   elseif key == "down" then
      local p = getpos( keyset, guess[selected] ) or 0
      for i=p+1, #keyset do
         if not inguess( keyset[i] ) then
            guess[selected] = keyset[i]
            break
         end
      end
   elseif key == "up" then
      local p = getpos( keyset, guess[selected] ) or #sol+1
      for i=p-1,1,-1 do
         if not inguess( keyset[i] ) then
            guess[selected] = keyset[i]
            break
         end
      end
   end
end

local function setcol( rgb, a )
   rgb = rgb or {1, 1, 1}
   a = a or 1
   rgb[4] = alpha * a
   lg.setColor( rgb )
end

local function drawglyph( g, f, x, y, w, h, col )
   col = col or colours.bg
   setcol( col )
   lg.rectangle( "fill", x, y, w, h )
   setcol( colours.text )
   local fh = f:getHeight()
   lg.printf( g, f, x, y+(h-fh)*0.5, w, "center" )
end

local function drawresult( exact, fuzzy, x, y, h )
   local str = ""
   for i=1,fuzzy do
      str = str .. "?"
   end
   for i=1,exact do
      str = str .. "!"
   end
   setcol{ 1, 0, 0 }
   lg.print( str, font, x, y+(h-fonth)*0.5 )
end

function mg.draw ()
   local x, y, s, b

   -- Draw glyph bar
   x = 0
   y = 0
   s = 40
   b = 10
   setcol( colours.text )
   lg.printf( "Codes", font, bx+x, by+y, s+40+b, "center" )
   y = y+25
   x = x+20
   lg.rectangle( "line", bx+x, by+y, s+b, s*#keyset+b )
   for k,v in ipairs(keyset) do
      local col
      if inlist( guess, v ) then
         col = colours.highlight
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
   local txt = fmt.f(_("Input the code sequence ({tries} tries left):"),{tries=tries})
   local txtw = font:getWidth( txt )
   local boxw = s*#sol+b
   local len = boxw + 160
   lg.print( txt, font, bx+x+(len-txtw)*0.5, by+y )

   x = x + (len-boxw)*0.5
   y = y+30
   setcol( colours.text )
   lg.rectangle( "line", bx+x, by+y, boxw, s+b )
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

   x = 90
   y = y + 20
   txt = _([[Help:
Guess the sequence of codes
? correct code, wrong position
! correct code and position]])
   lg.printf( txt, font, bx+x, by+y+s+b+10, len )

   -- Display attempts
   x = 90 + len + 20
   y = 0
   s = 40
   b = 10
   boxw = s*#sol+b+40
   setcol( colours.text )
   lg.printf( "Attempts", font, bx+x, by+y, boxw, "center" )
   y = y+25
   x = x
   setcol( colours.text )
   lg.rectangle( "line", bx+x, by+y, boxw, s*(max_tries-1)+b )
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
         le.quit()
      end
   else
      alpha = math.min( 1, alpha + spd*dt )
   end
   return false -- true to finish
end

return mg
