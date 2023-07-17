--local fmt = require "format"
local vni = require "vnimage"
local npc = require "common.npc"
local pir = require "common.pirate"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("A Dreamer clan soul, with a pocketful of star-shaped stickers, playfully distributing them to fellow bar patrons as a symbol of interconnectedness."),
   _("A pirate drumming on bongo drums with infectious energy"),
   _("A blissed-out Dreamer practicing meditative yoga poses in a corner."),
   _("A pirate creating abstract artwork on a portable canvas."),
   _("A Dreamer clan soul who seems to fade in and out of the concious reality."),
   _("A pirate that may or may have not taken too many illicit substances."),
   _("A Dreamer clansperson surrounded by a cloud of fragrant smoke"),
   _("A laid-back Dreamer soul with a serene smile."),
   _("A pirate who almost seems to be in a trance-like state."),
   _("A Dreamer soul with a peaceful smile and faraway look in their eyes."),
}

local msg_lore = {
   _([["I'm still adapting to the Dreamer Clan, but I love how they call everyone soul. You're a soul, I'm a soul, they're a soul. We are all one with the universe!"]]),
   _([["I used to be a devote believer of Sirichana, but I only truly learned to open my mind when I joined the Dreamer clan."]]),
   _([["All the rigidness and bad karma of the Great houses is really putting a hamper on human potential. As if humans were meant to spend all day locked in a cubicle processing paperwork!"]]),
   _([["I used to think that property was like a thing, you know? Now I know it's all an arbitrary mechanism to control us, like sheep! Being a pirate gives you a whole new perspective on this property stuff."]]),
   _([["If you're stealing from an indoctrinated wage-slaving zombie, is that ethically wrong? How can it be if they are not even truly alive? The system must break for them to be free!"]]),
   _([["There's nothing quite like being in the Dreamer clan. Sure we have to raid convoys here and there to make ends meet, but it's all so liberating."]]),
   _([["They say the Dreamer clan is inefficient because we have no hierarchy. Sure, we take a while to get set up as we have to do our voting, but no other pirate clan has seen a growth like ours!"]]),
   _([["I was taught growing up that anarchism was bad and order was good, it was, like, all indoctrination, soul! With the Dreamer clan collective harmony, anarchism is the only way!"]]),
   _([["Some of the other pirate clans don't think we're a real clan as we don't have a leader, however, our freedom is our strength! We can bounce back from anything!"]]),
}

local msg_tip = {
   _([["Dvaered ships have lots of armour and can be very hard to crack. Using torpedoes is a good way to bring them down."]]),
   _([["You have to be careful with Sirius ships, they can have psychic powers that can mess you up bad. If you get hit, your ship can get slowed down!"]]),
}

local msg_cond = {
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
   local presence = scur:presences()["Dreamer Clan"] or 0

   if not pir.factionIsPirate( cur:faction () ) then
      return nil
   end

   local w = 0
   if cur:faction() == faction.get("Dreamer Clan") then
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
      local name = _("Dreamer Soul")
      local desc = descriptions[ rnd.rnd(1,#descriptions) ]
      local image, prt = vni.pirate()
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
