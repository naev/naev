   --[[
      MISSION: diy-nerds
      DESCRIPTION: Cart some nerds and their hardware to some DIY
      contest on a neighbouring planet. Wait until the contest is
      over, then cart them back. Receive either your payment or
      their hardware. The player can fail in multiple ways.
      AUTHOR: thilo <thilo@thiloernst.de>
   --]]



-- Bar information, describes how the NPC appears in the bar
bar_desc = _("You see a bunch of guys and gals, excitedly whispering over some papers, which seem to contain column after column of raw numbers. Two of them don't participate in the babbling, but look at you expectantly.")

-- Mission details.
misn_title = _("DIY Nerds") 
misn_reward = _("20,000 credits")
misn_desc = _("Cart some nerds to their contest, and back.")

title = {}
text = {}

-- The nerd's job proposal
title[1] = _("A group of excited nerds")
text[1] = [[As you approach the group, the babbling ceases and the papers are quickly and jealously stashed away. One of the girls comes forward and introduces herself.
    "Hi, I'm Mia. We need transportation, and you look as if you could need some dough. Interested?"
    You reply that for a deal to be worked out, they better provide some detail.
    "Listen," she says, "there's this Homebrew Processing Box Masters on %s. Right over there, this system. I'm sure our box will get us the first prize. You take us there, you take us back, you get 20,000."
    You just start wondering at the boldness of so young a lady as she already signals her impatience. "What now, you do it?"]]

-- you accept
title[2] = _("We have a deal!")
text[2] = [[Upon accepting the task, you see the entire group relax visibly, and you can almost feel Mia's boldness fade away - to some extent, at least. It seems that the group is quite keen on the competition, but until now had no idea how to get there.
    As the others scramble to get up from their cramped table and start to gather their belongings, it is again up to Mia to address you:
    "Really? You'll do it? Um, great. Fantastic. I just knew that eventually, someone desperate would turn up. Ok, we're set to go. We better take off immediately and go directly to %s, or we'll be late for the contest!"]]
  
-- right planet, in time
title[3] = _("Happy nerds")
text[3] = [["Good job, %s," Mia compliments you upon arrival. "We'll now go win the competition and celebrate a bit. You better stay in the system. We will hail you in about 4 or 5 periods, so you can pick us up an' bring us back to %s."
    That said, the nerds shoulder the box and rushes towards a banner which reads "Admissions".]]

-- right planet, late
title[4] = _("Angry nerds")
text[4] = [[Mia fumes. "Great. Just great! We're late, you jerk." She points to a crumpled banner reading "Admissions". The area below it is deserted. "No contest for us, no payment for you. Understand? Go and take your sorry excuse for a ship into the corona of a suitable star. We will find someone else to take us back to %s. Someone reliable."
    As her emphasis on the last words is still ringing in your ears, the gang of nerds stroll toward an archway, behind which, judging from the bustling atmosphere, the contest is already going on.]]

-- wrong planet, late
title[5] = _("Furious nerds")
text[5] = [[The nerds quickly and quietly pack up their box and start to leave your ship. Finally, Mia turns to you. Her body language suggests that she's almost bursting with anger. Yet her voice is controlled when she starts talking:
    "You're a sorry loser. The contest is almost over and we are stranded in some dump we never wanted to see. I'm sure you agree that this isn't worth any payment." She turns to leave, but then adds: "Are you sure that everything is in order with your ship's core? You don't want it to melt down just in the middle of a fight, do you?" With this, she joins the rest of her group, and they are gone.]]

-- you are hailed to pick them up again
title[6] = _("In-system communication")
text[6] = [[A beep from your communications equipment tells you that someone wants to talk to you. You realize it is the nerds, and return the hail. "Yo! This is Mia," comes a familiar voice from the speaker. "We're done here. Time to come back and pick us up, we have things to do on %s."]]

title[7] = _("In-system communication")
text[7] = [[Your comm link comes up again. It is the nerds, whom you'd almost forgotten. You hear Mia's voice: "Hey, what are you waiting for? You'd better be here within one period, or we'll get another pilot and pay them, not you!"]]

-- you pick up the nerds in time, they won
title[8] = _("Happy nerds")
text[8] = [[As soon as you get of your ship, you are surrounded by the group of nerds, who are enthusiastic. "We won!" one of the dudes shouts at you. Surprisingly, the group seems to not completely be dependent on Mia when it comes to communicating with outsiders. Maybe the booze the group is obviously intoxicated with did help a little. "Take us back to %s," one of them says, "we'll continue to celebrate on the way."]]

-- you pick up the nerds in time, they didn't win
title[9] = _("Sad nerds")
text[9] = [[As you get of your ship, you do not immediately see the nerds. You finally find them in a dark corner of the landing pad quietly sitting on their box, obviously not in a good mood. You greet them, but nobody speaks a word. "Hey, what's up? Come on, tell me!" you inquire. The nerds warily glance at each other, and finally Mia bursts out:
    "That aristocratic ass of a bored teenager! He snatched the prize from us! It wasn't even fair play. His box wasn't home built. It was a brand new ship's processing unit, on which he banged his hammer until it looked acceptable. And the corrupt idiots in the jury pretended not to notice!"
    "So no, we didn't win" she adds after taking a few breaths to calm down. "Take us back to %s.]]

-- you do not pickup the nerds in time
title[10] = _("No more nerds")
text[10] = [[You look around, but the nerds are nowhere to be found. That is not much of a surprise, seeing that you are way too late.
    Suddenly, a guy approaches you. "Hi, are you %s? The nerds wanted you to know that, basically, they got another transport home. One of the girls said some more, in a particularly rude language, but I don't remember the details".]]

-- you do not pickup the nerds in time, and don't even land on the right planet
title[11] = _("You forgot the nerds")
text[11] = [[Seeing that it is already too late to pick up the nerds, and that you're quite far from %s, you decide it's better to forget about them completely.]]

-- you return the nerds, who have won the contest
title[12] = _("The End")
text[12] = [[The nerds, finally exhausted from all the partying, still smile as they pack up their prize-winning box and leave your ship. Mia beams as she turns to you. "Well done, %s. You see, since we got loads of prize money, we decided to give you a bonus. After all, we wouldn't have gotten there without your service. Here, have 30,000. Good day to you."]]

-- you return the nerds, who did not win the contest
title[13] = _("Minor Complications")
text[13] = [[With sagging shoulders, the nerds unload their box. Mia turns to address you, not bold at all this time. "Um, we got a bit of a problem here. You know, we intended to pay the trip from our prize money. Now we don't have no prize money."
    As you're trying to decide what to make of the situation, one of the other nerds creeps up behind Mia and cautiously gestures for her to join the group a few yards away, all the time avoiding your eyes. Strange guy, you think, as if he was not accustomed to be socializing with strangers. Mia joins the group, and some whispering ensues. Mia returns to you after a few minutes.
    "Ok, we have just solved our problem. See, that ass of a champion won the contest with a ship's processing unit. We can do it the other way round. We'll modify our box so that it can be used as a ship's core system, and you can have it as a compensation for your troubles. Interested?"]]

title[14] = _("So what?")
text[14] = [["Honestly, there is nothing you can do about it," Mia says impatiently, as if you were a small child complaining about the finiteness of an ice cream cone. "Just stand by while we rig the thing up."]]

title[15] = _("The End")
text[15] = [["You can wait for it, won't take longer than half a period," Mia informs you. You stand by as the nerds start to mod their box. As they are going for it, you wonder if they're actually wrecking it and you'll maybe be left with a piece of worthless junk.
    Finally, the modified box is set before you. "Here you are. Now you're the proud owner of the system's only home-made core system. It's gotten a bit bulkier than we thought, with all this rigging for energy and coolant supply, but it should work just fine, about equivalent to the %s. We need to go now and think about something more advanced for the next competition. Have a nice day."
    With that, the nerds leave. Having gotten nothing else out of this, you think you should visit an outfitter to see if the homemade core system may actually be of any use, or if you can at least sell it.]]

-- for use in accept(), if any of the mission's preconditions are not met
title[16] = _("Not enough cargo space")
text[16] = [["Aw, I forgot" she adds. "We would of course need 4 tons of free cargo space for our box."]]
title[17] = _("Young People")
text[17] = [["Sorry, we're busy right now."]]

-- additional text for stage 2 cargo space test
title[18] = _("Room for the box")
text[18] = [["Aw %s," Mia complains, "as if you didn't know that our box needs 4 tons of free cargo space. Make room now, and pick us up at the bar."]]

title[19] = _("Departure")
text[19] = [[The nerds follow you to your ship and finally stow away their box. Now, you're all set to go.]]

title[20] = _("No room, no job")
text[20] = [[As you enter the bar, the nerds are immediately upon you. "What is it with you?" Mia asks. "Is it so hard to make some room for our box? I am fed up with you. Consider our agreement nullified. I hope to never again have business with you." Some angry stares later, the nerds are gone, trying to find another pilot.]]

-- OSD texts
textosd = {}
-- stage 1
textosd[1] = _("Bring the nerds and their box to %s before %s")
textosd[2] = _("You have %s remaining.")
textosd[3] = _("You're late. Mia has started verbally abusing you and your ship. Better land to get rid of the nerds and their box.")
-- stage 2
textosd[4] = _("You are to wait several periods until hailed by the nerds for their return trip.")
textosd[5] = _("Pick up the nerds on %s for their return trip to %s.")
textosd[6] = _("The nerds are getting impatient.")
textosd[7] = _("You didn't pick up the nerds in time.")
-- stage 3
textosd[8] = _("Return the nerds to %s.")

textmsg = {}
-- displayed if you leave without the nerd's authorization
textmsg[1] = _("Have the nerds not told you to stay in the system? Mission failed!")
textmsg[2] = _("Have the nerds not told you to pick them up at the bar? Mission failed!")

-- the mission cargo (names as in commodity.xml)
misn_cargo1 = "Group of Nerds"
misn_cargoamount1 = 0
misn_cargo2 = "Box"
misn_cargoamount2 = 4

-- the outfit name as in outfit.xml
outfit = "Unicorp PT-100 Core System"

function create ()
   misn.setNPC( _("Young People"), "neutral/unique/mia" )
   misn.setDesc( bar_desc )
end

function accept ()
   local cp,s = planet.cur()
   srcPlanet = cp
   for i,p in ipairs(system.planets(system.cur())) do
      if planet.services(p)["land"] and p ~= cp and p:canLand() then
         destPlanet=p
         break -- atm, just take the first landable planet which is not the current one
      end
   end

   -- the mission cannot be started with less than two landable assets in the system
   if not system_hasAtLeast(2, "land") then
      tk.msg(title[17], text[17])
      misn.finish(false)
   end

   if not tk.yesno( title[1], string.format(text[1], planet.name(destPlanet) )) then
      misn.finish(false)
   else
         if player.pilot():cargoFree() < 4 then
         tk.msg(title[16], text[16])
         misn.finish(false)
      end

      misn.accept()
      misn.setTitle( misn_title)
      misn.setReward( misn_reward)
      misn.setDesc( misn_desc)
      marker = misn.markerAdd( system.cur(), "low" )

      tk.msg(title[2], string.format(text[2], planet.name(destPlanet)))
      local distance = vec2.dist( planet.pos(srcPlanet), planet.pos(destPlanet) )
      local stuperpx = 1 / player.pilot():stats().speed_max * 30 -- from cargo_common.lua
      expiryDate = time.get() + time.create(0, 0, 10010 + distance * stuperpx + 3300 ) -- takeoff + min travel time + leeway

      addNerdCargo()
      lhook = hook.land("nerds_land1", "land")
      misn.osdCreate( misn_title, {string.format(textosd[1], planet.name(destPlanet), time.str(expiryDate, 1)), string.format(textosd[2], time.str(expiryDate - time.get(), 1))})
      dhook = hook.date(time.create(0, 0, 100), "nerds_fly1")
   end
end


-- invoked upon landing (stage 1: cart the nerds to destPlanet)
function nerds_land1()
    local cp, s = planet.cur()

   if intime and cp ~= destPlanet then
      return
   end

   rmNerdCargo()
   hook.rm(lhook)
   hook.rm(dhook)

   if cp == destPlanet then
      if intime then
      -- in time, right planet
         tk.msg(title[3], string.format(text[3], player.name(), planet.name(srcPlanet) ))
           misn.osdCreate( misn_title, {textosd[4]} )
         expiryDate = time.get() + time.create(0, 0, 36000+rnd.rnd(-7500,7500), 0)
         hailed = false
         impatient = false
         dhook = hook.date(time.create(0, 0, 100), "nerds_fly2")
         lhook = hook.land("nerds_land2", "land")
         jhook = hook.jumpout("nerds_jump")

      else
      -- late, right planet
         tk.msg(title[4], string.format(text[4], planet.name(srcPlanet) ))
         misn.finish(true)
      end
   else
      if not intime then
      -- late, not even the right planet
         tk.msg(title[5], text[5])
         misn.finish(true)
      end
   end
end

-- date hooked to update the time in the mission osd in stage 1 (carting the nerds to the contest)
function nerds_fly1()
   intime = expiryDate >= time.get()
   if intime then
      misn.osdCreate( misn_title, {string.format(textosd[1], planet.name(destPlanet), time.str(expiryDate, 2)), string.format(textosd[2], time.str(expiryDate - time.get(), 1))})
   else
      misn.osdCreate( misn_title, {string.format(textosd[1], planet.name(destPlanet), time.str(expiryDate, 2)), textosd[3]})
      misn.osdActive(2)
   end
end

-- hooked to 'land' in the second stage (wait for the nerds to hail you for the return trip)
function nerds_land2()

   function cleanup()
      hook.rm(dhook)
      hook.rm(lhook)
      hook.rm(jhook)
   end

   if not hailed then
      return
   end

   if intime and planet.cur() == destPlanet then
   -- you pickup the nerds in time
      nerdswon = rnd.rnd() >= 0.6
      if nerdswon then
         tk.msg(title[8], string.format(text[8], planet.name(srcPlanet)))
      else
         tk.msg(title[9], string.format(text[9], planet.name(srcPlanet)))
      end
      cleanup()

         if player.pilot():cargoFree() >= 4 then
      -- player has enough free cargo
         nerds_return()
      else
      -- player has not enough free cargo space, give him last chance to make room
         tk.msg(title[18], string.format(text[18], player.name()))
         lhook = hook.land("nerds_bar", "bar")
         jhook = hook.takeoff("nerds_takeoff")
      end

   elseif not intime and planet.cur() == destPlanet then
   -- you're late for the pickup
      tk.msg(title[10], string.format(text[10], player.name()))
      cleanup()
      misn.finish(true)

   elseif not intime then
   -- you're late and far from the nerds
      tk.msg(title[11], string.format(text[11], planet.name(srcPlanet)))
      cleanup()
      misn.finish(true)
   end
end

-- date hooked in stage 2 (waiting for the nerds hail you for their return trip)
function nerds_fly2()
   if not hailed and time.get() > expiryDate then
      tk.msg(title[6], string.format(text[6], planet.name(srcPlanet)) )
        misn.osdCreate( misn_title, {string.format(textosd[5], planet.name(destPlanet), planet.name(srcPlanet))})
      hailed = true
   end

   intime = time.get() <= expiryDate + time.create(0,3,3000)

   -- no pickup since hail+2STP+1STP: mission failed (however, you must still land somewhere)
   if not intime then
        misn.osdCreate( misn_title, {string.format(textosd[5], planet.name(destPlanet), planet.name(srcPlanet)), textosd[7] })
        misn.osdActive(2)
   end

   -- no pickup since hail+2STP
   if hailed and intime and time.get() > expiryDate + time.create(0,2,0) then
      if not impatient then
         tk.msg(title[7], string.format(text[7], planet.name(srcPlanet)) )
         impatient = true
      end
        misn.osdCreate( misn_title, {string.format(textosd[5], planet.name(destPlanet), planet.name(srcPlanet)), textosd[6], string.format(textosd[2], time.str(expiryDate + time.create(0,3,0) - time.get(), 2)) })
        misn.osdActive(2)
   end
end

-- hooked to entering the bar in stage 2
function nerds_bar()
   hook.rm(jhook)
   hook.rm(lhook)
   if player.pilot():cargoFree() >= 4 then
      tk.msg(title[19], text[19])
      nerds_return()
   else
      tk.msg(title[20], text[20])
      misn.finish(true)
   end
end

-- hooked to leaving the system in stage 2
function nerds_jump()
   player.msg(textmsg[1])
   misn.finish(true)
end

-- hooked to inappropriately taking off in stage 2
function nerds_takeoff()
   hook.rm(jhook)
   hook.rm(lhook)
   player.msg(textmsg[2])
   misn.finish(true)
end


-- common prep for the final stage
function nerds_return()
   addNerdCargo()
   misn.osdCreate(misn_title, { string.format(textosd[8], planet.name(srcPlanet) ) })
   lhook = hook.land("nerds_land3", "land")
end

-- hooked to 'land' in the final stage (returning the nerds)
function nerds_land3()
   local cp,s = planet.cur()
   if cp == srcPlanet then
      if nerdswon then
         tk.msg(title[12], string.format(text[12], player.name()))
         player.pay(30000)
      else
         if not tk.yesno(title[13], text[13]) then
            tk.msg(title[14], text[14])
         end
         tk.msg(title[15], text[15]:format( outfit ))
         time.inc(time.create(0,0,5000))
         player.addOutfit(outfit)
         if planet.services(cp)["outfits"] then
            player.landWindow("equipment")
         end
      end
      rmNerdCargo()
      hook.rm(dhook)
      hook.rm(lhook)
      misn.markerRm(marker)
      misn.finish(true)
   end
end

-- to check if the assets in the current system have at least _amount_ of _service_
function system_hasAtLeast (amount, service)
   local p = {}
   for i,v in ipairs(system.planets(system.cur())) do
      if planet.services(v)[service] and v:canLand() then
         table.insert(p,v)
      end
   end
   return #p >= amount
end

-- helper functions, used repeatedly
function addNerdCargo()
   cargo1 = misn.cargoAdd(misn_cargo1, misn_cargoamount1)
   cargo2 = misn.cargoAdd(misn_cargo2, misn_cargoamount2)
end

function rmNerdCargo()
   misn.cargoRm(cargo1)
   misn.cargoRm(cargo2)
end

