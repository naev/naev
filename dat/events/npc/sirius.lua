--local fmt = require "format"
--local portrait = require "portrait"
local npc = require "common.npc"
local vni = require "vnimage"

-- State. Nothing persists.
local msg_combined

local desc_fyrra = {
   _("A Sirius civilian of the low Fyrra echelon."),
}
local desc_shiara = {
   _("A Sirius civilian of the mid Shiara echelon."),
}
local desc_serra = {
   _("A Sirius civilian of the high Serra echelon."),
}

local msg_lore = {
   _([["Greetings, traveler. May Sirichana's wisdom guide you as it guides me."]]),
   _([["I once met one of the Touched in person. Well, it wasn't really a meeting, our eyes simply met… But that instant alone was awe-inspiring."]]),
   _([["They say Sirichana lives and dies like any other man, but each new Sirichana is the same as the last. How is that possible?"]]),
   _([["My cousin was called to Mutris a cycle ago… He must be in Crater City by now. And one day, he will become one of the Touched!"]]),
   _([["Some people say Sirius society is unfair because our echelons are determined by birth. But even though we are different, we are all followers of Sirichana. Spiritually, we are equal."]]),
   _([["House Sirius is officially part of the Empire, but everyone knows that's only true on paper. The Emperor has nothing to say in these systems. We follow Sirichana, and no-one else."]]),
   _([["You can easily tell the different echelons apart. Every Sirian citizen and soldier wears clothing appropriate to his or her echelon."]]),
   _([["I hope to meet one of the Touched one day!"]]),
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
   local cur, scur = spob.cur()
   local presence = scur:presences()["Sirius"] or 0
   local tags = cur:tags()

   local w = 0
   if cur:faction() == faction.get("Sirius") then
      w = 1
   elseif presence>0 then
      w = 0.1
   end

   -- Need positive weight
   if w <= 0 then
      return nil
   end

   -- Don't appear on restricted assets
   if tags.restricted then
      -- TODO military personnel
      return nil
   end

   -- Create a list of conditional messages
   msg_combined = npc.combine_cond( msg_cond )

   local function gen_npc()
      -- TODO better way of choosing echelon, maybe spob tags?
      local name, desc, image, prt
      local r = rnd.rnd()
      if r < 0.6 then
         name = _("Fyrra Civilian")
         desc = desc_fyrra[ rnd.rnd(1,#desc_fyrra) ]
         image, prt = vni.sirius.fyrra()
      elseif r < 0.9 then
         name = _("Shiara Civilian")
         desc = desc_shiara[ rnd.rnd(1,#desc_shiara) ]
         image, prt = vni.sirius.shiara()
      else
         name = _("Serra Civilian")
         image, prt = vni.sirius.serra()
         desc = desc_serra[ rnd.rnd(1,#desc_serra) ]
      end
      local msg
      r = rnd.rnd()
      if r <= 0.45 then
         msg = getMessageLore()
      elseif r <= 0.7 then
         msg = getMessage( npc.msg_tip )
      else
         msg = getMessage( msg_combined )
      end
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg }
   end

   return { create=gen_npc, w=w }
end
