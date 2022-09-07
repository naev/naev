local lg = require "love.graphics"
--local la = require 'love.audio'
local love = require "love"
local love_shaders   = require "love_shaders"
--local fmt = require "format"
local mg = {}

local colours = {
   --bg        = {0x00/0xFF, 0x00/0xFF, 0x00/0xFF},
   --dark     = {0x04/0xFF, 0x6B/0xFF, 0x99/0xFF},
   --text        = {0xFF/0xFF, 0xFF/0xFF, 0xFF/0xFF},
   --bad         = { 0.8, 0.2, 0.2 },
   --dark     = {0x10/0xFF, 0x4C/0xFF, 0x71/0xFF},
   dark     = {0x1C/0xFF, 0x30/0xFF, 0x4A/0xFF},
   light    = {0x00/0xFF, 0xCF/0xFF, 0xFF/0xFF},
   won      = {0xB3/0xFF, 0xEF/0xFF, 0xFF/0xFF},
}

local board_size, selected_x, selected_y, board, headerfont, headertext
local movekeys, alpha, done, bx, by, bgshader, standalone, game_won

local function flip( x, y )
   -- flip
   for i=math.max(x-1,1), math.min(x+1,board_size) do
   board[i][y] = not board[i][y]
   end
   for j=math.max(y-1,1), math.min(y+1,board_size) do
   board[x][j] = not board[x][j]
   end
   board[x][y] = not board[x][y]
end

local function checkwon ()
   for i=1,board_size do
      for j=1,board_size do
         if not board[i][j] then
            return false
         end
      end
   end
   return true
end

function mg.load ()
   local c = naev.cache()
   local params = c.flip.params
   standalone = c.flip.standalone
   board_size = params.board_size or 4
   headertext = params.header

   -- Get movement keys
   movekeys = {
      shoot    = string.lower( naev.keyGet("primary") ),
      left     = string.lower( naev.keyGet("left") ),
      right    = string.lower( naev.keyGet("right") ),
      accel    = string.lower( naev.keyGet("accel") ),
      reverse  = string.lower( naev.keyGet("reverse") ),
   }

   -- Load audio if applicable
   if not mg.sfx then
      mg.sfx = {
         --goal     = la.newSource( 'snd/sounds/sokoban/goal.ogg' ),
         --invalid  = la.newSource( 'snd/sounds/sokoban/invalid.ogg' ),
         --level    = la.newSource( 'snd/sounds/sokoban/level.ogg' ),
      }
   end

   -- Initialize board
   local poslist = {}
   board = {}
   for i=1,board_size do
      board[i] = {}
      for j=1,board_size do
         --board[i][j] = (love.math.random() > 0.5)
         board[i][j] = true
         table.insert( poslist, {i, j} )
      end
   end
   repeat
      local n = board_size*board_size
      n = rnd.rnd( math.floor(n*0.4), math.floor(n*0.6) )
      poslist = rnd.permutation( poslist )
      for i=1,n do
         local p = poslist[i]
         flip( p[1], p[2] )
      end
   until not checkwon()

   selected_x = math.ceil(board_size/2)
   selected_y = math.ceil(board_size/2)

   local lw, lh = love.window.getDesktopDimensions()
   local s = 40*board_size-8
   bx = (lw-s)*0.5
   by = (lh-s)*0.5
   if headertext then
      by = by - 40
   end
   lg.setBackgroundColor(0, 0, 0, 0)
   lg.setNewFont( 16 )
   headerfont = lg.newFont(24)
   bgshader = love_shaders.circuit()
   alpha = 0
   game_won = false
end

function mg.keypressed( key )
   if key == "escape" then
      done = true
      return
   end

   if game_won then
      done = true
      return
   end

   -- handle keys
   if key == movekeys.shoot then
      flip( selected_x, selected_y )
      if checkwon() then
         local c = naev.cache()
         c.flip.won = true
         game_won = true
      end
   elseif key == movekeys.left then
      selected_x = math.max( selected_x-1, 1 )
   elseif key == movekeys.right then
      selected_x = math.min( selected_x+1, board_size )
   elseif key == movekeys.reverse then
      selected_y = math.min( selected_y+1, board_size )
   elseif key == movekeys.accel then
      selected_y = math.max( selected_y-1, 1 )
   end
end

function mg.mousepressed( mx, my, _button )
   if game_won then
      done = true
      return
   end

   -- identify the clicked cell (if any)
   mx, my = mx - bx, my - by
   if headertext then
      my = my - 45
   end
   if mx < 0 or (mx % 40) > 32 or mx > 40*board_size or
      my < 0 or (my % 40) > 32 or my > 40*board_size then
      return
   end
   selected_x = math.floor( mx / 40 ) + 1
   selected_y = math.floor( my / 40 ) + 1
   flip( selected_x, selected_y )
   if checkwon() then
      local c = naev.cache()
      c.flip.won = true
      game_won = true
   end
end

local function setcol( col )
   local r, g, b, a = table.unpack( col )
   a = a or 1
   lg.setColor( r, g, b, a*alpha )
end

function mg.draw ()
   local x, y = 0, 0
   local nw, nh = naev.gfx.dim()
   setcol( {0.2,0.2,0.2,0.85} )
   lg.setShader( bgshader )
   love_shaders.img:draw( 0, 0, 0, nw, nh )
   lg.setShader()

   if headertext then
      setcol{ 1, 1, 1 }
      lg.printf( headertext, headerfont, 0, by, nw, "center" )
      y = y + 45
   end

   -- Draw selected
   setcol{ 0, 0, 0 }
   lg.rectangle( "fill", bx+x-4, by+y-4, 40*board_size, 40*board_size )
   if not game_won then
      setcol{ 1, 1, 1 }
      lg.rectangle( "fill", bx+40*(selected_x-1)-2+x, by+40*(selected_y-1)-2+y, 36, 36 )
   end

   -- Draw grid
   for i=1,board_size do
      for j=1,board_size do
         local col
         if game_won then
            col = colours.won
         elseif board[j][i] then
            col = colours.light
         else
            col = colours.dark
         end
         setcol( col )
         lg.rectangle( "fill", bx+x, by+y, 32, 32 )

         x = x + 40
      end
      x = 0
      y = y + 40
   end

   y = y + 10
   if game_won then
      setcol( colours.won )
      lg.printf( _("Success!\nPress any key or click to continue…"), 0, by+y, nw, "center" )
   else
      setcol{ 1, 1, 1 }
      lg.printf( "#n".._("Flip the blocks until they are all lit"), 0, by+y, nw, "center" )
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
