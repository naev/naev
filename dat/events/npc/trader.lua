local vni = require "vnimage"

local desc_list = {
   _("A trader enjoying a drink."),
   _("A space trader relaxing at the bar."),
   _("The individual looks like they dabble in trade."),
   -- A bit weird
   _("A hard-boiled space capitalist."),
}

local msg_lore = {
   -- More generic messages
   _([["Just another link in the Great Chain, right?"]]),
   _([["You win some, you lose some, but if you don't try you're never going to win."]]),
   _([["If you don't watch the markets then you'll be hopping between planets in a jury-rigged ship in no time."]]),
   _([["Them blimming pirates, stopping honest folk from making an honest living - it's not like we're exploiting the needy!"]]),
   -- Some Trader Society Lore
   _([["People think that just being a member of the Space Traders Society makes credits fall out of the sky. We still have to work for it!"]]),
   _([["They say that if it weren't for the Astra Vigilus mercenaries, the Empire would have long succumbed to the pirate infestation."]]),
   _([["The Mining Vrata was the main reason humanity was able to expand into space so fast! Space colonies require a lot of resources, and it's much more efficient to pull them straight from nearby asteroids."]]),
   _([["The Space Traders Society is composed of many different organizations. For example, the Mining Vrata deals with asteroid mining and resources, while the Astra Vigilis mercenaries help keep people safe from pirates."]]),
   _([["I used to work on the original Space Trade Hub before it was destroyed in the Incident. I lost many good friends and colleagues that day."]]),
}

-- Get a lore message
local function getMessageLore ()
   return msg_lore[ rnd.rnd(1, #msg_lore) ]
end

return function ()
   local cur, scur = spob.cur()
   local presence = scur:presences()["Traders Society"] or 0
   local fct = cur:faction()
   local w

   -- Need a generic faction
   if not fct or not fct:tags().generic then
      return nil
   end

   -- Need independent presence in the system
   if presence <= 0 then
      return nil
   end

   -- Don't appear on restricted assets
   if cur:tags().restricted then
      return nil
   end

   -- Lower presence on non-trader worlds
   if fct == faction.get("Traders Society") then
      w = 1.5
   else
      w = 0.2
   end

   local function gen_npc()
      -- Append the faction to the civilian name, unless there is no faction.
      local name = _("Trader")
      local desc = desc_list[ rnd.rnd(1,#desc_list) ]
      local image, prt = vni.generic()
      local msg = getMessageLore()
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg }
   end

   return { create=gen_npc, w=w }
end
