--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Minerva Station Gambling">
 <trigger>land</trigger>
 <chance>100</chance>
</event>
--]]

--[[
-- Event handling the gambling stuff going on at Minerva station
--]]

local portrait = require "portrait"
local vn = require 'vn'
local ngettext = gettext.ngettext

-- NPC Stuff
gambling_priority = 3
terminal_name = _("Terminal")
terminal_portrait = "none" -- TODO replace
terminal_desc = _("A terminal with which you can check your current token balance and buy items with tokens.")
blackjack_name = _("Blackjack")
blackjack_portrait = "none" -- TODO replace
blackjack_desc = _("Seems to be one of the more popular card games where you can play blackjack against a \"cyborg chicken\".")
chuckaluck_name = _("Chuck-a-luck")
chuckaluck_portrait = "none" -- TODO replace
chuckaluck_desc = _("A fast-paced luck-based betting game using dice. You can play against other patrons.")
greeter_portrait = portrait.get() -- TODO replace?

function create()
   -- Create NPCs
   npc_terminal = evt.npcAdd( "terminal", terminal_name, terminal_portrait, terminal_desc, gambling_priority )
   npc_blackjack = evt.npcAdd( "blackjack", blackjack_name, blackjack_portrait, blackjack_desc, gambling_priority )
   npc_chuckaluck = evt.npcAdd( "chuckaluck", chuckaluck_name, chuckaluck_portrait, chuckaluck_desc, gambling_priority )

   -- If they player never had tokens, it is probably their first time
   if not var.peek( "minerva_tokens" ) then
      hook.land( "enterbar", "bar" )
   end
   -- End event on takeoff.
   hook.takeoff( "leave" )
end

function tokens_get()
   local v = var.peek( "minerva_tokens" )
   if v == nil then
      return 0
   end
   return v
end

function tokens_pay( amount )
   local v = tokens_get()
   var.push( "minerva_tokens", v+amount )
end

function enterbar()
   vn.clear()
   vn.scene()
   local g = vn.newCharacter( _("Greeter"),
         { image=portrait.getFullPath( greeter_portrait ) } )
   vn.fadein()
   vn.na( _("As soon as you enter the spaceport bar, a neatly dressed individual runs up to you and hands you a complementary drink. It is hard to make out what he is saying over all the background noise created by other patrons and gambling machines, but you try to make it out as best as you can.") )
   g:say( _("\"Welcome to the Minerva Station resort! It appears to be your first time here. As you enjoy your complementary drink, let me briefly explain to you how this wonderful place works. It is all very exciting!\"") )
   g:say( _("\"The currency we use on this station are Minerva Tokens. Unlike credits, they are physical and so very pretty! You can not buy Minerva Tokens directly, however, by participating and betting credits in the various fine games available, you can obtain Minerva Tokens. When you have enough Minerva Tokens, you are able to buy fabulous prizes and enjoy more exclusive areas of our resort. To start out your fun Minerva Adventure®, please enjoy these 100 complementary Minerva Tokens!\"") )
   g:say( _("\"If you want more information or want to check your balance. Please use the terminals located throughout the station. And remember, 'life is short, spend it at Minerva Station'®!\"") )
   vn.fadeout()
   vn.run()

   tokens_pay( 100 )

   --var.push( "minerva_firsttime", true )
end

function terminal()
   local msgs = {
      _(" TODAY MIGHT BE YOUR LUCKY DAY."),
      _(" THIS IS SO EXCITING."),
      _(" YOU SEEM LIKE YOU MIGHT ENJOY A GAME OF BLACKJACK."),
      _(" FORTUNE FAVOURS THE PERSISTENT."),
      _(" LIFE IS SHORT, SPEND IT AT MINERVA STATION."),
   }
   vn.clear()
   vn.scene()
   local t = vn.newCharacter( terminal_name,
         { image=portrait.getFullPath(terminal_portrait), color={0.8, 0.8, 0.8} } )
   vn.fadein()
   vn.label( "start" )
   t:say( function() return string.format(
         ngettext("\"VALUED CUSTOMER, YOU HAVE \ap%d MINERVA TOKEN\a0.%s\n\nWHAT DO YOU WISH TO DO TODAY?\"",
                  "\"VALUED CUSTOMER, YOU HAVE \ap%d MINERVA TOKENS\a0.%s\n\nWHAT DO YOU WISH TO DO TODAY?\"", tokens_get()),
               tokens_get(), msgs[rnd.rnd(1,#msgs)]) end )
   vn.menu( {
      {_("Information"), "info"},
      {_("Trade-in"), "trade"},
      {_("Leave"), "leave"},
   } )
   vn.label( "info" )
   t:say( _("\"I AM PROGRAMMED TO EXPLAIN ABOUT THE WONDERFUL MINERVA STATION GAMBLING FACILITIES. WHAT WOULD YOU LIKE TO KNOW ABOUT?\"") )
   vn.jump( "info_menu" )
   vn.label( "more_info" )
   t:say( _("\"WHAT ELSE WOULD YOU LIKE TO KNOW?\"") )
   vn.label( "info_menu" )
   vn.menu( {
      {_("Station"), "info_station"},
      {_("Gambling"), "info_gambling"},
      {_("Trade-in"), "info_trade"},
      {_("Cyborg Chicken"), "info_chicken"},
      {_("Back"), "start"},
   } )
   vn.label( "info_station" )
   t:say( _("\"MINERVA STATION IS THE BEST PLACE TO SIT BACK AND ENJOY RELAXING GAMBLING ACTIVITIES. ALTHOUGH THE AREA IS HEAVILY DISPUTED BY THE ZA'LEK AND DVAERED, REST ASSURED THAT THERE IS LESS THAN A 2% OF CHANCE OF TOTAL DESTRUCTION OF THE STATION.\"") )
   vn.jump( "more_info" )
   vn.label( "info_gambling" )
   t:say( _("\"WHILE GAMBLING IS NOT ALLOWED IN MOST OF THE EMPIRE, MINERVA STATION BOASTS OF AN EXCLUSIVE STATUS THANKS TO THE IMPERIAL DECREE 289.78 ARTICLE 478 SECTION 19 ALLOWING GAMBLING TO BE ENJOYED WITHOUT RESTRICTIONS. IT IS POSSIBLE TO PLAY GAMES USING CREDITS TO OBTAIN MINERVA TOKENS THAT CAN BE TRADED IN FOR GOODS AND SERVICES ANY TERMINAL THROUGHOUT THE STATION.\"" ) )
   vn.jump( "more_info" )
   vn.label( "info_trade" )
   t:say( _("\"IT IS POSSIBLE TO TRADE MINERVA TOKENS FOR GOODS AND SERVICES AT TERMINALS THROUGHOUT THE STATION. THANKS TO THE IMPERIAL DECREE 289.78 ARTICLE 478 SECTION 72, ALL TRADE-INS ARE NOT SUBJECT TO STANDARD IMPERIAL LICENSE RESTRICTIONS. FURTHERMORE, THEY ALL HAVE 'I Got This Sucker at Minerva Station' ENGRAVED ON THEM.\"") )
   vn.jump( "more_info" )
   vn.label( "info_chicken" )
   t:say( _("\"CYBORG CHICKEN IS OUR MOST POPULAR BLACKJACK DEALER. NO WHERE ELSE IN THE UNIVERSE WILL YOU BE ABLE TO PLAY CARD GAMES WITH AN AI-ENHANCED CHICKEN CYBORG. IT IS A ONCE AND A LIFE-TIME CHANCE THAT YOU SHOULD NOT MISS.\"") )
   vn.jump( "more_info" )

   vn.label( "trade_notenough" )
   t:say( function() return string.format(
         ngettext("\"SORRY, YOU DO NOT HAVE ENOUGH MINERVA TOKENS TO TRADE-IN FOR YOUR REQUESTED ITEM. WOULD YOU LIKE TO TRADE-IN FOR SOMETHING ELSE? YOU HAVE \ap%d MINERVA TOKEN\a0.\"",
                  "\"SORRY, YOU DO NOT HAVE ENOUGH MINERVA TOKENS TO TRADE-IN FOR YOUR REQUESTED ITEM. WOULD YOU LIKE TO TRADE-IN FOR SOMETHING ELSE? YOU HAVE \ap%d MINERVA TOKENS\a0.\"", tokens_get()),
         tokens_get() ) end )
   vn.jump( "trade_menu" )
   vn.label( "trade" )
   t:say( function() return string.format(
         ngettext("\"YOU CAN TRADE IN YOUR PRECIOUS \ap%d MINERVA TOKEN\a0 FOR THE FOLLOWING GOODS.\"",
                  "\"YOU CAN TRADE IN YOUR PRECIOUS \ap%d MINERVA TOKENS\a0 FOR THE FOLLOWING GOODS.\"", tokens_get()),
            tokens_get() ) end )
   local trades = {
      {"Laser Cannon", 500}
   }
   local handler = function (idx)
      -- Jump in case of 'Back'
      if type(idx)=="string" then
         vn.jump(idx)
         return
      end
   end
   local opts = {}
   for k,v in ipairs(trades) do
      opts[k] = { string.format(_("%s (%d Tokens)"), v[1], v[2]), k }
   end
   table.insert( opts, {_("Back"), "start"} )
   vn.label( "trade_menu" )
   vn.menu( opts, handler )
   vn.jump( "start" )

   vn.label( "leave" )
   vn.fadeout()
   vn.run()
end

function blackjack()
   vn.clear()
end

function chuckaluck()
end

--[[
--    Event is over when player takes off.
--]]
function leave ()
   evt.finish()
end
