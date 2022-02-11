--[[
-- Playing dice game stuff
--]]
local lg = require 'love.graphics'
local la = require 'love.audio'
local love_math = require 'love.math'
local class = require 'class'

local diceio = {
   sound = {
      throw = {
         -- Added below
      },
   },
}
for i=1,3 do
   local f = string.format("snd/sounds/gambling/dieThrow%d.ogg",i)
   local s = la.newSource(f)
   table.insert( diceio.sound.throw, s )
end

--[[
-- Dice class
--
-- Dice are designed to be drawn at 50 x 50 px
--]]
diceio.Die = class.inheritsFrom( lg.Drawable )
function diceio.newDie()
   local c = diceio.Die.new()
   c:roll()
   return c
end
function diceio.Die:draw( x, y )
   lg.setColor( 1, 1, 1 )
   lg.rectangle( "fill", x, y, 50, 50 )
   lg.setColor( 0, 0, 0 )
   lg.rectangle( "line", x, y, 50, 50 )

   if self.value==1 then
      lg.circle("fill",x+25,y+25, 10 )
   elseif self.value==2 then
      lg.circle("fill",x+15,y+25, 5 )
      lg.circle("fill",x+35,y+25, 5 )
   elseif self.value==3 then
      lg.circle("fill",x+15,y+35, 5 )
      lg.circle("fill",x+25,y+25, 5 )
      lg.circle("fill",x+35,y+15, 5 )
   elseif self.value==4 then
      lg.circle("fill",x+15,y+15, 5 )
      lg.circle("fill",x+15,y+35, 5 )
      lg.circle("fill",x+35,y+15, 5 )
      lg.circle("fill",x+35,y+35, 5 )
   elseif self.value==5 then
      lg.circle("fill",x+15,y+15, 5 )
      lg.circle("fill",x+15,y+35, 5 )
      lg.circle("fill",x+35,y+15, 5 )
      lg.circle("fill",x+35,y+35, 5 )
      lg.circle("fill",x+25,y+25, 5 )
   elseif self.value==6 then
      lg.circle("fill",x+15,y+13, 5 )
      lg.circle("fill",x+35,y+13, 5 )
      lg.circle("fill",x+15,y+25, 5 )
      lg.circle("fill",x+35,y+25, 5 )
      lg.circle("fill",x+15,y+37, 5 )
      lg.circle("fill",x+35,y+37, 5 )
   end
end
function diceio.Die:roll()
   self.value = love_math.random(1,6)
   return self.value
end

return diceio
