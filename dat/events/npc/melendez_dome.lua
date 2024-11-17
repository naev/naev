--local fmt = require "format"
--local npc = require "common.npc"
local vni = require "vnimage"

local desc_list = {
   _("A patron who seems to be enjoying the races."),
   _("An individual sipping a drink while attentively watching the races."),
   _("A care-free individual passing the time."),
}

local msg_lore = {
   _([["It's incredible how fast some of the pilots are going. I can't get anywhere close their speed in my Llama…"]]),
   _([["I want to become a space ship engine mechanic, but I never seem to pass the official test. I just can't memorize all the parts manuals."]]),
   _([["I'm training to be a pilot! Well, the best way to learn is by example, no?"]]),
   _([["I wonder why the station is called the Melendez Dome. It's not anything like a dome, isn't it?"]]),
   -- Some tips?
   _([["Afterburners seem like they can really help on tight curves."]]),
   _([["Lots of pilots seem to fly interceptors, they can really put out speed!"]]),
   _([["The engine makes a ton of difference in a ship's top speed. Tricon engines really seem to pay off."]]),
}

local function getMessageLore ()
   return msg_lore[ rnd.rnd(1, #msg_lore) ]
end

local mdome = spob.get("Melendez Dome")
return function ()
   local cur = spob.cur()
   if cur~=mdome then return nil end

   local function gen_npc()
      -- Append the faction to the civilian name, unless there is no faction.
      local name = _("Race Fan")
      local desc = desc_list[ rnd.rnd(1,#desc_list) ]
      local image, prt = vni.generic()
      local msg = getMessageLore()
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg }
   end

   return { create=gen_npc, w=2 }
end
