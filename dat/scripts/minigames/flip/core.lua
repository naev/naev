local lg = require "love.graphics"
--local la = require 'love.audio'
local love = require "love"
local love_shaders   = require "love_shaders"
--local fmt = require "format"
local mg = {}

local colours = {
   --bg        = {0x00/0xFF, 0x00/0xFF, 0x00/0xFF},
   dark          = {0x1C/0xFF, 0x30/0xFF, 0x4A/0xFF},
   --dark     = {0x04/0xFF, 0x6B/0xFF, 0x99/0xFF},
   --foobar      = {0xB3/0xFF, 0xEF/0xFF, 0xFF/0xFF},
   --text        = {0xFF/0xFF, 0xFF/0xFF, 0xFF/0xFF},
   --bad         = { 0.8, 0.2, 0.2 },
   --dark     = {0x10/0xFF, 0x4C/0xFF, 0x71/0xFF},
   light    = {0x00/0xFF, 0xCF/0xFF, 0xFF/0xFF},
}

local board_size, selected_x, selected_y, board--, headerfont, headertext
local movekeys, alpha, done, bx, by, bgshader, standalone
function mg.load ()
   local c = naev.cache()
   local params = c.flip.params
   standalone = c.flip.standalone
   board_size = params.board_size or 3

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

   local lw, lh = love.window.getDesktopDimensions()
   local s = 40*board_size-8
   bx = (lw-s)*0.5
   by = (lh-s)*0.5
   lg.setBackgroundColor(0, 0, 0, 0)
   --headerfont = lg.newFont(24)
   --headertext = params.header
   bgshader = love_shaders.circuit()
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
   local r, g, b, a = table.unpack( col )
   a = a or 1
   lg.setColor( r, g, b, a*alpha )
end

function mg.draw ()
   local nw, nh = naev.gfx.dim()
   setcol( {0.2,0.2,0.2,0.85} )
   lg.setShader( bgshader )
   love_shaders.img:draw( 0, 0, 0, nw, nh )
   lg.setShader()

   setcol{ 1, 1, 1 }
   --lg.printf( headertext, headerfont, bx+x, by+y, nw, "center" )

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
