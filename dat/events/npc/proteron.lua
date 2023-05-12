--local fmt = require "format"
local npc = require "common.npc"
local vni = require "vnimage"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("A proteron civilian."),
   _("An individual idly waiting for a drink."),
   _("A determined looking individual."),
   _("A person who looks full of self-confidence."),
   _("An civilian with a strong gaze."),
}
--desc_list["agriculture"] = {}
--desc_list["industrial"] = {}
--desc_list["mining"] = {}
--desc_list["tourism"] = {}
--desc_list["medical"] = {}
--desc_list["trade"] = {}
--desc_list["old"] = {}
--desc_list["immigration"] = {}
--desc_list["prison"] = {}
--desc_list["station"] = {}
--desc_list["government"] = {}

local msg_lore = {
   _([["Our system of government is clearly superior to all others. Nothing could be more obvious."]]),
   _([["The Incident really set back our plan for galactic dominance, but that was only temporary."]]),
   _([["We don't have time for fun and games. The whole reason we're so great is because we're more productive than any other society."]]),
   _([["We are superior, so of course we deserve control over the galaxy. It's our destiny."]]),
   _([["The Empire is weak, obsolete. That is why we must replace them."]]),
   _([["Slaves? Of course we're not slaves. Slaves are beaten and starved. We are in top shape so we can serve our country better."]]),
   _([["I can't believe the Empire continues to allow families. So primitive. Obviously, all this does is make them less productive."]]),
   _([["The exact cause of the Incident is a tightly-kept secret, but the government says it was caused by the Empire's inferiority. I would expect nothing less."]]),
   _([["I came across some heathen a few months back who claimed, get this, that we Proterons were the cause of the Incident! What slanderous nonsense. Being the perfect society we are, of course we would never cause such a massive catastrophe."]]),
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
   local tags = cur:tags()

   -- Need spob faction
   if cur:faction() ~= faction.get("Proteron") then
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
      local name = _("Proteron Citizen")
      local desc = descriptions[ rnd.rnd(1,#descriptions) ]
      local image, prt = vni.generic()
      local msg
      local r = rnd.rnd()
      if r <= 0.45 then
         msg = getMessageLore()
      elseif r <= 0.7 then
         msg = getMessage( npc.msg_tip )
      else
         msg = getMessage( msg_combined )
      end
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg }
   end

   return { create=gen_npc }
end
