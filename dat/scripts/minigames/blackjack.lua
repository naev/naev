local minerva = require "minerva"
local lg = require 'love.graphics'
local la = require 'love.audio'
local cardio = require 'minigames.cardio'
local love_math = require 'love.math'
require 'numstring'

local bj = { -- too lazy to write blackjack over and over
   sound = {
      place = cardio.sound.place,
      chips = {
      }
   }
}
for i=1,6 do
   local f = string.format("snd/sounds/gambling/chipsStack%d.ogg",i)
   local s = la.newSource(f)
   table.insert( bj.sound.chips, s )
end

function bj.init( x, y, w, h, donefunc )
   bj.deck = cardio.newDeckWestern( false )
   bj.font = lg.newFont(16)

   -- Compute position stuff
   bj.buttons = {_("#wH#0it"), _("#wS#0tay")}
   bj.bets =  {_("#w1#0. Bet 10 k¤"), _("#w2#0. Bet 100 k¤"), _("#w3#0. Leave")}
   bj.bets_b = 15
   bj.bets_w = -bj.bets_b
   for k,s in ipairs(bj.bets) do
      local bw = bj.font:getWidth(s) + 3*bj.bets_b
      bj.bets_w = bj.bets_w + bw
   end
   bj.bets_x = x + (w-bj.bets_w)/2
   bj.bets_y = y

   -- Scaling factor
   -- Total height is: scale*(105*2+sep)+40+3*font:height
   h = h - 4*bj.font:getHeight() - 60 - bj.bets_b*2
   bj.scale = math.min( 1, h/240 )
   bj.donefunc = donefunc

   -- Start out betting
   bj.status = 0
   bj.done = false
   bj.betting = true
   bj.dealer = {}
   bj.player = {}
   bj.msg = nil
   bj.chatter = nil
   bj.chatter_color = nil
end

local function _chatter( chat_type )
   local text
   bj.chatter_color = minerva.chicken.colour
   if chat_type==nil then
      text = nil
   elseif chat_type=="won" then
      if love_math.random() < 0.5 then
         text = nil
      else
         local textlist = {
            _("Cyborg Chicken does a little jig."),
            _("Cyborg Chicken does a short dance."),
            _("Cyborg Chicken strikes a cool pose."),
            _("Cyborg Chicken squawks in delight."),
            _("Cyborg Chicken looks happy."),
            _("Cyborg Chicken does the robot."),
            _("Cyborg Chicken beams."),
         }
         text = textlist[love_math.random(1,#textlist)]
      end
   elseif chat_type=="lost" then
      if love_math.random() < 0.5 then
         text = nil
      else
         local textlist = {
            _("Cyborg Chicken looks a bit slum."),
            _("Cyborg Chicken squints at you."),
            _("Cyborg Chicken pecks at the ground."),
            _("Cyborg Chicken kicks at the blackjack table."),
            _("Cyborg Chicken beeps."),
            _("Cyborg Chicken squawks defeatedly."),
         }
         text = textlist[love_math.random(1,#textlist)]
      end
   elseif chat_type=="push" then
      text = _("Cyborg Chicken squawks.")
   elseif chat_type=="won_blackjack" then
      text = _("Cyborg Chicken looks very smug.")
   elseif chat_type=="lost_blackjack" or chat_type=="double_blackjack" then
      text = _("Cyborg Chicken squawks in surprise!")
   end
   bj.chatter = text
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
   local p = _total(bj.player)
   local d = _total(bj.dealer)
   local msg
   if p>21 or (d<=21 and p<d) then
      msg = _("#rYou lost!#0")
      if #bj.dealer == 2 and d==21 then
         msg = string.format(_("#pBlackjack!#0 %s"), msg)
         _chatter( "won_blackjack" )
      else
         _chatter( "won" )
      end
   elseif d>21 or (p<=21 and d<p) then
      local won = bj.betamount / 1000
      minerva.tokens_pay( won )
      msg = string.format(_("#gYou won #p%d Minerva Tokens#g!#0"), won)
      if #bj.player == 2 and p==21 then
         msg = string.format(_("#pBlackjack!#0 %s"), msg)
         _chatter( "lost_blackjack" )
      else
         _chatter( "lost" )
      end
   else
      if #bj.player==2 and #bj.dealer==2 and d==21 and p==21 then
         msg = _("Double Blackjack! Push!")
         _chatter( "double_blackjack" )
      else
         msg = _("Push!")
         _chatter( "push" )
      end
   end
   bj.msg = msg
   bj.betting = true
end

function bj.deal()
   bj.chatter = nil
   bj.status = 0
   bj.done = false
   bj.betting = false
   bj.dealer = {}
   bj.player = {}
   bj.deck:shuffle()
   bj.msg = nil
   table.insert( bj.player, bj.deck:draw() )
   table.insert( bj.player, bj.deck:draw() )
   table.insert( bj.dealer, bj.deck:draw() )
   table.insert( bj.dealer, bj.deck:draw() )
   -- Special case for blackjacks
   local p = _total(bj.player)
   local d = _total(bj.dealer)
   if p==21 or d==21 then
      _done()
   end
end

function bj.hit()
   bj.sound.place[love_math.random(1,#bj.sound.place)]:play()
   table.insert( bj.player, bj.deck:draw() )
   -- Check if player lost
   local p = _total(bj.player)
   if p > 21 then
      _done()
   end
   if p==21 then
      -- See if dealer grabs another card
      local d, da = _total(bj.dealer)
      while d < 17 or (d==17 and da>0) do
         table.insert( bj.dealer, bj.deck:draw() )
         d, da = _total(bj.dealer)
      end
      _done()
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
   _done()
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
   local y = by + 20

   -- Special chatter
   if bj.chatter then
      lg.setColor( bj.chatter_color or {1,1,1} )
      lg.print( bj.chatter, bj.font, x, y )
   end
   y = y + bj.font:getHeight()+20

   -- Dealer
   if #bj.dealer > 0 then
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
   else
      y = y + bj.font:getHeight()+10
   end

   -- Player
   if #bj.player > 0 then
      local tplayer = _total(bj.player)
      x = rs
      y = y + (h + sep)*bj.scale
      lg.setColor( 1, 1, 1 )
      lg.print( string.format(_("Player: %d"),tplayer), bj.font, x, y )
      y = y + bj.font:getHeight()+10
      _drawhand( x, y, bj.player )
   else
      y = y + (h + sep)*bj.scale + bj.font:getHeight()+10
   end

   -- Print status
   y = y + h + 20
   if bj.msg ~= nil then
      x = rs
      lg.setColor( 1, 1, 1 )
      lg.print( bj.msg, bj.font, x, y )
   end

   -- Buttons
   bj.bets_y = y + bj.font:getHeight() + 20
   local b = bj.bets_b
   local mx, my = love.mouse.getX(), love.mouse.getY()
   y = bj.bets_y
   x = bj.bets_x
   h = bj.font:getHeight()
   local buttons
   if bj.betting then
      buttons = bj.bets
   else
      buttons = bj.buttons
   end
   for k,s in ipairs( buttons ) do
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
      lg.setColor( 0.7, 0.7, 0.7 )
      lg.print( s, bj.font, x+b, y+b )
      x = x + 3*b + w
   end
   y = bj.bets_y + h+3*b
   local tokens = minerva.tokens_get()
   local s = string.format(_("You have %s credits and #p%s Minerva Tokens#0."), creditstring(player.credits()), numstring(tokens))
   w = bj.font:getWidth( s )
   lg.print( s, bj.font, bx+(bw-w)/2, y )
end

local function trybet( betamount )
   if player.credits() < betamount then
      bj.msg = string.format(_("#rNot enough credits! You only have %s!#0"), creditstring(player.credits()))
   else
      player.pay(-betamount)
      bj.betamount = betamount
      bj.msg = string.format(_("You bet %s."),creditstring(betamount))
      bj.deal()
      bj.sound.chips[love_math.random(1,#bj.sound.chips)]:play()
   end
end

function bj.keypressed( key )
   if not bj.betting then
      if key=="h" then
         bj.hit()
      elseif key=="s" then
         bj.stay()
      end
   else
      if key=="1" then
         trybet( 10000 )
      elseif key=="2" then
         trybet( 100000 )
      elseif key=="3" then
         bj.donefunc()
      end
   end
end

function bj.mousepressed( mx, my, button )
   local y = bj.bets_y
   local x = bj.bets_x
   local h = bj.font:getHeight()
   local b = bj.bets_b
   local buttons
   if bj.betting then
      buttons = bj.bets
   else
      buttons = bj.buttons
   end
   for k,s in ipairs( buttons ) do
      local w = bj.font:getWidth( s )
      local col
      if _inbox( mx, my, x, y, w+2*b, h+2*b ) then
         if bj.betting then
            local credits = player.credits()
            if k==1 then
               trybet( 10000 )
            elseif k==2 then
               trybet( 100000 )
            else
               bj.donefunc()
            end
         else
            if k==1 then
               bj.hit()
            else
               bj.stay()
            end
         end
         return
      end
      x = x + 3*b + w
   end
end

return bj
