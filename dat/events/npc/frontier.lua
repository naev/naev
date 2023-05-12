--local fmt = require "format"
local npc = require "common.npc"
local vni = require "vnimage"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("A citizen of the Frontier."),
   _("An individual that has a solemn gaze."),
   _("This person seems to be very easy going."),
   _("A civilian that looks slightly bored."),
   _("A laid back person relaxing."),
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
   _([["We value our autonomy. We don't want to be ruled by those Dvaered Warlords! Can't they just shoot at each other instead of threatening us? If it wasn't for the Frontier Liberation Front…"]]),
   _([["Have you studied your galactic history? The Frontier worlds were the first to be colonized by humans. That makes our worlds the oldest human settlements in the galaxy, now that Earth is gone."]]),
   _([["We have the Dvaered encroaching on our territory on one side, and the Sirius zealots on the other. Sometimes I worry that in a few decacycles, the Frontier will no longer exist."]]),
   _([["Have you visited the Frontier Museum? They've got a scale model of a First Growth colony ship on display in one of the big rooms. Even scaled down like that, it's massive! Imagine how overwhelming the real ones must have been."]]),
   _([["There are twelve true Frontier worlds, because twelve colony ships successfully completed their journey in the First Growth. But did you know that there were twenty colony ships to begin with? Eight of them never made it. Some are said to have mysteriously disappeared. I wonder what happened to them?"]]),
   _([["We don't have much here in the Frontier, other than our long history leading directly back to Earth. But I don't mind. I'm happy living here, and I wouldn't want to move anywhere else."]]),
   _([["You know the Frontier Liberation Front? They're the guerilla movement that fights for the Frontier. Not to be confused with the Liberation Front of the Frontier, the Frontier Front for Liberation, or the Liberal Frontier's Front!"]]),
   _([["My grandparents used to say that the Frontier was a much harsher place back in the day. Now we are spoiled by Empire and Dvaered technology."]]),
   _([["The only losers in the FLF and Dvaered fighting are us regular civilians. We're the ones getting caught in the crossfire and paying the bills!"]]),
   _([["The Frontier Liberation Front used to be a pacifist movement before it was outlawed and radicalized. Now they're so obsessed with eliminating the Dvaered that they even bomb and attack civilian targets!"]]),
   _([["Sometimes it seems like the Dvaered like to use the FLF as an excuse to advance their plans. It's much easier to enforce lock downs and control laws when there is rampant terrorism."]]),
   _([["You'd think more people would want to emigrate out of the Frontier, but it is hard to leave your home-planet. It's not like there's a much better place to go to."]]),
   _([["The Dvaered economic crisis also affects the Frontier. Lots of young people join the Frontier Liberation Front or Dreamer Clan pirates just to be able to survive."]]),
   _([["The Dvaered conflict has broken many families in the Frontier. Some love the comforts of the Empire, while others abhor the control policies and loss of freedom. Why can't we all just get along?"]]),
   _([["I used to be a big fan of the Frontier Liberation Front, but now they are too violent and unpredictable. Not like the Dvaered are much better though…"]]),
}

--local msg_tip = {}

local msg_cond = {
   { npc.test_misnDone("Destroy the FLF base!"), _([["I heard that the Dvaered blew up a major Frontier Liberation Front base. This makes me uncertain of the future of the Frontier."]]) },
   { function () return (player.chapter()=="0") end, _([["I've heard the Great Houses are building megastructures in space. We don't have anything like that in the Frontier."]])},
   { function () return (player.chapter()~="0") end, _([["They say that a great Hypergate network has gone online. You can travel across the galaxy nearly instantly. We don't have anything like that here in the Frontier…"]])},
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

   if cur:faction() ~= faction.get("Frontier") then
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
      local name = _("Frontier Denizen")
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
