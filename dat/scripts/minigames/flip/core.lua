local lg = require "love.graphics"
--local la = require 'love.audio'
local love = require "love"
--local love_shaders   = require "love_shaders"
--local fmt = require "format"
local mg = {}

local colours = {
   --dark        = {0x00/0xFF, 0x00/0xFF, 0x00/0xFF},
   --bg          = {0x1C/0xFF, 0x30/0xFF, 0x4A/0xFF},
   --dark     = {0x04/0xFF, 0x6B/0xFF, 0x99/0xFF},
   --foobar      = {0xB3/0xFF, 0xEF/0xFF, 0xFF/0xFF},
   --text        = {0xFF/0xFF, 0xFF/0xFF, 0xFF/0xFF},
   --bad         = { 0.8, 0.2, 0.2 },
   dark     = {0x10/0xFF, 0x4C/0xFF, 0x71/0xFF},
   light    = {0x00/0xFF, 0xCF/0xFF, 0xFF/0xFF},
}

--local font, fonth
local board_size, selected_x, selected_y, board
--local bgshader,
local movekeys, alpha, done
function mg.load ()
   --local c = naev.cache()
   --local params = c.stringguess.params
   local params = {}
   board_size = params.board_size or 3

   -- Get movement keys
   movekeys = {
      --[[
      shoot    = string.lower( naev.keyGet("primary") ),
      left     = string.lower( naev.keyGet("left") ),
      right    = string.lower( naev.keyGet("right") ),
      accel    = string.lower( naev.keyGet("accel") ),
      reverse  = string.lower( naev.keyGet("reverse") ),
      --]]
      shoot    = "space",
      left     = "left",
      right    = "right",
      accel    = "up",
      reverse  = "down",
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
   board = {}
   for i=1,board_size do
      board[i] = {}
      for j=1,board_size do
         --board[i][j] = (rnd.rnd() > 0.5)
         board[i][j] = (love.math.random() > 0.5)
      end
   end

   selected_x = math.floor(board_size/2)
   selected_y = math.floor(board_size/2)

   alpha = 0
end


function mg.keypressed( key )
   if key == "escape" then
      done = true
   end

   -- handle keys
   if key == movekeys.shoot then
      -- flip
      for i=math.max(selected_x-1,1), math.min(selected_x+1,board_size) do
         board[i][selected_y] = not board[i][selected_y]
      end
      for j=math.max(selected_y-1,1), math.min(selected_y+1,board_size) do
         board[selected_x][j] = not board[selected_x][j]
      end
      board[selected_x][selected_y] = not board[selected_x][selected_y]
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

local function setcol( col )
   local r, g, b, a = unpack( col )
   a = a or 1
   lg.setColor( r, g, b, a*alpha )
end

function mg.draw ()
   local bx, by = 2, 2

   -- Draw selected
   setcol{ 1, 1, 1 }
   lg.rectangle( "fill", bx+40*(selected_x-1)-2, by+40*(selected_y-1)-2, 36, 36 )

   -- Draw grid
   local x, y = 0, 0
   for i=1,board_size do
      for j=1,board_size do
         local col
         if board[j][i] then
            col = colours.light
         else
            col = colours.dark
         end
         setcol( col )
         lg.rectangle( "fill", bx+x, by+y, 32, 32 )
         lg.print( string.format("%d,%d",i,j), bx+x, by+y )
         x = x + 40
      end
      x = 0
      y = y + 40
   end
end

function mg.update( dt )
   local spd = 4
   if done then
      alpha = alpha - spd*dt
      if alpha < 0 then
         return true
      end
   else
      alpha = math.min( 1, alpha + spd*dt )
   end
   --bgshader:update(dt)
   return false
end

return mg
