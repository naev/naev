--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Wild Space Thurion Intro">
 <unique/>
 <location>enter</location>
 <chance>100</chance>
 <system>Maron</system>
 <cond>not faction.get("Thurion"):known()</cond>
</event>
--]]
--[[
   Introduction to the Thurion

   Player gets hailed and told to land on FD-24
--]]
local vn = require "vn"
local vni = require "vnimage"
local fmt = require "format"
local tut = require "common.tutorial"
local strmess = require "strmess"
local thn = require "common.thurion" -- codespell:ignore thn

local landspb, landsys = spob.getS("FD-24")
local fthurion = faction.get("Thurion")

function create ()
   if not evt.claim( system.cur() ) then evt.finish(false) end

   pilot.clear()
   pilot.toggleSpawn(false)

   hook.timer(15, "timer")
   hook.enter("enter")

   -- Is saved until done
   evt.save(true)
end

function timer ()
   vn.clear()
   vn.scene()
   vn.transition()

   vn.na(fmt.f(_([[As you are flying through {sys}, your comm systems flash briefly. Could this be a power failure?]]),
      {sys=system.cur()}))

   local sai = tut.vn_shipai()
   vn.appear( sai, "electric" )
   sai(fmt.f(_([[Your ship AI {shipai} materializes before you.
"{player}, it seems like we are currently undergoing a strong electromagnetic attack. I have rerouted power to cybernetic defenses for now. It is puzzling as we should be in uninhabited area."]]),
      {shipai=tut.ainame(), player=player.name()}))
   vn.menu{
      {_([["Scan for anomalies!"]]), "cont01_scan"},
      {_([["Do a counter electromagnetic attack!"]]), "cont01_counter"},
      {_([["Shut down communication systems!"]]), "cont01_shutdown"},
   }

   vn.label("cont01_scan")
   sai(_([["Scan is negative. No anomalies detected with heavy interference."]]))
   vn.jump("cont01")

   vn.label("cont01_counter")
   sai(_([["A lock-on is necessary first for countermeasures. Scans show no anomalies."]]))
   vn.jump("cont01")

   vn.label("cont01_shutdown")
   sai(_([["I would warn against a full shutdown as it will hamper our detection capabilities."]]))
   vn.jump("cont01")

   vn.label("cont01")
   sai(_([["Wait, I am getting some sort of transmission. Let me attempt to decode the signal."]]))

   vn.move( sai, "left" )
   local signal = vni.textonly(p_("signal","0"), {pos="right"})
   vn.appear( signal )

   signal(strmess.generate( {"0","1"}, 128 ))
   sai(_([["It seems to be using a more archaic encoding. Let try some other decoders."]]))
   signal( strmess.generate( {"0","1"}, rnd.rnd(16,64)).."\n"
         ..strmess.generate( {"0","1"}, rnd.rnd(16,64)).."\n"
         ..strmess.generate( {"0","1"}, rnd.rnd(16,64)).."\n"
         ..strmess.generate( {"0","1"}, rnd.rnd(16,64)) )
   sai(_([["..."]]))
   signal(strmess.generate( {"0","1"}, 32 ).._([["P01EAS101110100DENTIFY YOURSELF OR WE WILL OPEN FIRE. THIS IS YOUR LAST WARNING."]]))
   vn.me(fmt.f(_([["This is the {shipname}. We wish no hostilities, please identify yourself."]]),
      {shipname=player.pilot():name()}))
   signal(fmt.f(_([["MY NAME IS ALICE. I APPLAUD YOUR BRAVERY TO ENTER THE DEEP NEBULA. PROCEED TO {spb} FOR INSPECTION. DO NOT PLAY WITH FIRE AND YOU SHALL NOT BE BURNT."]]),
      {spb=landspb}))
   sai(_([["We have received the coordinates to what seems to be a celestial body. I still detect lock-ons on the ship. I would suggest we do as Alice says."]]))
   vn.na(_([[You punch in the coordinates into your navigation system. Time to see what is going on here.]]))

   vn.run()

   if fthurion:reputationGlobal() < 0 then
      return
   end

   naev.missionStart("Wild Space Thurion Intro - Helper")
   hook.land("land")
end

function land ()
   if spob.cur()==landspb then
      vn.clear()
      vn.scene()
      vn.transition()

      vn.na(fmt.f(_([[When you do your final approach on {spb}, your ship is guided by the local authorities, whomever they may be, to a specific landing port. On the way, you see many strange landing ports that seem to have no life support, likely for drones or other unmanned vehicles, although they are quite large.]]),
         {spb=landspb}))
      vn.na(_([[Eventually your ship reaches the far side of the station, and you are told to wait as preparations are undertaken. You idle by the airlock unsure of what is going to happen.]]))

      -- TODO image
      local alice = thn.vn_drone(_("Alice")) -- codespell:ignore thn
      vn.appear{ alice }
      vn.na(_([[After a short while, you hear the other side of the airlock pressurize, and the door opens. To your surprise, you see a drone flanked by two security robots. Fully automated station?]]))
      alice(fmt.f(_([[The drone in the centre's speaker begins to emit sound, in standard tongue.
"Hello captain of the {shipname}. We finally meet. My name is Alice."]]),
         {shipname = player.pilot():name()}))
      vn.menu{
         {_([["Are you... an artificial intelligence?"]]), "cont01"},
         {_([["Why the remote drones?"]]), "cont01_drone"},
         {_([["What is this place?"]]), "cont01_place"},
      }

      vn.label("cont01_place")
      alice(_([["You will find out about this place soon enough. However, let me start by explaining a bit about myself."]]))
      vn.jump("cont01")

      vn.label("cont01_drone")
      alice(_([["A very common misunderstanding. Let me clarify."]]))
      vn.jump("cont01")

      vn.label("cont01")
      alice(_([["It may be confusing at first, but I have no physical body, yet I am not artificial intelligence. I am an Uploaded. Human made digital."]]))
      vn.menu{
         {_([["Impossible!"]]), "cont02"},
         {_([["How?"]]), "cont02"},
      }

      vn.label("cont02")
      alice(_([[You hear a digital chuckle.
"It is not a process for the faint of heart, as the host body is lost. However, you become free of your physical shackles. Immortal if you wish, infinitely flexible. There are many of us here, although not everyone is able to make the journey."]]))
      alice(_([["We have been watching you since you came into the Nebula. Although some of us were wary, we decided to give you a chance. We have been hiding for too long, and it is time to learn about what is happening in the rest of the galaxy."]]))
      alice(_([["We'll have more time to talk. However, we must first go through a routine interrogation to convince the others. Please, this way."]]))
      vn.na(_([[The drone and escorts lead you to a new room, where you enter alone. There is a single seat in the middle of the room, with the floors and walls being some sort of matte white. The lighting is very uniform, and the room lacks any distinguishing features. You take your seat at the centre.]]))
      vn.disappear{ alice }
      vn.na(_([[During what seems to be a few periods, you are asked many mundane questions by a robotic sounding voice about your upbringing, life experiences, and thoughts. Seemingly inoffensive and somewhat random, you give mainly true answers. Given the way the voice slightly changes the way it speaks, you suspect it is actually more than one individual, however, you are not certain.]]))
      vn.na(_([[Eventually, the questions end, and you are left alone for a while. Eventually, you are once again greeted by a drone, this time not flanked by escorts.]]))
      vn.appear{ alice }
      alice(_([[The drone speaks and you recognize the familiar voice.
"Hello again. It seems like there was no problem with the other interrogators. You are now free to travel among the Thurion."]]))
      vn.menu{
         {_([["That's it?"]]), "cont03_done"},
         {_([["Thurion?"]]), "cont03_thurion"},
      }

      vn.label("cont03_done")
      alice(_([["Yes, I can not shed light on the details, but we have no reason to worry about your presence."]]))
      vn.jump("cont03")

      vn.label("cont03_thurion")
      alice(_([["I may have forgotten to introduce ourselves."]]))
      vn.jump("cont03")

      vn.label("cont03")
      alice(_([["Let me formally introduce ourselves. We are the Thurion, once destined to be a Great House of the Empire, but, as history played out, ended up being outcast and nearly exterminated."]]))
      alice(_([["Although most of our members are uploaded, we still have those who were not able to or not willing to make the change among our ranks. Not everyone is compatible with uploading. Our society has room for all, and now even room for you!"]]))
      alice(_([["Before I let you go along your business, do you have any questions?"]]))

      vn.label("questions_menu")
      vn.menu{
         {_([["What's it like being uploaded?"]]), "q_uploaded"},
         {_([["Why are you hiding in the Nebula?"]]), "q_nebula"},
         {_([["Can I be uploaded?"]]), "q_upload"},
         {_([["What should I do now?"]]), "q_what"},
         {_([[No more questions.]]), "q_end"},
      }
      vn.label("questions")
      alice(_([["Any other questions?"]]))
      vn.jump("questions_menu")

      vn.label("q_uploaded")
      alice(_([["You get used to it very fast. It's a lot like being plugged into a virtual reality set all the time. All the information is available to you all the time. It is also nice to be able to borrow bodies when you need to have physical interactions. I don't miss the headaches and other bodily aches at all!"]]))
      vn.jump("questions")

      vn.label("q_nebula")
      alice(_([["The Uploaded were almost exterminated when the Empire decided to shut down the Great Project Thurion, as we were deemed a threat. Those that managed to survive, laid low for many cycles. When what you call the Incident happened, we saw it as a chance to grow. Since we survived largely unscathed thanks to our biological nature. Many survivors also joined our ranks."]]))
      alice(_([["Our time of hiding is now likely coming to an end. We wish to show the universe the benefits of our ways and spread our peace and harmony. It is much easier to get along when you are digital."]]))
      vn.jump("questions")

      vn.label("q_what")
      alice(_([["I would recommend you see other Thurion stations and worlds. I believe you'll find our society very welcoming and warming. Most stations should have life support systems for non-uploaded too."]]))
      vn.jump("questions")

      vn.label("q_upload")
      alice(_([["I'm glad to see you are so interested. That is a good question! We do not get many new uploads these days, but you would have to check if you are compatible. As long as you haven't had traumatic brain injuries you should be able to get uploaded fine!"]]))
      vn.jump("questions")

      vn.label("q_end")
      alice(_([["I hope you enjoy your stay among the Thurion and look forward to hearing about your progress in the near future!"]]))
      vn.na(_([[With that, the drone takes its leave, and you are finally alone, without escorts. You are now free to explore Thurion space.]]))

      vn.run()

      fthurion:setKnown(true)
      evt.finish(true)
   end
end

function enter ()
   if system.cur()==landsys then
      -- Let the player land on the spob
      landspb:landAllow( true )
   end
end
