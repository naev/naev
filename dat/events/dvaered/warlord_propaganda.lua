--[[
<?xml version='1.0' encoding='utf8'?>
 <event name="Deliver Flyers">
  <unique />
  <location>land</location>
  <chance>30</chance>
  <cond>spob.cur():faction() == faction.get("Dvaered") and var.peek("dc_misn") ~= nil and var.peek("dc_misn") &gt;= 2</cond>
  <notes>
   <campaign>Dvaered Recruitment</campaign>
   <done_misn name="Dvaered Census">2 times or more</done_misn>
  </notes>
 </event>
 --]]
--[[
--Event for Dvaered Recruitment arc: the player meets a Dvaered soldier at a bar, who unlocks for them the Dvaered Propaganda missions.
--]]

local fmt      = require "format"
local vn       = require 'vn'
local vni      = require 'vnimage'

local img, port = vni.dvaeredMilitary()

function create()
   evt.npcAdd("approach", _("Dvaered soldier"),  port, _("A Dvaered soldier with many bruises on their face is looking at you."))
   hook.takeoff("takeoff")
end

function approach()

   vn.clear()
   vn.scene()
   local sol = vn.newCharacter( _("Dvaered soldier"), { image=img } )
   local doaccept = false

   vn.transition( "hexagon" )
   sol(fmt.f(_([["Hello, citizen {player}, I am a member of the Warlords Affairs Office at the Dvaered High Command. We have remarked that you have completed missions for the Office before. I have been charged to propose you to get registered as a potential pilot for more challenging and dangerous missions. If you accept, the missions will be available at your mission computer from now on."]]), {player=player.name()}))
   vn.jump( "menu" )

   vn.label("menu")
   sol(_([["So, what do you say?"]]))
   vn.menu{
      {_("Please add those missions to my computer."), "accept"},
      {_("I'm not interested"), "decline"},
      {_("What is it about?"), "info"},
      {_("What happened to your face?"), "bruises"},
   }

   vn.label("decline")
   sol(_([["Ah! I knew it! You refuse because I said it is dangerous! You're a coward, as shown by the disturbing absence of bruises on your face!"]]))
   vn.func( function () doaccept = false end )
   vn.done( "hexagon" )

   vn.label("accept")
   sol(_([["Very well, citizen. I am sure you will help us at the office a lot."]]))
   vn.func( function () doaccept = true end )
   vn.done( "hexagon" )

   vn.label("bruises")
   sol(_([["I simply got hit multiple times by someone. Isn't that obvious?"]]))
   vn.jump( "menu" )

   vn.label("info")
   sol(_([["Warlords, as you know, are successful generals who have been entrusted with regalian sovereignty on Dvaered planets. Warlords are allowed to declare war to their rivals if they were offended or in order to conquer their planets."]]))
   sol(_([["I will not explain in detail how the Dvaered internal wars work, but you have to know that they are often divided into a space campaign and a ground campaign. The space campaign usually takes only a few periods, while ground campaign can take an entire cycle. And during ground campaigns, for many strategic reasons, it is necessary for each warlord to keep the support of the planet's population."]]))
   sol(_([["This is where private pilots are used: a Warlord can require to the Warlords Affairs Office to hire a private pilot for him to spread a few tonnes of propaganda flyers in the atmosphere of a given planet. And the more tonnes of flyers you can carry, the more you will get paid."]]))
   sol(_([["On the technical point of view, all you have to do is to fly over the planet, and the posters will be automatically dropped."]]))
   sol(_([["However, you have to be aware that after that, the rival of the Warlord who ordered the operation, and maybe also their allies will probably send ships after you. So you have to be ready to dodge Vendettas attacks if you take one of these missions. What is more, landing on the planet where you just dropped your posters is not an option."]]))
   sol(_([["Also note that only the ships of the 'wrong' Warlords will be chasing you, and only in the system where you executed the mission. What is more, it is totally allowed to destroy those ships as it is done in the framework of a loyal war between honourable Warlords.
"Once you get out of the system, you have nothing to fear from Dvaered patrol ships anymore, even if you go back to the afterwards."]]))
   vn.jump( "menu" )

   vn.run()

   -- Manage acceptance and refusal
   if doaccept then
      var.push("dp_available", true)
      evt.finish(true)
   end
   evt.finish(false)
end

function takeoff()
   evt.finish(false)
end
