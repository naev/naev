local minerva = require "common.minerva"
local lg = require 'love.graphics'
local la = require 'love.audio'
local diceio = require 'minigames.lib.diceio'
local love_math = require 'love.math'
local fmt = require "format"


local cl = { -- too lazy to write chuck-a-luck
   sound = {
      throw = diceio.sound.throw,
      chips = {
      }
   }
}
for i=1,6 do
   local f = string.format("snd/sounds/gambling/chipsStack%d.ogg",i)
   local s = la.newSource(f)
   table.insert( cl.sound.chips, s )
end

local secretcode = { 5, 6, 3, 1 }
local secretcode_status = 0

function cl.init( x, y, w, _h, donefunc )
   cl.dice = { diceio.newDie(), diceio.newDie(), diceio.newDie() }
   cl.font = lg.newFont(16)

   -- Reset secret code
   secretcode_status = 0

   -- Compute position stuff
   cl.buttons = { "1", "2", "3", "4", "5", "6" }
   cl.buttons_b = 15
   cl.buttons_w = -cl.buttons_b
   for k,s in ipairs(cl.buttons) do
      local bw = cl.font:getWidth(s) + 3*cl.buttons_b
      cl.buttons_w = cl.buttons_w + bw
   end
   cl.buttons_x = x + (w-cl.buttons_w)/2
   cl.buttons_y = y
   -- bets
   cl.bets =  {_("#w1#0. Bet 10 k¤"), _("#w2#0. Bet 100 k¤"), _("#w3#0. Leave")}
   cl.bets_b = 15
   cl.bets_w = -cl.bets_b
   for k,s in ipairs(cl.bets) do
      local bw = cl.font:getWidth(s) + 3*cl.bets_b
      cl.bets_w = cl.bets_w + bw
   end
   cl.bets_x = x + (w-cl.bets_w)/2
   cl.bets_y = y

   -- Start out betting
   cl.betting = true
   cl.msg = nil
   cl.chatter = nil
   cl.chatter_color = nil
   cl.donefunc = donefunc
end

local function _chatter( chat_type )
   local text
   cl.chatter_color = nil
   -- Special secretcode being input chat
   if var.peek("minerva_caninputcode") then
      if secretcode_status == 4 then
         cl.chatter = _("The dealer hands you a note saying to meet him after his shift.")
         naev.trigger( "minerva_secretcode" )
         cl.secretcode = true
         return
      elseif secretcode_status > 2 then
         cl.chatter = _("The dealer raises his eyebrows at you.")
         return
      end
   end
   -- Normal chat
   if chat_type==nil then
      text = nil
   elseif chat_type==0 then
      if love_math.random() < 0.5 then
         text = nil
      else
         local textlist = {
            _("The dealer hums to himself."),
            _("The dealer smiles."),
            _("The dealer chuckles."),
         }
         text = textlist[love_math.random(1,#textlist)]
      end
   elseif chat_type==1 then
      if love_math.random() < 0.5 then
         text = nil
      else
         local textlist = {
            _("The dealer frowns slightly."),
            _("The dealer wrinkles their eyebrows."),
         }
         text = textlist[love_math.random(1,#textlist)]
      end
   elseif chat_type==2 then
      local textlist = {
         _("The dealer frowns."),
         _("The dealer sighs."),
      }
      text = textlist[love_math.random(1,#textlist)]
   elseif chat_type==3 then
      text = _("The dealer lets out a big sigh.")
   end
   cl.chatter = text
end

local function _inbox( mx, my, x, y, w, h )
   return (mx>=x and mx<=x+w and my>=y and my<=y+h)
end

function cl.draw( bx, by, bw, _bh )
   naev.gfx.clearDepth()
   local sep = 25
   local w = 50
   local h = 50
   local rs = bx + (bw-(w*3+sep*2))/2
   local x = rs
   local y = by + 20

   -- Special chatter
   if cl.chatter then
      lg.setColor( cl.chatter_color or {1,1,1} )
      local tw = cl.font:getWidth( cl.chatter )
      lg.print( cl.chatter, cl.font, bx+(bw-tw)/2, y )
   end
   y = y + cl.font:getHeight()+20

   -- Dice
   for k,die in ipairs(cl.dice) do
      die:draw( x, y )
      x = x + w+sep
   end
   y = y + h+20

   -- Print status
   if cl.msg ~= nil then
      lg.setColor( 1, 1, 1 )
      local tw = cl.font:getWidth( cl.msg )
      lg.print( cl.msg, cl.font, bx+(bw-tw)/2, y )
   end

   -- Buttons
   y = y + cl.font:getHeight() + 20
   local b
   local mx, my = love.mouse.getX(), love.mouse.getY()
   h = cl.font:getHeight()
   local buttons
   if cl.betting then
      buttons = cl.bets
      b = cl.bets_b
      x = cl.bets_x
      cl.bets_y = y
   else
      buttons = cl.buttons
      b = cl.buttons_b
      x = cl.buttons_x
      cl.buttons_y = y
   end
   for k,s in ipairs(buttons) do
      local tw = cl.font:getWidth( s )
      local col
      if _inbox( mx, my, x, y, tw+2*b, h+2*b ) then
         col = {0.5, 0.5, 0.5}
      else
         col = {0, 0, 0}
      end
      lg.setColor( 0.5, 0.5, 0.5 )
      lg.rectangle( "fill", x, y, tw+2*b, h+2*b )
      lg.setColor( col )
      lg.rectangle( "fill", x+2, y+2, tw+2*b-4, h+2*b-4 )
      lg.setColor( 0.7, 0.7, 0.7 )
      lg.print( s, cl.font, x+b, y+b )
      x = x + 3*b + tw
   end
   y = y + h+3*b
   local tokens = minerva.tokens_get()
   local s = fmt.f(n_("You have {credits} and #p{n} Minerva Token#0.", "You have {credits} and #p{n} Minerva Tokens#0.", tokens), {credits=fmt.credits(player.credits()), n=fmt.number(tokens)})
   local tw = cl.font:getWidth( s )
   lg.print( s, cl.font, bx+(bw-tw)/2, y )
end

local function trybet( betamount )
   if player.credits() < betamount then
      cl.msg = fmt.f(_("#rNot enough credits! You only have {credits}!#0"), {credits=fmt.credits(player.credits())})
   else
      player.pay(-betamount)
      cl.betamount = betamount
      cl.msg = fmt.f(_("You bet {credits}."), {credits=fmt.credits(betamount)})
      cl.sound.chips[love_math.random(1,#cl.sound.chips)]:play()
      cl.betting = false
   end
end

function cl.play( value )
   cl.sound.throw[love_math.random(1,#cl.sound.throw)]:play()

   -- Handle secretcode
   if secretcode_status >= 0 then
      if secretcode[ secretcode_status+1 ] == value then
         secretcode_status = secretcode_status+1
      end
   end

   -- All logic here
   local matches = 0
   for k,die in ipairs(cl.dice) do
      local v = die:roll()
      if v==value then
         matches = matches + 1
      end
   end
   local won = cl.betamount / 1000
   local msg = nil
   if matches==0 then
      won = won*0
      msg = "#r".._("You lost!").."#0"
   elseif matches==1 then
      won = won*1
      msg = fmt.f(n_("#gYou won #p{n} Minerva Token#g (matched one)!#0", "#gYou won #p{n} Minerva Tokens#g (matched one)!#0", won), {n=won})
   elseif matches==2 then
      won = won*2
      msg = fmt.f(n_("#gYou won #p{n} Minerva Token#g (matched two)!#0", "#gYou won #p{n} Minerva Tokens#g (matched two)!#0", won), {n=won})
   elseif matches==3 then
      won = won*10
      msg = fmt.f(n_("#gYou won #p{n} Minerva Token#g (matched three)!#0", "#gYou won #p{n} Minerva Tokens#g (matched three)!#0", won), {n=won})
   end
   if won > 0 then
      minerva.tokens_pay( won )
   end
   cl.msg = msg
   cl.betting = true
   _chatter( matches )
end

function cl.keypressed( key )
   if not cl.betting then
      local val = tonumber(key)
      if val and val>=1 and val<=6 then
         cl.play(val)
      end
   else
      if key=="1" then
         trybet( 10e3 )
      elseif key=="2" then
         trybet( 100e3 )
      elseif key=="3" then
         cl.donefunc()
      end
   end
end

function cl.mousepressed( mx, my, _button )
   local x, y, b
   local h = cl.font:getHeight()
   local buttons
   if cl.betting then
      buttons = cl.bets
      x = cl.bets_x
      y = cl.bets_y
      b = cl.bets_b
   else
      buttons = cl.buttons
      x = cl.buttons_x
      y = cl.buttons_y
      b = cl.buttons_b
   end
   for k,s in ipairs( buttons ) do
      local w = cl.font:getWidth( s )
      if _inbox( mx, my, x, y, w+2*b, h+2*b ) then
         if cl.betting then
            if k==1 then
               secretcode_status = math.max( secretcode_status, 0 )
               trybet( 10e3 )
            elseif k==2 then
               secretcode_status = -1
               trybet( 100e3 )
            else
               cl.donefunc()
            end
         else
            cl.play(k)
         end
         return
      end
      x = x + 3*b + w
   end
end

return cl
