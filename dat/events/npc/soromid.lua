--local fmt = require "format"
local portrait = require "portrait"
local npc = require "common.npc"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("A new human civilian is twirling their drink."),
   _("A genetic-modified individual is idle at the bar."),
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

local gfx_list = {
   "soromid/soromid_heavy_civilian_1.webp",
   "soromid/soromid_heavy_civilian_2.webp",
}

local msg_lore = {
   _("Hello. Can I interest you in one of our galaxy famous cosmetic gene treatments? You look like you could use them…"),
   _([["Can you believe it? I was going to visit Sorom to find my roots, and then boom! It got burnt to a crisp! Even now, cycles later, I still can't believe it."]]),
   _([["Everyone knows that we Soromid altered ourselves to survive the deadly conditions on Sorom during the Great Quarantine. What you don't hear so often is that billions of us died from the therapy itself. We paid a high price for survival."]]),
   _([["Our cosmetic gene treatments are even safer now for non-Soromids, with a rate of survival of 99.4%!"]]),
   _([["We have been rebuilding and enhancing our bodies for so long, I say we've become a new species, one above human."]]),
   _([["The Soromid have been shunned and ridiculed by the Great Houses since inception. Over time they have had no choice but to learn to respect us, however, treatment as equals seems still far away."]]),
   _([["It feels good to be free of Empire interference. If we, the Soromid, were a Great House like the Dvaered or Za'lek, we would have much less freedom than what we enjoy today!"]]),
   _([["I once went to Empire territory and got stared at and ridiculed for my looks. The Empire is such a backwards society. Probably all the paperwork and bureaucracy is rotting their brains."]]),
   _([["We sometimes get Za'lek researchers trying to find out the science behind our bioships. No matter how much they analyze them, they can never reproduce anything. They can't seem to understand that bioships are more of an art than a science!"]]),
   _([["The Empire laughs at our bioships and calls us brutish cattle herders. However, they seem to laugh a lot less when their cruisers get bitten in half by a Soromid capship!"]]),
   _([["I've heard of some bioships going missing during creation. It must be the will of the universe."]]),
   _([["We Soromid are split into tribes, each with their own traditions and customs. Tribes are mainly independent, but  Elder convene with each other periodically to decide what to do with things that affect us all."]]),
   _([["I have no idea how the Great Houses can organize them as they do. How can a single person manage and be in charge of billions and billions of humans? It makes no sense!"]]),
   _([["There are those who wished we had all perished on Sorom, however, the Soromid do not go down without a fight!"]]),
   _([["We never meant to become new humans, but when you are faced with the annihilation of your people, you have to take decisive action to save as many as you can."]]),
   _([["We do not easily forget the treatment the Empire gave us during the Great Quarantine. We will make sure that does not happen again."]]),
   _([["We have many stories about the Great Quarantine. It is important to not forget our past as we strive for a better future."]]),
   _([["Some think that making the Soromid inhabit the barely inhabitable is a sign of inferiority to the Great Houses. However, we see it as our pride and accomplishments. Let the Great Houses fight over their few worlds while we live harmonious with nature!"]]),
}

local msg_tip = {
   _([["Soromid Bioships are living creatures. If you train them properly they can learn to do all sorts of new tricks!"]]),
   _([["If you screw up training your bioship you can always reset the skills. This comes at an experience penalty though."]]),
   _([["Plasma weapons are not like other weapons. Instead of only directly damaging ships, most of the damage comes from a corrosive effect that takes place over a few seconds!"]]),
   _([["If used properly, plasma weapons are really good at taking down enemy ships. They do lack range and are hard to hit with, but if you get up close, the corrosion will melt your enemies away!"]]),
}

local msg_cond = {
   { function () return (player.chapter()=="0") end, _([["They are building something big in Feye and apparently need all the rare minerals they can get. If you're a good miner, this may be a good way to make some quick credits."]]) },
   { function () return (player.chapter()~="0") end, _([["I hadn't been to Feye in a while and had my mind blown when I saw the new hypergate they built. Apparently it lets you go to far away systems in a single jump."]]) }
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
   local presence = scur:presences()["Soromid"] or 0
   local tags = cur:tags()

   local w = 0
   if cur:faction() == faction.get("Soromid") then
      w = 2
   elseif presence>0 then
      w = 0.1 -- Fewer NPC
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

   -- Add tag-appropriate descriptions
   local descriptions = npc.combine_desc( desc_list, tags )

   local function gen_npc()
      local name = _("Soromid Tribesperson")
      local desc = descriptions[ rnd.rnd(1,#descriptions) ]
      local prt  = portrait.get( "Soromid" )
      local image = portrait.getFullPath( prt )
      -- TODO probably use tags to control what portraits get used
      if rnd.rnd() < 0.3 then
         prt = gfx_list[ rnd.rnd(1,#gfx_list) ]
         image = prt
      end
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
