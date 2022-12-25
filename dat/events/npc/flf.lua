--local fmt = require "format"
local npc = require "common.npc"
local vni = require "vnimage"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("A FLF fighter."),
   _("A FLF combatant."),
   _("A tired-looking FLF combatant."),
   _("An exhausted FLF fighter."),
   _("An FLF combatant that smells like ship oil."),
   _("A grease-covered FLF militant."),
}

local msg_lore = {
   _([["I can't stand Dvaereds. I just want to wipe them all off the map. Don't you?"]]),
   _([["One of these days, we will completely rid the Frontier of Dvaered oppressors. Mark my words!"]]),
   _([["Have you ever wondered about our chances of actually winning over the Dvaereds? Sometimes I worry a little."]]),
   _([["I was in charge of a bombing run recently. The mission was a success, but I lost a lot of comrades. Oh well… this is the sacrifice we must make to resist the oppressors."]]),
   _([["What after we beat the Dvaereds, you say? Well, our work is never truly done until the Frontier is completely safe from oppression. Even if the Dvaered threat is ended, we'll still have those Sirius freaks to worry about. I don't think our job will ever end in our lifetimes."]]),
   _([["Yeah, it's true, lots of Frontier officials fund our operations. If they didn't, we'd have a really hard time landing on Frontier planets, what with the kinds of operations we perform against the Dvaereds."]]),
   _([["Yeah, some civilians die because of our efforts, but that's just a sacrifice we have to make. It's for the greater good."]]),
   _([["No, we're not terrorists. We're soldiers. True terrorists kill and destroy without purpose. Our operations do have a purpose: to drive out the Dvaered oppressors from the Frontier."]]),
   _([["Riddle me this: how can we be terrorists if the Dvaereds started it by encroaching on Frontier territory? It's the most ridiculous thing I ever heard."]]),
   _([["Well, no, the Dvaereds never actually attacked Frontier ships, but that's not the point. They have their ships in Frontier territory. What other reason could they possibly have them there for if not to set up an invasion?"]]),
}

--local msg_tip = {}

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
   local cur, _scur = spob.cur()
   --local presence = scur:presences()["FLF"] or 0
   local tags = cur:tags()

   -- Only spawn on FLF assets
   -- TODO sympathizers
   if cur:faction() ~= faction.get("FLF") then
      return nil
   end

   -- Don't appear on restricted assets
   if tags.restricted then
      -- TODO military personnel
      return nil
   end

   -- Create a list of conditional messages
   msg_combined = npc.combine_cond( msg_cond )

   -- Add tag-appropriate descriptions
   local descriptions = npc.combine_desc( desc_list, tags )

   local function gen_npc()
      local name = _("FLF Supporter")
      local desc = descriptions[ rnd.rnd(1,#descriptions) ]
      local image, prt = vni.generic()
      local msg
      local r = rnd.rnd()
      if r <= 0.7 then
         msg = getMessageLore()
      else
         msg = getMessage( msg_combined )
      end
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg }
   end

   return { create=gen_npc }
end
