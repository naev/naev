--[[
-- Playing card game images
--]]
local lg = require 'love.graphics'
local la = require 'love.audio'
local class = require 'class'

local cardio = {
   sound = {
      place = {
         -- Added below
      },
   },
}
for i=1,8 do
   local f = string.format("snd/sounds/gambling/cardSlide%d.ogg",i)
   local s = la.newSource(f)
   table.insert( cardio.sound.place, s )
end

--[[
-- Card superclass
--
-- Cards are designed to be drawn at 75 x 105 px
--]]
cardio.Card = class.inheritsFrom( lg.Drawable )
function cardio.newCard()
   local c = cardio.Card.new()
   return c
end
function cardio.Card.draw( _self, x, y )
   lg.setLineWidth(5)
   lg.rectangle( "line", x, y, 75, 105)
   lg.setLineWidth(1)
end



--[[
-- Deck superclass
--]]
cardio.Deck = class.inheritsFrom( nil )
function cardio.newDeck( cards )
   local d = cardio.Deck.new()
   d.cards = cards
   return d
end
function cardio.Deck:shuffle()
   self._dealt = 0
   --self._order = naev.rnd.permutation( #self.cards )

   self._order = {}
   for i = 1,#self.cards do
      local pos = math.random(1, #self._order+1)
      table.insert(self._order, pos, i)
   end
end
function cardio.Deck:draw()
   self._dealt = self._dealt+1
   if self._order == nil then
      self._order = {}
      for i = 1,#self.cards do
         table.insert( self._order, i )
      end
   end
   if self._dealt <= #self.cards then
      return self.cards[ self._order[ self._dealt ] ]
   end
end


--[[
-- Western Playing Cards
--]]
cardio.CardWestern = class.inheritsFrom( cardio.Card )
cardio.CardWestern.font = lg.newFont(32)
cardio.CardWestern.sfont = lg.newFont(16)
function cardio.newCardWestern( rank, suite )
   local c = cardio.CardWestern.new()
   c.rank = rank
   c.suite = suite
   return c
end
local function _torankstr( rank )
   if rank == 1 then
      return "A"
   elseif rank == 11 then
      return "J"
   elseif rank == 12 then
      return "Q"
   elseif rank == 13 then
      return "K"
   end
   return tostring(rank)
end
local function _tosuitestr( suite )
   if suite=="diamond" then
      return "♦"
   elseif suite=="heart" then
      return "♥"
   elseif suite=="spade" then
      return "♠"
   elseif suite=="club" then
      return "♣"
   else
      return "?"
   end
end
function cardio.CardWestern.drawBack( _self, x, y )
   -- background
   lg.setColour( 0.8, 0.8, 1 )
   lg.rectangle( "fill", x, y, 75, 105 )
   lg.setColour( 0, 0, 0 )
   lg.setLineWidth(5)
   lg.rectangle( "line", x, y, 75, 105)
   lg.setLineWidth(1)
   lg.setColour( 0.5, 0.5, 1 )
   lg.rectangle( "fill", x+6, y+5, 63, 95 )
end
function cardio.CardWestern:draw( x, y )
   -- background
   lg.setColour( 0.8, 0.8, 1 )
   lg.rectangle( "fill", x, y, 75, 105 )
   lg.setColour( 0, 0, 0 )
   lg.setLineWidth(5)
   lg.rectangle( "line", x, y, 75, 105)
   lg.setLineWidth(1)
   lg.setColour( 1, 1, 1 )
   lg.rectangle( "fill", x+6, y+5, 63, 95 )

   -- number
   local font = cardio.CardWestern.font
   local s = _torankstr(self.rank)
   local w = font:getWidth(s)
   if self.suite=="heart" or self.suite=="diamond" then
      lg.setColour( 1, 0.2, 0.2 )
   else
      lg.setColour( 0, 0, 0 )
   end
   lg.print( s, font, x+6+(63-w)/2, y+5+10 )
   s = _tosuitestr(self.suite)
   w = font:getWidth(s)
   lg.print( s, font, x+6+(63-w)/2, y+5+50 )
end
cardio.DeckWestern = class.inheritsFrom( cardio.Deck )
function cardio.newDeckWestern()
   local cards = {}
   for i,v in ipairs{ "heart", "spade", "club", "diamond" } do
      for n = 1,13 do
         local c = cardio.newCardWestern( n, v )
         table.insert( cards, c )
      end
   end
   local d = cardio.DeckWestern.new()
   d.cards = cards
   return d
end

-- TODO hanafuda
cardio.DeckHanafuda = class.inheritsFrom( cardio.Deck )

return cardio
