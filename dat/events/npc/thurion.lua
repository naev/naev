--local fmt = require "format"
local npc = require "common.npc"
local vni = require "vnimage"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("A non-uploaded Thurion civilian."),
   _("An individual idling around.")
}

local msg_lore = {
   _([["Non-uploaded citizens are known as 'inits'. We have the possibility of initializing an upload, however, in many cases we are not able to upload due to physical problems."]]),
   _([["Did you know that even the slightest bit of brain damage can lead to death during the upload process? That's why we're very careful to not allow our brains to be damaged, even a little."]]),
   _([["One great thing once you're uploaded is that you can choose to forget things you don't want to remember. My great-grandfather had a movie spoiled for him before he could watch it, so once he got uploaded, he deleted that memory and watched it with a fresh perspective. Cool, huh?"]]),
   _([["The best part of our lives is after we're uploaded, but that doesn't mean we lead boring lives before then. We have quite easy and satisfying biological lives as inits before uploading."]]),
   _([["Being uploaded allows you to live forever, but that doesn't mean you're forced to. Any uploaded Thurion can choose to end their own life if they want, though few have chosen to do so."]]),
   _([["Uploading is a choice in our society. No one is forced to do it. It's just that, well, what kind of person would turn down the chance to live a second life on the network and live as an init?"]]),
   _([["We were lucky to not get touched by the Incident. In fact, we kind of benefited from it. The nebula that resulted gave us a great cover and sealed off the Empire from us. It also got rid of those dangerous Proterons."]]),
   _([["We don't desire galactic dominance. That being said, we do want to spread our way of life to the rest of the galaxy, so that everyone can experience the joy of being uploaded."]]),
   _([["I think you're from the outside, aren't you? That's awesome! I've never met a foreigner before. What's it like outside the nebula?"]]),
   _([["We actually make occasional trips outside of the nebula, though only rarely, and we always make sure to not get discovered by the Empire."]]),
   _([["The Soromid have a rough history. Have you read up on it? First the Empire confined them to a deadly planet and doomed them to extinction. Then, when they overcame those odds, the Incident blew up their homeworld. The fact that they're still thriving now despite that is phenomenal, I must say."]]),
   _([["I'm not sure if it's the upload process or the access to the vast information, but many people change character significantly after they get uploaded. Maybe I would change too if I was uploaded!"]]),
   _([["Sometimes I feel like the uploaded treat us inits as pets they have to take care of just because we can't backup ourselves if something goes wrong."]]),
   _([["Many uploaded have backups. If for some reason their ship blows up or their processor blows up, their backups get automatically activated and they can keep on going like nothing happened!"]]),
   _([["Lots of uploaded modify their memory and edit themselves, is this why their personality changes so much after uploading?"]]),
   _([["I never really understood why non-uploaded Thurions were called inits until it was explained to me. It is because we can be seeds to initialize uploads!"]]),
}
--[=[
local msg_lore_uploaded = {
   _([["My father unfortunately hit his head when he was young, so he couldn't be safely uploaded. It's okay, though; he had a long and fulfilling life, for a non-uploaded human, that is."]]),
}
--]=]

local msg_tip = {}

local msg_cond = {}

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
   if cur:faction() ~= faction.get("Thurion") then
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
      local name = _("Thurion Init")
      local desc = descriptions[ rnd.rnd(1,#descriptions) ]
      local image, prt = vni.generic()
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

   return { create=gen_npc }
end
