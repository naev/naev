--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Majesteka Stowaways Aboard">
 <location>land</location>
 <chance>100</chance>
 <spob>Majesteka II</spob>
 <unique />
</event>
--]]
--[[
   Some kids sneak onto your ship if you land on Majesteka II.
--]]
local neu = require "common.neutral"
local vn = require "vn"
local tut = require "common.tutorial"
local fmt = require "format"

local stowaway_spob = spob.get("Majesteka II")

function create ()
   hook.takeoff("takeoff")
end

local timer_on
function takeoff ()
   if timer_on then return end
   hook.timer( 10 + 30*rnd.rnd(), "found_stowaways" )
   timer_on = true
end

function found_stowaways ()
   local cleanup = false

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   vn.na(fmt.f(_([[You are flying around, minding your own business when suddenly, {shipai} materializes in front of you.]]),
      {shipai=tut.ainame()}))
   sai(fmt.f(_([[For some reason, they are speaking in a very low volume, almost as if whispering.
"{playername}, it seems that we have a situation aboard the ship."]]),
      {playername=player.name()}))
   vn.menu{
      {_([["What is it?"]]), "01_cont"},
      {_([["Did you have another 'accident' with ship oil?"]]), "01_joke"},
      {_([[…]]), "01_cont"},
   }

   vn.label("01_joke")
   sai(_([["Wait, how did you kn…?!?"
They quickly change the topic.
"About the situation aboard the ship."]]))

   vn.label("01_cont")
   sai(_([["I have detected alien lifeforms in the cargo holds! These are completely unrelated to the fungus growing in the spoiled food rations. You should go investigate!"]]))
   vn.disappear( sai, tut.shipai.transition )

   vn.na(fmt.f(_([[You begrudgingly go to investigate the cargo holds. As you are about to open the door, you hear some weird noises. Seems like {ainame} was right this time.]]),
      {ainame=tut.ainame()}))
   vn.menu{
      {_([[Ask who is there]]), "02_ask"},
      {_([[Walk in casually]]), "02_walkin"},
      {_([[Draw your weapon and barge in]]), "02_barge"},
   }

   vn.label("02_ask")
   vn.na(_([[You ask if anybody is there, but don't get any response. It is probably best to go in and check.]]))
   vn.menu{
      {_([[Walk in casually]]), "02_walkin"},
      {_([[Draw your weapon and barge in]]), "02_barge"},
   }

   vn.label("02_walkin")
   vn.na(_([[You walk in casually and turn on the lights to see what is going on. A quick inspection doesn't turn up anything. You turn around to leave the ship when suddenly something jumps on your back!]]))
   vn.na(_([[You fumble around and slam your back into a wall and it drops off with a resounding thud. Another unknown object jumps onto your leg and you manage to kick it off!]]))
   vn.na(_([[You recompose yourself and look around and find two small kids unconscious on the floor. They look really dirty and are covered in rags. When did they get on your ship?]]))
   vn.na(_([[Seeing as they don't seem like much of a threat, you get some water and splash it on their faces to wake them up. They slowly wake up and immediately run and try to hide in a corner. You slowly calm them down and coax them to tell you what is going on.]]))
   vn.jump("02_cont")

   vn.label("02_barge")
   vn.na(_([[You barge in with your weapon drawn, ready to shoot whatever sort of space monster awaiting you. You catch a quick motion in the corner of the eye and shoot a low power warning shot. Suddenly, you hear what appears to be sobbing.]]))
   vn.na(_([[You carefully approach with your weapon still in hand and find yourself face to face with two small kids covered in rags. One is bawling their heart out while the other is hugging them trying to calm them down. They look at your fearfully.]]))
   vn.na(_([[Seeing that they don't seem like much of a threat, you lower your weapon and try to calm them down. Once they are not panicked anymore, you coax them to tell you what is going on.]]))
   vn.jump("02_cont")

   vn.label("02_cont")
   vn.na(fmt.f(_([[It seems like they are a pair of orphans from {spob} who stowed away in the cargo hold while you were landed. They don't really have anywhere to go and don't want to end up like their parent and wanted to at least try to get away from there for a better life.]]),
      {spob=stowaway_spob}))
   vn.na(_([[Seeing that your ship is no place to raise a couple of orphans, you have to make a decision of what to do with them.]]))
   vn.menu{
      {_([[Try to find a good place for them]]), "03_goodhome"},
      {_([[Drop them off on the nearest planet]]), "03_dropoff"},
      --{_([[Dump them out the airlock]]), "03_airlock"},
   }

   vn.label("03_dropoff")
   vn.func( function ()
      hook.land( "land" )
      evt.save()
   end )
   vn.na(_([[You decide to drop them off next time you land to get rid of the problem as soon as possible. At the end of the day, you are not here to supervise orphans, and any place will be a better future for them.]]))
   vn.done()

   vn.label("03_goodhome")
   vn.na(_([[You decide to take the orphans to a better place and give them a promising future. However, first things first, you decide to wash them down and give them some of your old clothes. Last thing you want to get is an infestation of space lice.]]))
   vn.na(_([[Once you get hygiene out of the way, you give them some drawing utensils to keep them distracted while you fly them to a better home.]]))
   vn.func( function ()
      naev.missionStart("Majesteka Stowaways")
      cleanup = true
   end )

   vn.run()

   if cleanup then
      evt.finish(true)
   end
end

function land ()
   if not spob.cur():services()["inhabited"] then return end

   vn.clear()
   vn.scene()
   vn.transition()

   if spob.cur() == stowaway_spob then
      vn.na(fmt.f(_([[You land again on {spob} to drop off the orphans at their home world. You figure it is the environment that they grew up in and are most accustomed to, and will probably be best for them.]]),
         {spob=stowaway_spob}))
      vn.na(_([[The orphans become quiet and sad when they realize they are back to where they came from. However, knowing their battle to be lost they get off your ship and disappear into the grimy darkness.]]))
      vn.na(_([[With the orphans out of the way, you can now get back to your adventures, however, you have a small nagging feeling that maybe that was not the best for the orphans.]]))

      neu.addMiscLog(fmt.f(_([[You returned a pair of stowaway orphans to their home planet of {spob}.]]),
         {spob=stowaway_spob}))
   else
      vn.na(fmt.f(_([[You land and promptly get the orphans off your ship. Being their first times they are off their home world they are in amaze at all the things out there. With a boundless sense of wonder and curiosity, they disappear. At least they made it away from {spob}.]]),
         {spob=stowaway_spob}))

      neu.addMiscLog(fmt.f(_([[You took a pair of stowaway orphans from {spob} to {curspob} in the {cursys}.]]),
         {spob=stowaway_spob, curspob=spob.cur(), cursys=system.cur()}))
   end

   vn.run()

   evt.finish(true)
end
