--local fmt = require "format"
local vni = require "vnimage"
local npc = require "common.npc"
local pir = require "common.pirate"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("A Black Lotus clansperson sipping a sophisticated cocktail."),
   _("A Black Lotus pirate with a sharp wit and a charming smile."),
   _("A Black Lotus clansperson with a subtle hint of intrigue."),
   _("A Black Lotus clansperson skillfully manipulating a deck of cards in a mesmerizing display of precision."),
   _("A Black Lotus pirate elegantly sipping a glass of vintage wine."),
   _("A burly Black Lotus clansperson scanning the bar for trouble."),
   _("A tall Black Lotus clansperson quietly observing the spaceport's comings and goings with a penetrating gaze."),
}

local msg_lore = {
   _([["I thought joining a pirate clan would lead to adventure and flying around the universe. Instead, I spend all my time crunching numbers in a cubicle. Maybe I picked the wrong clan to join."]]),
   _([["Being in the Black Lotus has its perks, health care is much better than what I had back when I lived in the Empire!"]]),
   _([["They say our methods are anti-social and violent, yet the Great Houses get away with incarceration and taxation at a massive scale! Who is the real criminal here?"]]),
   _([["We're the real ones keeping the peace here. If it weren't for us, House Za'lek and House Dvaered would be wiping planets clean and killing billions of people! Humanity should be grateful to us."]]),
   _([["Funny how House Za'lek condemns piracy, yet they always come crawling for us to get them their scientific toys. Talk about double standards."]]),
   _([["We pirates are supposed to be the violent brutes, yet House Dvaered is subjugating everything they can get their hands on by force, including each other!"]]),
   _([["The Black Lotus is all about making use of personal skill. If you work hard enough, you can even become a manager or executive!"]]),
}

local msg_tip = {
   _([["Za'lek drones can quickly overwhelm your ship. It's important to take them down as fast as possible. Disabling them is a good way to get them out of combat and not allow the mothership to launch new ones."]]),
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

local fct = faction.get("Black Lotus")
return function ()
   local cur, scur = spob.cur()
   local tags = cur:tags()
   local presence = scur:presences()["Black Lotus"] or 0

   if not pir.factionIsPirate( cur:faction () ) then
      return nil
   end

   local w = 0
   if cur:faction() == fct then
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
      local name = _("Black Lotus Associate")
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
