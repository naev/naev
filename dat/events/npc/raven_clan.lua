--local fmt = require "format"
local portrait = require "portrait"
local npc = require "common.npc"
local pir = require "common.pirate"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("A clansperson enjoying some rum."),
   _("A Raven clansperson trader."),
   _("A clansperson merchant pilot."),
   _("A well-kempt Raven clansperson."),
}

local msg_lore = {
   _([["The Raven Clan is the backbone of the pirate clans. We keep everything running!"]]),
   _([["The Wild Ones Clan are said to be very wild and unruly, but they are always quite keen to trade with us."]]),
   _([["It's hard being a grog dealer. All the clans are constantly asking for more!"]]),
   _([["The Empire says they're hard on pirates, but most of their officials are already in our pockets!"]]),
   _([["I hate dealing with the Dreamer Clan, they always forget to keep their promises and it's a pain in the ass to chase them down to get what they owe you."]]),
   _([["I remember when the Black Lotus Clan use to be a small-time mafia. Look at how organized they are now!"]]),
   _([["The pirate assembly is always a great place to make trades. All the grog makes them be less stingy with their wallets!"]]),
}

local msg_tip = {
   _([["Them Empire patrols be lookin' mighty tough, but even when they spot yur contraband, it be easy enough to bribe 'em to look the other way."]]),
}

local msg_cond = {
   { function () return (player.chapter()=="0") end, _([["The Black Lotus have been requesting lots of high tech contraband lately. I think they must be building something fancy."]]) },
}

-- Returns a lore message for the given faction.
local function getMessageLore ()
   return msg_lore[ rnd.rnd(1,#msg_lore) ]
end

local function getMessage( lst )
   if #lst == 0 then
      return getMessageLore()
   end
   return lst[ rnd.rnd(1, #lst) ]
end

return function ()
   local cur, scur = spob.cur()
   local tags = cur:tags()
   local presence = scur:presences()["Raven Clan"] or 0

   if not pir.factionIsPirate( cur:faction () ) then
      return nil
   end

   local w = 0
   if cur:faction() == faction.get("Raven Clan") then
      w = 1
   elseif presence>0 then
      w = 0.2 -- Fewer NPC
   end

   -- Need presence
   if w <= 0 then
      return nil
   end

   -- Create a list of conditional messages
   msg_combined = npc.combine_cond( msg_cond )

   -- Add tag-appropriate descriptions
   local descriptions = npc.combine_desc( desc_list, tags )

   local function gen_npc()
      local name = _("Raven Clan Clansperson")
      local desc = descriptions[ rnd.rnd(1,#descriptions) ]
      local prt  = portrait.get( "Pirate" )
      local image = portrait.getFullPath( prt )
      local msg
      local r = rnd.rnd()
      if r <= 0.45 then
         msg = getMessageLore()
      elseif r <= 0.7 then
         msg = getMessage( msg_tip )
      else
         msg = getMessage( msg_combined )
      end
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg }
   end

   return { create=gen_npc, w=w }
end
