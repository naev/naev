--local fmt = require "format"
local npc = require "common.npc"
local vni = require "vnimage"

-- State. Nothing persists.
local msg_combined

local desc_shaira = {
   _("A Sirius civilian of the low Shaira echelon."),
}
local desc_fyrra = {
   _("A Sirius civilian of the mid Fyrra echelon."),
}
local desc_serra = {
   _("A Sirius civilian of the high Serra echelon."),
}

local msg_lore = {
   shaira = {
      _([["I hope to meet one of the Touched one day!"]]),
      _([["They say Sirichana lives and dies like any other man, but each new Sirichana is the same as the last. How is that possible?"]]),
      _([["My cousin was called to Mutris a cycle ago… He must be in Crater City by now. And one day, he will become one of the Touched!"]]),
      _([["The Shaira echelon is the backbone of House Sirius. We keep the ships flying and the economy running. It is an honour to play such an important role for Sirichana!"]]),
      _([["Manual labour fulfils me. I try to make everything with as much care as would be necessary for the Sirichana."]]),
   },
   fyrra = {
      _([["I once met one of the Touched in person. Well, it wasn't really a meeting, our eyes simply met… But that instant alone was awe-inspiring."]]),
      _([["We, the Fyrra echelon, maintain the social order and run most of the commercial operations. It is such to be blessed by the Sirichana."]]),
      _([["Commercial among the Sirius is always prosperous. I wish I could say the same about striking business deals with the Dvaered or Za'lek."]]),
   },
   serra = {
      _([["Some people say Sirius society is unfair because our echelons are determined by birth. But even though we are different, we are all followers of Sirichana. Spiritually, we are equal."]]),
      _([["There is no such greater honour than to serve the Sirichana in the Serra echelon."]]),
   },
}
-- Common stuff added to all echelons
local msg_lore_common = {
   _([["Greetings, traveller. May Sirichana's wisdom guide you as it guides me."]]),
   _([["House Sirius is officially part of the Empire, but everyone knows that's only true on paper. The Emperor has nothing to say in these systems. We follow Sirichana, and no-one else."]]),
   _([["You can easily tell the different echelons apart. Every Sirian citizen and soldier wears clothing appropriate to his or her echelon."]]),
}
for k,v in pairs(msg_lore) do
   for i,m in ipairs(msg_lore_common) do
      table.insert( v, m )
   end
end

--local msg_tip = {}

local msg_cond = {
}

-- Returns a lore message for the given faction.
local function getMessageLore( echelon )
   return msg_lore[echelon][ rnd.rnd(1,#msg_lore[echelon]) ]
end

local function getMessage( lst, echelon )
   if #lst == 0 then
      return getMessageLore( echelon )
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
      local echelon, name, desc, image, prt
      local r = rnd.rnd()
      if r < 0.3 then
         echelon = "shaira"
         name = _("Shaira Acolyte")
         desc = desc_shaira[ rnd.rnd(1,#desc_shaira) ]
         image, prt = vni.sirius.shaira()
      elseif r < 0.9 then
         echelon = "fyrra"
         name = _("Fyrra Acolyte")
         desc = desc_fyrra[ rnd.rnd(1,#desc_fyrra) ]
         image, prt = vni.sirius.fyrra()
      else
         echelon = "serra"
         name = _("Serra Acolyte")
         desc = desc_serra[ rnd.rnd(1,#desc_serra) ]
         image, prt = vni.sirius.serra()
      end
      local msg
      r = rnd.rnd()
      if r <= 0.45 then
         msg = getMessageLore( echelon )
      elseif r <= 0.7 then
         msg = getMessage( npc.msg_tip, echelon )
      else
         msg = getMessage( msg_combined, echelon )
      end
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg }
   end

   return { create=gen_npc, w=w }
end
