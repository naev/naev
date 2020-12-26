local lg = require 'love.graphics'

local bj = {} -- too lazy to write blackjack over and over

function bj.init( w, h, donefunc )
   local cardio = require 'minigames.cardio'
   bj.deck = cardio.newDeckWestern( false )
   bj.font = lg.newFont(16)
   -- Scaling factor
   h = h - 3*bj.font:getHeight()
   bj.scale = math.min( 1, h / 300 )
   bj.donefunc = donefunc
end

local function _total( cards )
   local total = 0
   local aces = 0
   for k,c in ipairs(cards) do
      total = total + math.min( c.rank, 10 )
      if c.rank==1 then
         aces = aces + 1
      end
   end
   while aces > 0 and total <= 11 do
      total = total + 10
      aces = aces - 1
   end
   return total, aces
end

local function _done( status )
   bj.done = true
   bj.status = status
   if bj.donefunc then
      bj.donefunc( status )
   end
end

function bj.deal()
   bj.status = 0
   bj.done = false
   bj.dealer = {}
   bj.player = {}
   bj.deck:shuffle()
   table.insert( bj.player, bj.deck:draw() )
   table.insert( bj.player, bj.deck:draw() )
   table.insert( bj.dealer, bj.deck:draw() )
   table.insert( bj.dealer, bj.deck:draw() )
   -- Special case for blackjacks
   local p = _total(bj.player)
   local d = _total(bj.dealer)
   if p==21 and d==21 then
      __done(0)
   elseif p==21 then
      __done(1)
   elseif d==21 then
      __done(-1)
   end
end

function bj.hit()
   table.insert( bj.player, bj.deck:draw() )
   -- Check if player lost
   local p = _total(bj.player)
   if p > 21 then
      _done(-1)
   else
      -- See if dealer grabs another card
      local d, da = _total(bj.dealer)
      if d < 17 or (d==17 and da>0) then
         table.insert( bj.dealer, bj.deck:draw() )
      end
      -- Check if dealer lost
      d = _total(bj.dealer)
      if d > 21 then
         _done(1)
      end
   end
end

function bj.stay()
   local p = _total(bj.player)
   local d = _total(bj.dealer)
   while d<p do
      table.insert( bj.dealer, bj.deck:draw() )
      p = _total(bj.player)
      d = _total(bj.dealer)
   end
   if d>21 then
      _done(1)
   elseif d>p then
      _done(-1)
   elseif p>d then
      _done(1)
   end
end

function bj.ai()
   local total, aces = _total(bj.dealer)
   if total < 17 or (total==17 and aces>0) then
      table.insert( bj.dealer, bj.deck:draw() )
      return true
   end
   return false
end

local function _drawhand( x, y, cards, hidefirst )
   lg.push()
   lg.translate( x, y )
   lg.scale( bj.scale )
   x = 0
   y = 0
   if #cards > 4 then
      x = -(#cards-4)*50
   end
   for k,v in ipairs(cards) do
      if k==1 and hidefirst then
         v:drawBack( x, y )
      else
         v:draw( x, y )
      end
      x = x + 100
   end
   lg.pop()
end

function bj.draw( bx, by, bw, bh)
   local sep = 25
   local w = 75
   local h = 105
   local rs = bx + (bw-bj.scale*(w*4+sep*3))/2
   local x = rs
   local y = by + 10
   -- Dealer
   lg.setColor( 1, 1, 1 )
   local tdealer = _total(bj.dealer)
   local str
   if not bj.done then
      str = "?"
   else
      str = tostring(_total(bj.dealer))
   end
   lg.print( string.format(_("Dealer: %s"),str), bj.font, x, y )
   y = y + bj.font:getHeight()+10
   _drawhand( x, y, bj.dealer, not bj.done )

   -- Player
   local tplayer = _total(bj.player)
   x = rs
   y = y + (h + sep)*bj.scale
   lg.setColor( 1, 1, 1 )
   lg.print( string.format(_("Player: %d"),tplayer), bj.font, x, y )
   y = y + bj.font:getHeight()+10
   _drawhand( x, y, bj.player )

   -- Print status
   local p = _total(bj.player)
   local d = _total(bj.dealer)
   local msg = nil
   if bj.status < 0 then
      msg = _("You lost!")
      if #bj.dealer == 2 and d==21 then
         msg = string.format(_("Blackjack! %s"), msg)
      end
   elseif bj.status > 0 then
      msg = _("You won!")
      if #bj.player == 2 and p==21 then
         msg = string.format(_("Blackjack! %s"), msg)
      end
   elseif bj.done then
      if #bj.player==2 and #bj.dealer==2 and d==21 and p==21 then
         msg = _("Double Blackjack! Push!")
      else
         msg = _("Push!")
      end
   end
   if msg ~= nil then
      x = rs
      y = y + h + 20
      lg.setColor( 1, 1, 1 )
      lg.print( msg, bj.font, x, y )
   end
end

function bj.keypressed( key )
   if bj.status ~= 0 then
      bj.deal()
      return
   end

   if key=="h" then
      bj.hit()
   elseif key=="s" then
      bj.stay()
   end
end

return bj
