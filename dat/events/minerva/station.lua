--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Minerva Station Gambling">
 <location>land</location>
 <chance>100</chance>
 <cond>spob.cur()==spob.get("Minerva Station")</cond>
 <notes>
  <campaign>Minerva</campaign>
  <provides name="Minerva Station" />
 </notes>
</event>
--]]
--[[
   Event handling the gambling stuff going on at Minerva station
--]]
local fmt = require "format"
local minerva = require 'common.minerva'
local portrait = require "portrait"
local vn = require 'vn'
local blackjack = require 'minigames.blackjack'
local chuckaluck = require 'minigames.chuckaluck'
local lg = require 'love.graphics'
local window = require 'love.window'
--local love_shaders = require 'love_shaders'

local npc_patrons -- Non-persistent state

-- NPC Stuff
local gambling_priority = 3
local important_npc_priority = 4
local terminal = minerva.terminal
local blackjack_portrait = "blackjack.png"
local chuckaluck_portrait, chuckaluck_image
if var.peek("minerva_chuckaluck_change") then
   chuckaluck_portrait = portrait.get() -- Becomes a random NPC
   chuckaluck_image = portrait.getFullPath( chuckaluck_portrait )
else
   chuckaluck_portrait = minerva.mole.portrait
   chuckaluck_image = minerva.mole.image
end
local greeter_portrait = portrait.get() -- TODO replace?

-- Special
local spaticketcost = 100

local patron_names = {
   _("Patron"),
}
local patron_descriptions = {
   _("A gambling patron enjoying their time at the station."),
   _("A tourist looking a bit bewildered at all the noises and shiny lights all over."),
   _("A patron who seems down on their luck."),
   _("A patron who looks exhilarated as if they won big today."),
   _("A patron that looks like they have spend a lot of time at the station. There are clear dark circles under their eyes."),
   _("A patron that looks strangely out of place."),
   _("A patron that fits in perfectly into the gambling station."),
}
local patron_messages = {
   _([["This place is totally what I thought it would be. The lights, the sounds, the action! I feel like I'm in Heaven!"]]),
   _([["It's incredible! Who would have thought to make money physical! These Minerva Tokens defy all logic!"]]),
   function ()
      local soldoutmsg = ""
      if player.numOutfit("Fuzzy Dice") > 0 then
         soldoutmsg = _(" Wait, what? What do you mean they are sold out!?")
      end
      return fmt.f(_([["I really have my eyes on the Fuzzy Dice available at the terminal. I always wanted to own a piece of history!{msg}"]]), {msg=soldoutmsg} ) end,
   _([["I played 20 hands of blackjack with that Cyborg Chicken. I may have lost them all, but that was worth every credit!"]]),
   _([["This place is great! I still have no idea how to play blackjack, but I just keep on playing again and again against that Cyborg Chicken."]]),
   function () return fmt.f(
      _([["I came all the way from {pnt} to be here! We don't have anything like this back at home."]]),
      {pnt=spob.get( {faction.get("Dvaered"), faction.get("Za'lek"), faction.get("Empire"), faction.get("Soromid")} )}
   ) end,
   _([["Critics of Minerva Station say that being able to acquire nice outfits here without needing licenses increases piracy. I think they are all lame!"]]),
   _([["I really want to go to the VIP hot springs they have, but I don't have the tokens. How does that even work in a space station?"]]),
   _([["I hear you can do all sorts of crazy stuff here if you have enough tokens. Need… to… get… more…!"]]),
   _([["I have never seen robots talk so roboty like the terminals here. That is so retro!"]]),
   _([["I scrounged up my lifetime savings to get a ticket here, but I forgot to bring extra to gamble…"]]),
   _([["I gambled all my savings away… I'm going to get killed when I get back home…"]]),
   _([["They say you shouldn't gamble more than you can afford to lose. I wish someone had told me that yesterday. I don't even own a ship anymore!"]]),
   _([["I like to play blackjack. I'm not addicted to gambling. I'm addicted to sitting in a semi-circle."]]), -- Mitch Hedberg
   _([["Gambling has brought our family together. We had to move to a smaller house."]]), -- Tommy Cooper
   _([["You don’t gamble to win. You gamble so you can gamble the next day."]]), -- Bert Ambrose
   _([["A credit won is twice as sweet as a credit earned!"]]), -- Paul Newman (dollar -> credit)
   _([["There is a very easy way to return from Minerva Station with a small fortune: come here with a large one!"]]), -- Jack Yelton (paraphrased)
   _([["Luck always seems to be against the ones who depends on it."]]),
}

function create()

   -- Create NPCs
   mem.npc_terminal = evt.npcAdd( "approach_terminal", terminal.name, terminal.portrait, terminal.description, gambling_priority )
   mem.npc_blackjack = evt.npcAdd( "approach_blackjack", _("Blackjack"), blackjack_portrait, _("Seems to be one of the more popular card games where you can play blackjack against a \"cyborg chicken\"."), gambling_priority )
   mem.npc_chuckaluck = evt.npcAdd( "approach_chuckaluck", _("Chuck-a-luck"), chuckaluck_portrait, _("A fast-paced luck-based betting game using dice."), gambling_priority )

   -- Some conditional NPCs
   if player.misnDone("Maikki's Father 2") then
      local desclist = {
         _("You see Maikki enjoying a parfait."),
         _("You see Maikki talking on a transponder."),
         _("You see Maikki looking thoughtfully into the distance."),
      }
      local desc = desclist[ rnd.rnd(1,#desclist) ]
      mem.npc_maikki = evt.npcAdd( "approach_maikki", minerva.maikki.name, minerva.maikki.portrait, desc, important_npc_priority )
   end

   -- Create random noise NPCs
   local npatrons = rnd.rnd(3,5)
   npc_patrons = {}
   local msglist = rnd.permutation( patron_messages ) -- avoids duplicates
   for i = 1,npatrons do
      local name = patron_names[ rnd.rnd(1, #patron_names) ]
      local img = portrait.get()
      local desc = patron_descriptions[ rnd.rnd(1, #patron_descriptions) ]
      local msg = msglist[i]
      local id = evt.npcAdd( "approach_patron", name, img, desc, 10 )
      local npcdata = { name=name, image=portrait.getFullPath(img), message=msg }
      npc_patrons[id] = npcdata
   end

   -- If scavengers are not dead, they sometimes appear
   if var.peek("maikki_scavengers_alive") and rnd.rnd() < 0.05 then
      evt.npcAdd( "approach_scavengers", minerva.scavengers.name, minerva.scavengers.portrait, minerva.scavengers.description )
   end

   -- If they player never had tokens, it is probably their first time
   if not var.peek( "minerva_tokens_gained" ) then
      mem.greeterhook = hook.land( "bargreeter", "bar" )
   end
   -- End event on takeoff.
   hook.takeoff( "leave" )
   hook.custom( "minerva_molecaught", "molecaught" )
end

local function has_event( name )
   return player.evtDone( name ) or player.evtActive( name )
end
--[[
-- Function that handles creating and starting random events that occur at the
-- bar. This is triggered randomly upon finishing gambling activities.
--]]
local function random_event()
   -- Conditional helpers
   local alter1 = has_event("Minerva Station Altercation 1")
   local alter_helped = (var.peek("minerva_altercation_helped")~=nil)
   local alter_prob = var.peek("minerva_altercation_probability") or 0.4
   local alter2 = has_event("Minerva Station Altercation 2")
   local maikki2 = player.misnDone("Maikki's Father 2")
   local spapropaganda = has_event("Spa Propaganda")
   local r = rnd.rnd()
   -- Altercations
   if not alter1 and minerva.tokens_get_gained() > 10 and r < 0.5 then
      hook.safe( "start_alter1" )
   elseif not alter2 and maikki2 and r < 0.5 then
      hook.safe( "start_alter2" )
   elseif not alter1 and alter2 and not alter_helped and r < alter_prob then
      hook.safe( "start_alter1" )

   -- Spa Propaganda
   elseif maikki2 and player.misnActive("Minerva Pirates 3") and not spapropaganda then
      hook.safe( "start_spapropaganda" )
   end
end

-- TODO probably a bug, but we should be able to pass a hook argument instead of hardcoding the function
-- Doesn't seem to work however
function start_alter1 ()
   naev.eventStart( "Minerva Station Altercation 1" )
end
function start_alter2 ()
   naev.eventStart( "Minerva Station Altercation 2" )
end
function start_spapropaganda ()
   naev.eventStart( "Spa Propaganda" )
end

function bargreeter()
   vn.clear()
   vn.scene()
   local g = vn.newCharacter( _("Greeter"),
         { image=portrait.getFullPath( greeter_portrait ) } )
   vn.transition()
   vn.na( _([[As soon as you enter the spaceport bar, a neatly dressed individual runs up to you and hands you a complementary drink. It is hard to make out what he is saying over all the background noise created by other patrons and gambling machines, but you try to make it out as best as you can.]]) )
   g:say( _([["Welcome to the Minerva Station resort! It appears to be your first time here. As you enjoy your complementary drink, let me briefly explain to you how this wonderful place works. It is all very exciting!"]]) )
   g:say( _([["The currency we use on this station are Minerva Tokens. Unlike credits, they are physical and so very pretty! You can not buy Minerva Tokens directly, however, by participating and betting credits in the various fine games available, you can obtain Minerva Tokens. When you have enough Minerva Tokens, you are able to buy fabulous prizes and enjoy more exclusive areas of our resort. To start out your fun Minerva Adventure®, please enjoy these 10 complementary Minerva Tokens!"]]) )
   g:say( _([["If you want more information or want to check your balance. Please use the terminals located throughout the station. I highly recommend you check out our universe-famous Cyborg Chicken at the blackjack table, and always remember, 'life is short, spend it at Minerva Station'®!"]]) )
   vn.run()

   minerva.tokens_pay( 10 )
   hook.rm( mem.greeterhook ) -- Have to remove
end

function approach_terminal()

   local msgs = {
      _(" TODAY MIGHT BE YOUR LUCKY DAY."),
      _(" THIS IS SO EXCITING."),
      _(" YOU SEEM LIKE YOU MIGHT ENJOY A GAME OF BLACKJACK."),
      _(" FORTUNE FAVOURS THE PERSISTENT."),
      _(" LIFE IS SHORT, SPEND IT AT MINERVA STATION."),
   }
   vn.clear()
   vn.scene()
   local t = vn.newCharacter( minerva.vn_terminal() )
   vn.transition()
   vn.label( "start" )
   t:say( function() return fmt.f(
         n_([["VALUED CUSTOMER, YOU HAVE #p{n} MINERVA TOKEN#0.{msg}

WHAT DO YOU WISH TO DO TODAY?"]],
            [["VALUED CUSTOMER, YOU HAVE #p{n} MINERVA TOKENS#0.{msg}

WHAT DO YOU WISH TO DO TODAY?"]], minerva.tokens_get()),
               {n=fmt.number(minerva.tokens_get()), msg=msgs[rnd.rnd(1,#msgs)]}) end )
   vn.menu( {
      {_("Information"), "info"},
      {_("Trade-in"), "trade"},
      {_("Leave"), "leave"},
   } )
   vn.label( "info" )
   t:say( _([["I AM PROGRAMMED TO EXPLAIN ABOUT THE WONDERFUL MINERVA STATION GAMBLING FACILITIES. WHAT WOULD YOU LIKE TO KNOW ABOUT?"]]) )
   vn.jump( "info_menu" )
   vn.label( "more_info" )
   t:say( _([["WHAT ELSE WOULD YOU LIKE TO KNOW?"]]) )
   vn.label( "info_menu" )
   vn.menu( {
      {_("Station"), "info_station"},
      {_("Gambling"), "info_gambling"},
      {_("Trade-in"), "info_trade"},
      {_("Cyborg Chicken"), "info_chicken"},
      {_("Back"), "start"},
   } )
   vn.label( "info_station" )
   t:say( _([["MINERVA STATION IS THE BEST PLACE TO SIT BACK AND ENJOY RELAXING GAMBLING ACTIVITIES. ALTHOUGH THE AREA IS HEAVILY DISPUTED BY THE ZA'LEK AND DVAERED, REST ASSURED THAT THERE IS LESS THAN A 2% OF CHANCE OF TOTAL DESTRUCTION OF THE STATION."]]) )
   vn.jump( "more_info" )
   vn.label( "info_gambling" )
   t:say( _([["WHILE GAMBLING IS NOT ALLOWED IN MOST OF THE EMPIRE, MINERVA STATION BOASTS OF AN EXCLUSIVE STATUS THANKS TO THE IMPERIAL DECREE 289.78 ARTICLE 478 SECTION 19 ALLOWING GAMBLING TO BE ENJOYED WITHOUT RESTRICTIONS. IT IS POSSIBLE TO PLAY GAMES USING CREDITS TO OBTAIN MINERVA TOKENS THAT CAN BE TRADED IN FOR GOODS AND SERVICES ANY TERMINAL THROUGHOUT THE STATION."]] ) )
   vn.jump( "more_info" )
   vn.label( "info_trade" )
   t:say( _([["IT IS POSSIBLE TO TRADE MINERVA TOKENS FOR GOODS AND SERVICES AT TERMINALS THROUGHOUT THE STATION. THANKS TO THE IMPERIAL DECREE 289.78 ARTICLE 478 SECTION 72, ALL TRADE-INS ARE NOT SUBJECT TO STANDARD IMPERIAL LICENSE RESTRICTIONS. FURTHERMORE, THEY ALL HAVE 'I Got This Sucker at Minerva Station' ENGRAVED ON THEM."]]) )
   vn.jump( "more_info" )
   vn.label( "info_chicken" )
   t:say( _([["CYBORG CHICKEN IS OUR MOST POPULAR BLACKJACK DEALER. NOWHERE ELSE IN THE UNIVERSE WILL YOU BE ABLE TO PLAY CARD GAMES WITH AN AI-ENHANCED CHICKEN CYBORG. IT IS A ONCE AND A LIFE-TIME CHANCE THAT YOU SHOULD NOT MISS."]]) )
   vn.jump( "more_info" )

   vn.label( "trade_notenough" )
   t:say( function() return fmt.f(
         n_([["SORRY, YOU DO NOT HAVE ENOUGH MINERVA TOKENS TO TRADE-IN FOR YOUR REQUESTED ITEM. WOULD YOU LIKE TO TRADE-IN FOR SOMETHING ELSE? YOU HAVE #p{n} MINERVA TOKEN#0."]],
            [["SORRY, YOU DO NOT HAVE ENOUGH MINERVA TOKENS TO TRADE-IN FOR YOUR REQUESTED ITEM. WOULD YOU LIKE TO TRADE-IN FOR SOMETHING ELSE? YOU HAVE #p{n} MINERVA TOKENS#0."]], minerva.tokens_get()),
         {n=fmt.number(minerva.tokens_get())} ) end )
   vn.jump( "trade_menu" )
   vn.label( "trade_soldout" )
   t:say( function() return fmt.f(
         n_([["I AM SORRY TO INFORM YOU THAT THE ITEM THAT YOU DESIRE IS CURRENTLY SOLD OUT. WOULD YOU LIKE TO TRADE-IN FOR SOMETHING ELSE? YOU HAVE #p{n} MINERVA TOKEN#0."]],
            [["I AM SORRY TO INFORM YOU THAT THE ITEM THAT YOU DESIRE IS CURRENTLY SOLD OUT. WOULD YOU LIKE TO TRADE-IN FOR SOMETHING ELSE? YOU HAVE #p{n} MINERVA TOKENS#0."]], minerva.tokens_get()),
	    {n=fmt.number(minerva.tokens_get())} ) end )
   vn.jump( "trade_menu" )
   vn.label( "trade" )
   t:say( function() return fmt.f(
         n_([["YOU CAN TRADE IN YOUR PRECIOUS #p{n} MINERVA TOKEN#0 FOR THE FOLLOWING GOODS."]],
            [["YOU CAN TRADE IN YOUR PRECIOUS #p{n} MINERVA TOKENS#0 FOR THE FOLLOWING GOODS."]], minerva.tokens_get()),
	    {n=fmt.number(minerva.tokens_get())} ) end )
   local trades = {
      {"Ripper Cannon", {100, "outfit"}},
      {"TeraCom Fury Launcher", {500, "outfit"}},
      {"Railgun", {1000, "outfit"}},
      {"Grave Lance", {1200, "outfit"}},
      {"Fuzzy Dice", {5000, "outfit"}},
      {"Admonisher", {7000, "ship"}},
   }
   local tradein_item = nil
   local handler = function (idx)
      -- Jump in case of 'Back'
      if idx=="start" then
         vn.jump(idx)
         return
      end

      -- Special case
      if idx=="spaticket" then
         if spaticketcost > minerva.tokens_get() then
            -- Not enough money.
            vn.jump( "trade_notenough" )
         else
            vn.jump(idx)
         end
         return
      end

      if idx < 0 then
         vn.jump( "trade_soldout" )
         return
      end

      local trade = trades[idx]
      local tokens = trade[2][1]
      if tokens > minerva.tokens_get() then
         -- Not enough money.
         vn.jump( "trade_notenough" )
         return
      end

      tradein_item = trade
      if trade[2][2]=="outfit" then
         local o = outfit.get(trade[1])
         tradein_item.description = o:description()
      elseif trade[2][2]=="ship" then
         local s = ship.get(trade[1])
         tradein_item.description = s:description()
      else
         error(_("unknown tradein type"))
      end
      vn.jump( "trade_confirm" )
   end
   vn.label( "trade_menu" )
   vn.menu( function ()
      local opts = {}
      for k,v in ipairs(trades) do
         local tokens = v[2][1]
         local soldout = (v[2][2]=="outfit" and outfit.unique(v[1]) and player.numOutfit(v[1])>0)
         if soldout then
            opts[k] = { fmt.f(_("{item} (#rSOLD OUT#0)"), {item=_(v[1])}), -1 }
         else
            opts[k] = { fmt.f(_("{item} ({tokens})"), {item=_(v[1]), tokens=minerva.tokens_str(tokens)}), k }
         end
      end
      -- Add special ticket
      if player.evtDone("Spa Propaganda") and var.peek("minerva_spa_ticket")==nil then
         table.insert( opts, 1, {fmt.f(_("Special Spa Ticket ({tokens})"), {tokens=minerva.tokens_str(spaticketcost)}), "spaticket"} )
      end
      table.insert( opts, {_("Back"), "start"} )
      return opts
   end, handler )
   vn.jump( "start" )

   -- Buying stuff
   vn.label( "trade_confirm" )
   t:say( function () return fmt.f(
         _([["ARE YOU SURE YOU WANT TO TRADE IN FOR THE '#w{item}#0'? THE DESCRIPTION IS AS FOLLOWS:"
#w{description}#0]]),
         {item=_(tradein_item[1]), description=_(tradein_item.description)} ) end )
   vn.menu( {
      { _("Trade"), "trade_consumate" },
      { _("Cancel"), "trade" },
   }, function (idx)
      if idx=="trade_consumate" then
         local ti = tradein_item
         minerva.tokens_pay( -ti[2][1] )
         if ti[2][2]=="outfit" then
            player.outfitAdd( ti[1] )
            player.msg( _("Gambling Bounty"), fmt.reward(ti[1]))
         elseif ti[2][2]=="ship" then
            player.shipAdd( ti[1] )
         else
            error(_("unknown tradein type"))
         end
      end
      vn.jump(idx)
   end )
   vn.label("trade_consumate")
   vn.sfxMoney()
   t:say(_([["THANK YOU FOR YOUR BUSINESS."]]))
   vn.jump("trade")

   -- Buying the ticket
   vn.label("spaticket")
   t(_([["ARE YOU SURE YOU WANT TO TRADE IN FOR THE PREMIUM AND EXCLUSIVE SPECIAL SPA TICKET?"
"THIS TICKET WILL ALLOW YOU TO ENTER A LOTTERY TO WIN AN EXCLUSIVE RELAXING TIME AT THE MINERVA STATION ALL NATURAL SPA WITH CYBORG CHICKEN."]]))
   vn.menu( {
      {_("Trade"), "spabuyyes" },
      {_("Cancel"), "trade" },
   } )
   vn.label("spabuyyes")
   vn.func( function ()
      minerva.tokens_pay( -spaticketcost )
      var.push( "minerva_spa_ticket", true )
   end )
   vn.jump("trade_consumate")

   vn.label( "leave" )
   vn.run()

   -- Handle random bar events if necessary
   random_event()
end

function approach_blackjack()
   local firsttime = not var.peek("cc_known")
   -- Not adding to queue first
   local cc = minerva.vn_cyborg_chicken()
   vn.clear()
   vn.scene()
   if firsttime then
      vn.transition()
      vn.na( _("You make your way to the blackjack table which seems to be surrounded by many patrons, some of which are apparently taking pictures of something. You eventually have to elbow your way to the front to get a view of what is going on." ) )
      vn.appear( cc )
      vn.na( _("When you make it to the front you are greeted by the cold eyes of what apparently seems to be the Cyborg Chicken you were told about. It seems to be sizing the crowd while playing against a patron. The way it moves is very uncanny with short precise mechanical motions. You can tell it has been doing this for a while. You watch as the game progresses and the patron loses all his credits to the chicken, who seems unfazed.") )
      var.push("cc_known",true)
   end
   if not firsttime then
      vn.newCharacter( cc )
      vn.transition()
      vn.na( _("You elbow your way to the front of the table and are once again greeted by the cold mechanical eyes of Cyborg Chicken.") )
   end
   vn.na( "", true ) -- Clear buffer without waiting
   vn.label("menu")
   vn.menu( {
      { _("Play"), "blackjack" },
      { _("Explanation"), "explanation" },
      { _("Leave"), "leave" },
   } )
   vn.label( "explanation" )
   vn.na( _("Cyborg Chicken's eyes blink one second and go blank as a pre-recorded explanation is played from its back. Wait… are those embedded speakers?") )
   cc(_([["Welcome to MINERVA STATION'S blackjack table. The objective of this card game is to get as close to a value of 21 without going over. All cards are worth their rank except for Jack, Queen, and King which are all worth 10, and ace is either worth 1 or 11. You win if you have a higher value than CYBORG CHICKEN without going over 21."]]))
   vn.na( _("Cyborg Chicken eyes flutter as it seems like consciousness returns to its body.") )
   vn.jump("menu")
   vn.label( "blackjack" )
   -- Resize the window
   local lw, lh = window.getDesktopDimensions()
   local textbox_h = vn.textbox_h
   local textbox_x = vn.textbox_x
   local textbox_y = vn.textbox_y
   local dealer_x, dealer_newx
   local blackjack_h = 500
   local blackjack_x = math.min( lw-vn.textbox_w-100, textbox_x+200 )
   local blackjack_y = (lh-blackjack_h)/2
   local setup_blackjack = function (alpha)
      if dealer_x == nil then
         dealer_x = cc.offset -- cc.offset is only set up when the they appear in the VN
         dealer_newx = 0.2
      end
      vn.textbox_h = textbox_h + (blackjack_h - textbox_h)*alpha
      vn.textbox_x = textbox_x + (blackjack_x - textbox_x)*alpha
      vn.textbox_y = textbox_y + (blackjack_y - textbox_y)*alpha
      vn.namebox_alpha = 1-alpha
      cc.offset = dealer_x + (dealer_newx - dealer_x)*alpha
   end
   vn.animation( 0.5, function (alpha) setup_blackjack(alpha) end )
   local bj = vn.custom()
   bj._init = function( self )
      -- TODO play some blackjack music
      blackjack.init( vn.textbox_x, vn.textbox_y, vn.textbox_w, vn.textbox_h, function ()
         self.done = true
         -- TODO go back to normal music
      end )
   end
   bj._draw = function( _self )
      local x, y, w, h =  vn.textbox_x, vn.textbox_y, vn.textbox_w, vn.textbox_h
      -- Horrible hack where we draw ontop of the textbox a background
      lg.setColor( 0.5, 0.5, 0.5 )
      lg.rectangle( "fill", x, y, w, h )
      lg.setColor( 0, 0, 0 )
      lg.rectangle( "fill", x+2, y+2, w-4, h-4 )

      -- Draw blackjack game
      blackjack.draw( x, y, w, h)
   end
   bj._keypressed = function( _self, key )
      blackjack.keypressed( key )
   end
   bj._mousepressed = function( _self, mx, my, button )
      blackjack.mousepressed( mx, my, button )
   end
   -- Undo the resize
   vn.animation( 0.5, function (alpha) setup_blackjack(1-alpha) end )
   vn.label( "leave" )
   vn.na( _("You leave the blackjack table behind and head back to the main area.") )
   vn.run()

   -- Handle random bar events if necessary
   random_event()
end

function approach_chuckaluck ()
   -- Not adding to queue first
   vn.clear()
   vn.scene()
   local dealer = vn.newCharacter( _("Dealer"), {image=chuckaluck_image} )
   vn.transition()
   vn.na(_("You approach the chuck-a-luck table."))
   vn.na( "", true ) -- Clear buffer without waiting
   vn.label("menu")
   vn.menu( {
      { _("Play"), "chuckaluck" },
      { _("Explanation"), "explanation" },
      { _("Leave"), "leave" },
   } )
   vn.label( "explanation" )
   dealer(_([["Chuck-a-luck is a straight-forward game. You make your bet and then place a wager on what number the dice will come up as. If the number you chose matches one die, you win a token for each 1000 credits you bet. If you match two dice, you get double the amount of tokens. Furthermore, if you match all three dice, you get ten times the amount of tokens!"]]))
   vn.jump("menu")

   vn.label( "chuckaluck" )
   -- Resize the window
   local lw, lh = window.getDesktopDimensions()
   local textbox_h = vn.textbox_h
   local textbox_x = vn.textbox_x
   local textbox_y = vn.textbox_y
   local dealer_x, dealer_newx
   local chuckaluck_h = 500
   local chuckaluck_x = math.min( lw-vn.textbox_w-100, textbox_x+200 )
   local chuckaluck_y = (lh-chuckaluck_h)/2
   local setup_chuckaluck = function (alpha)
      if dealer_x == nil then
         dealer_x = dealer.offset -- dealer.offset is only set up when the they appear in the VN
         dealer_newx = 0.2
      end
      vn.textbox_h = textbox_h + (chuckaluck_h - textbox_h)*alpha
      vn.textbox_x = textbox_x + (chuckaluck_x - textbox_x)*alpha
      vn.textbox_y = textbox_y + (chuckaluck_y - textbox_y)*alpha
      vn.namebox_alpha = 1-alpha
      dealer.offset = dealer_x + (dealer_newx - dealer_x)*alpha
   end
   vn.animation( 0.5, function (alpha) setup_chuckaluck(alpha) end )
   local cl = vn.custom()
   cl._init = function( self )
      -- TODO play some gambling music
      chuckaluck.init( vn.textbox_x, vn.textbox_y, vn.textbox_w, vn.textbox_h, function ()
         self.done = true
         -- TODO go back to normal music
      end )
   end
   cl._draw = function( _self )
      local x, y, w, h =  vn.textbox_x, vn.textbox_y, vn.textbox_w, vn.textbox_h
      -- Horrible hack where we draw ontop of the textbox a background
      lg.setColor( 0.5, 0.5, 0.5 )
      lg.rectangle( "fill", x, y, w, h )
      lg.setColor( 0, 0, 0 )
      lg.rectangle( "fill", x+2, y+2, w-4, h-4 )

      -- Draw chuckaluck game
      chuckaluck.draw( x, y, w, h)
   end
   cl._keypressed = function( _self, key )
      chuckaluck.keypressed( key )
   end
   cl._mousepressed = function( _self, mx, my, button )
      chuckaluck.mousepressed( mx, my, button )
   end
   -- Undo the resize
   vn.animation( 0.5, function (alpha) setup_chuckaluck(1-alpha) end )
   vn.label( "leave" )
   vn.na( _("You leave the chuck-a-luck table behind and head back to the main area.") )
   vn.run()

   -- Handle random bar events if necessary, however, don't do it with secret code or we get a dialogue inside a dialogue.
   if not chuckaluck.secretcode then
      random_event()
   end
end

-- The mole was caught, we have to change and redo the chuckaluck NPC
function molecaught()
   var.push("minerva_chuckaluck_change", true)
   evt.npcRm( mem.npc_chuckaluck ) -- Just remove for now
end

-- Just do random noise
function approach_patron( id )
   local npcdata = npc_patrons[id]
   vn.clear()
   vn.scene()
   local patron = vn.newCharacter( npcdata.name, { image=npcdata.image } )
   vn.transition()
   patron( npcdata.message )
   vn.run()

   -- Handle random bar events if necessary
   -- TODO should patrons also generate random events?
   random_event()
end

function approach_scavengers ()
   vn.clear()
   vn.scene()
   local scavA = vn.newCharacter( minerva.scavengera.name,
         { image=minerva.scavengera.image, color=minerva.scavengera.colour, pos="left" } )
   --[[local scavB =]] vn.newCharacter( minerva.scavengerb.name,
         { image=minerva.scavengerb.image, color=minerva.scavengerb.colour, pos="right" } )
   vn.transition()
   vn.na(_([[The scavengers fall silent as soon as they notice your presence.]]))
   scavA(_([["What are you looking at?"]]))
   vn.done()
   vn.run()
end

function approach_maikki ()
   vn.clear()
   vn.scene()
   local maikki = vn.newCharacter( minerva.vn_maikki() )
   --local kex = minerva.vn_kex{ pos=0, rotation=30*math.pi/180., shader=love_shaders.aura() }
   local kex = minerva.vn_kex{ pos=0, rotation=30*math.pi/180. }
   vn.music( minerva.loops.maikki )
   vn.transition("hexagon")
   vn.na(_("You find Maikki, who beams you a smile as you approach."))

   maikki(_([["Did you find anything new?"]]))
   vn.label("menu")
   vn.menu( function ()
      local opts = {
         { _("Leave"), "leave" },
      }
      if player.evtDone("Chicken Rendezvous") then
         if player.misnDone("Kex's Freedom 5") then
            table.insert( opts, 1, { _("Talk about Kex"), "nokex" } )
         else
            table.insert( opts, 1, { _("Talk about Kex"), "kex" } )
         end
      end
      --table.insert( opts, 1, { _("Ask about her father"), "memory" } )
      return opts
   end)
   vn.label("menu_msg")
   maikki(_([["Anything else?"]]))
   vn.jump("menu")

   vn.label("memory")
   vn.na(_("You ask to see if she remembered anything else about her father."))
   maikki("TODO")
   vn.jump("menu_msg")

   vn.label("kex")
   vn.appear(kex, "slideright")
   vn.na(_("As you are about to talk, you notice Kex out of the corner of your eye."))
   kex(_([[As he stares directly at you, he makes a gesture that you should watch your back.]]))
   vn.na(_("You decide against telling Maikki anything. It does not seem like it is the time, unless you wish to get murdered by a rampant cyborg duck."))
   vn.disappear(kex, "slideright") -- played backwards so slides left
   vn.jump("menu_msg")

   vn.label("nokex")
   vn.na(_("You are about to talk, but decide not to. Now is not the time to deal with this."))
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.done()
   vn.run()
end


--[[
-- Event is over when player takes off.
--]]
function leave ()
   evt.finish()
end
