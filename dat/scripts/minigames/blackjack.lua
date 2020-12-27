local lg = require 'love.graphics'

local bj = {} -- too lazy to write blackjack over and over

function bj.init( w, h, donefunc )
   local cardio = require 'minigames.cardio'
   bj.deck = cardio.newDeckWestern( false )
   bj.font = lg.newFont(16)

   -- Compute position stuff
   bj.bets =  {_("Bet 10 k¤"), _("Bet 100 k¤"), _("Leave")}
   bj.bets_b = 15
   bj.bets_w = -bj.bets_b
   for k,s in ipairs(bj.bets) do
      local bw = bj.font:getWidth(s) + 3*bj.bets_b
      bj.bets_w = bj.bets_w + bw
   end
   bj.bets_x = (w-bj.bets_w)/2

   -- Scaling factor
   -- Total height is: scale*(105*2+sep)+40+3*font:height
   h = h - 4*bj.font:getHeight() - 60 - bj.bets_b*2
   bj.scale = math.min( 1, h/240 )
   bj.donefunc = donefunc
end

local function _inbox( mx, my, x, y, w, h )
   return (mx>=x and mx<=x+w and my>=y and my<=y+h)
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
      _done(0)
   elseif p==21 then
      _done(1)
   elseif d==21 then
      _done(-1)
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
   else
      _done(0)
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
      msg = _("\arYou lost!\a0")
      if #bj.dealer == 2 and d==21 then
         msg = string.format(_("\apBlackjack!\a0 %s"), msg)
      end
   elseif bj.status > 0 then
      msg = _("\agYou won!\a0")
      if #bj.player == 2 and p==21 then
         msg = string.format(_("\apBlackjack!\a0 %s"), msg)
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

   -- Buttons
   if bj.done then
      local mx, my = love.mouse.getX(), love.mouse.getY()
      y = y + bj.font:getHeight()+20
      x = bx + bj.bets_x
      h = bj.font:getHeight()
      local b = bj.bets_b
      for k,s in ipairs( bj.bets ) do
         w = bj.font:getWidth( s )
         local col
         if _inbox( mx, my, x, y, w+2*b, h+2*b ) then
            col = {0.5, 0.5, 0.5}
         else
            col = {0, 0, 0}
         end
         lg.setColor( 0.5, 0.5, 0.5 )
         lg.rectangle( "fill", x, y, w+2*b, h+2*b )
         lg.setColor( col )
         lg.rectangle( "fill", x+2, y+2, w+2*b-4, h+2*b-4 )
         lg.setColor( 1, 1, 1 )
         lg.print( s, bj.font, x+b, y+b )
         x = x + 3*b + w
      end
   end
end

function bj.keypressed( key )
   if bj.done then
      bj.deal()
      return
   end

   if key=="h" then
      bj.hit()
   elseif key=="s" then
      bj.stay()
   end
end

function bj.mousepressed( mx, my, button )
end

return bj
