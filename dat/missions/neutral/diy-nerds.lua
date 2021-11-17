--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="DIY Nerds">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <chance>2</chance>
   <location>Bar</location>
  </avail>
  <notes>
   <tier>1</tier>
  </notes>
 </mission>
 --]]
   --[[
      MISSION: diy-nerds
      DESCRIPTION: Cart some nerds and their hardware to some DIY
      contest on a neighbouring planet. Wait until the contest is
      over, then cart them back. Receive either your payment or
      their hardware. The player can fail in multiple ways.
      AUTHOR: thilo <thilo@thiloernst.de>
   --]]
local pir = require "common.pirate"
local fmt = require "format"

-- Mission details.
local reward = 40e3

-- helper functions, defined below
local addNerdCargo, rmNerdCargo, nerds_return, system_hasAtLeast
-- luacheck: globals nerds_bar nerds_fly1 nerds_fly2 nerds_jump nerds_land1 nerds_land2 nerds_land3 nerds_takeoff (Hook functions passed by name)

-- the mission cargo
local misn_cargo1 = N_("Group of Nerds")
local misn_cargodesc1 = N_("A bunch of protagonists.")
local misn_cargoamount1 = 0
local misn_cargo2 = N_("Box")
local misn_cargodesc2 = N_("A homebrew processing unit.")
local misn_cargoamount2 = 4

-- the outfit name as in outfit.xml
local reward_outfit = "Unicorp PT-16 Core System"

function create ()
   misn.setNPC( _("Young People"), "neutral/unique/mia.webp", _("You see a bunch of guys and gals, excitedly whispering over some papers, which seem to contain column after column of raw numbers. Two of them don't participate in the babbling, but look at you expectantly.") )
end

function accept ()
   local cp = planet.cur()
   mem.srcPlanet = cp
   for i,p in ipairs(system.planets(system.cur())) do
      if planet.services(p)["land"] and p ~= cp and p:canLand() then
         mem.destPlanet=p
         break -- atm, just take the first landable planet which is not the current one
      end
   end

   -- the mission cannot be started with less than two landable assets in the system
   if not system_hasAtLeast(2, "land") then
      tk.msg(_("Young People"), _([["Sorry, we're busy right now."]]))
      misn.finish(false)
   end

   if not tk.yesno( _("A group of excited nerds"), fmt.f(_([[As you approach the group, the babbling ceases and the papers are quickly and jealously stashed away. One of the girls comes forward and introduces herself.
    "Hi, I'm Mia. We need transportation, and you look as if you could need some dough. Interested?"
    You reply that for a deal to be worked out, they better provide some detail.
    "Listen," she says, "there's this Homebrew Processing Box Masters on {pnt}. Right over there, this system. I'm sure our box will get us the first prize. You take us there, you take us back, you get 20,000."
    You just start wondering at the boldness of so young a lady as she already signals her impatience. "What now, you do it?"]]), {pnt=mem.destPlanet} )) then
      misn.finish(false)
   else
      if player.pilot():cargoFree() < 4 then
         tk.msg(_("Not enough cargo space"), _([["Aw, I forgot" she adds. "We would of course need 4 tonnes of free cargo space for our box."]]))
         misn.finish(false)
      end

      misn.accept()
      misn.setTitle( _("DIY Nerds") )
      misn.setReward( fmt.credits(reward) )
      misn.setDesc( _("Cart some nerds to their contest, and back.") )
      mem.marker = misn.markerAdd( mem.destPlanet, "low" )

      tk.msg(_("We have a deal!"), fmt.f(_([[Upon accepting the task, you see the entire group relax visibly, and you can almost feel Mia's boldness fade away - to some extent, at least. It seems that the group is quite keen on the competition, but until now had no idea how to get there.
    As the others scramble to get up from their cramped table and start to gather their belongings, it is again up to Mia to address you:
    "Really? You'll do it? Um, great. Fantastic. I just knew that eventually, someone desperate would turn up. OK, we're set to go. We better take off immediately and go directly to {pnt}, or we'll be late for the contest!"]]), {pnt=mem.destPlanet}))
      local distance = vec2.dist( planet.pos(mem.srcPlanet), planet.pos(mem.destPlanet) )
      local stuperpx = 1 / player.pilot():stats().speed_max * 30 -- from common.cargo
      mem.expiryDate = time.get() + time.create(0, 0, 10010 + distance * stuperpx + 3300 ) -- takeoff + min travel time + leeway

      addNerdCargo()
      mem.lhook = hook.land("nerds_land1", "land")
      misn.osdCreate( _("DIY Nerds"), {
	      fmt.f(_("Bring the nerds and their box to {pnt} before {time}"), {pnt=mem.destPlanet, time=time.str(mem.expiryDate, 1)}),
	      fmt.f(_("You have {time} remaining"), {time=time.str(mem.expiryDate - time.get(), 1)}),
      })
      mem.dhook = hook.date(time.create(0, 0, 100), "nerds_fly1")
   end
end


-- invoked upon landing (stage 1: cart the nerds to mem.destPlanet)
function nerds_land1()
   local cp = planet.cur()

   if mem.intime and cp ~= mem.destPlanet then
      return
   end

   rmNerdCargo()
   hook.rm(mem.lhook)
   hook.rm(mem.dhook)

   if cp == mem.destPlanet then
      if mem.intime then
      -- in time, right planet
         tk.msg(_("Happy nerds"), fmt.f(_([["Good job, {player}," Mia compliments you upon arrival. "We'll now go win the competition and celebrate a bit. You better stay in the system. We will hail you in about 4 or 5 periods, so you can pick us up an' bring us back to {pnt}."
    That said, the nerds shoulder the box and rushes towards a banner which reads "Admissions".]]), {player=player.name(), pnt=mem.srcPlanet} ))
           misn.osdCreate( _("DIY Nerds"), {_("Wait several periods in this system until hailed by the nerds for their return trip")} )
         mem.expiryDate = time.get() + time.create(0, 0, 36000+rnd.rnd(-7500,7500), 0)
         mem.hailed = false
         mem.impatient = false
         mem.dhook = hook.date(time.create(0, 0, 100), "nerds_fly2")
         mem.lhook = hook.land("nerds_land2", "land")
         mem.jhook = hook.jumpout("nerds_jump")

      else
      -- late, right planet
         tk.msg(_("Angry nerds"), fmt.f(_([[Mia fumes. "Great. Just great! We're late, you jerk." She points to a crumpled banner reading "Admissions". The area below it is deserted. "No contest for us, no payment for you. Understand? Go and take your sorry excuse for a ship into the corona of a suitable star. We will find someone else to take us back to {pnt}. Someone reliable."
    As her emphasis on the last words is still ringing in your ears, the gang of nerds stroll toward an archway, behind which, judging from the bustling atmosphere, the contest is already going on.]]), {pnt=mem.srcPlanet} ))
         misn.finish(true)
      end
   else
      if not mem.intime then
      -- late, not even the right planet
         tk.msg(_("Furious nerds"), _([[The nerds quickly and quietly pack up their box and start to leave your ship. Finally, Mia turns to you. Her body language suggests that she's almost bursting with anger. Yet her voice is controlled when she starts talking:
    "You're a sorry loser. The contest is almost over and we are stranded in some dump we never wanted to see. I'm sure you agree that this isn't worth any payment." She turns to leave, but then adds: "Are you sure that everything is in order with your ship's core? You don't want it to melt down just in the middle of a fight, do you?" With this, she joins the rest of her group, and they are gone.]]))
         misn.finish(true)
      end
   end
end

-- date hooked to update the time in the mission OSD in stage 1 (carting the nerds to the contest)
function nerds_fly1()
   mem.intime = mem.expiryDate >= time.get()
   if mem.intime then
      misn.osdCreate( _("DIY Nerds"), {
         fmt.f(_("Bring the nerds and their box to {pnt} before {time}"), {pnt=mem.destPlanet, time=time.str(mem.expiryDate, 2)}),
         fmt.f(_("You have {time} remaining"), {time=time.str(mem.expiryDate - time.get(), 1)}),
      })
   else
      misn.osdCreate( _("DIY Nerds"), {
         fmt.f(_("Bring the nerds and their box to {pnt} before {time}"), {pnt=mem.destPlanet, time=time.str(mem.expiryDate, 2)}),
         _("You're late and the nerds are getting angry and abusive; land to get rid of the nerds and their box"),
      })
      misn.osdActive(2)
   end
end

-- hooked to 'land' in the second stage (wait for the nerds to hail you for the return trip)
function nerds_land2()
   local function cleanup()
      hook.rm(mem.dhook)
      hook.rm(mem.lhook)
      hook.rm(mem.jhook)
   end

   if not mem.hailed then
      return
   end

   if mem.intime and planet.cur() == mem.destPlanet then
   -- you pickup the nerds in time
      mem.nerdswon = rnd.rnd() >= 0.6
      if mem.nerdswon then
         tk.msg(_("Happy nerds"), fmt.f(_([[As soon as you get of your ship, you are surrounded by the group of nerds, who are enthusiastic. "We won!" one of the dudes shouts at you. Surprisingly, the group seems to not completely be dependent on Mia when it comes to communicating with outsiders. Maybe the booze the group is obviously intoxicated with did help a little. "Take us back to {pnt}," one of them says, "we'll continue to celebrate on the way."]]), {pnt=mem.srcPlanet}))
      else
         tk.msg(_("Sad nerds"), fmt.f(_([[As you get of your ship, you do not immediately see the nerds. You finally find them in a dark corner of the landing pad quietly sitting on their box, obviously not in a good mood. You greet them, but nobody speaks a word. You ask them what's wrong. The nerds warily glance at each other before Mia bursts out in frustration.
    "That aristocratic ass of a bored teenager! He snatched the prize from us! It wasn't even fair play. His box wasn't home built. It was a brand new ship's processing unit, on which he banged his hammer until it looked acceptable. And the corrupt assholes in the jury pretended not to notice!"
    "So no, we didn't win" she adds after taking a few breaths to calm down. "Take us back to {pnt}."]]), {pnt=mem.srcPlanet}))
      end
      cleanup()

         if player.pilot():cargoFree() >= 4 then
      -- player has enough free cargo
         nerds_return()
      else
      -- player has not enough free cargo space, give him last chance to make room
         tk.msg(_("Room for the box"), fmt.f(_([["Aw {player}," Mia complains, "as if you didn't know that our box needs 4 tonnes of free cargo space. Make room now, and pick us up at the bar."]]), {player=player.name()}))
         mem.lhook = hook.land("nerds_bar", "bar")
         mem.jhook = hook.takeoff("nerds_takeoff")
      end

   elseif not mem.intime and planet.cur() == mem.destPlanet then
   -- you're late for the pickup
      tk.msg(_("No more nerds"), fmt.f(_([[You look around, but the nerds are nowhere to be found. That is not much of a surprise, seeing that you are way too late.
    Suddenly, a guy approaches you. "Hi, are you {player}? The nerds wanted you to know that, basically, they got another transport home. One of the girls said some more, in a particularly rude language, but I don't remember the details".]]), {player=player.name()}))
      cleanup()
      misn.finish(true)

   elseif not mem.intime then
   -- you're late and far from the nerds
      tk.msg(_("You forgot the nerds"), fmt.f(_([[Seeing that it is already too late to pick up the nerds, and that you're quite far from {pnt}, you decide it's better to forget about them completely.]]), {pnt=mem.srcPlanet}))
      cleanup()
      misn.finish(true)
   end
end

-- date hooked in stage 2 (waiting for the nerds hail you for their return trip)
function nerds_fly2()
   if not mem.hailed and time.get() > mem.expiryDate then
      tk.msg(_("In-system communication"), fmt.f(_([[A beep from your communications equipment tells you that someone wants to talk to you. You realize it is the nerds, and return the hail. "Yo! This is Mia," comes a familiar voice from the speaker. "We're done here. Time to come back and pick us up, we have things to do on {pnt}."]]), {pnt=mem.srcPlanet}) )
        misn.osdCreate( _("DIY Nerds"), {
           fmt.f(_("Pick up the nerds on {pickup_pnt} for their return trip to {dropoff_pnt}"), {pickup_pnt=mem.destPlanet, dropoff_pnt=mem.srcPlanet}),
        })
      mem.hailed = true
   end

   mem.intime = time.get() <= mem.expiryDate + time.create(0,3,3000)

   -- no pickup since hail+2STP+1STP: mission failed (however, you must still land somewhere)
   if not mem.intime then
        misn.osdCreate( _("DIY Nerds"), {
           fmt.f(_("Pick up the nerds on {pickup_pnt} for their return trip to {dropoff_pnt}"), {pickup_pnt=mem.destPlanet, dropoff_pnt=mem.srcPlanet}),
           _("You didn't pick up the nerds in time"),
        })
        misn.osdActive(2)
   end

   -- no pickup since hail+2STP
   if mem.hailed and mem.intime and time.get() > mem.expiryDate + time.create(0,2,0) then
      if not mem.impatient then
         tk.msg(_("In-system communication"), _([[Your comm link comes up again. It is the nerds, whom you'd almost forgotten. You hear Mia's voice: "Hey, what are you waiting for? You'd better be here within one period, or we'll get another pilot and pay them, not you!"]]) )
         mem.impatient = true
      end
        misn.osdCreate( _("DIY Nerds"), {
           fmt.f(_("Pick up the nerds on {pickup_pnt} for their return trip to {dropoff_pnt}"), {pickup_pnt=mem.destPlanet, dropoff_pnt=mem.srcPlanet}),
           _("The nerds are getting impatient"),
           fmt.f(_("You have {time} remaining"), {time=time.str(mem.expiryDate + time.create(0,3,0) - time.get(), 2)}),
        })
        misn.osdActive(2)
   end
end

-- hooked to entering the bar in stage 2
function nerds_bar()
   hook.rm(mem.jhook)
   hook.rm(mem.lhook)
   if player.pilot():cargoFree() >= 4 then
      tk.msg(_("Departure"), _([[The nerds follow you to your ship and finally stow away their box. Now, you're all set to go.]]))
      nerds_return()
   else
      tk.msg(_("No room, no job"), _([[As you enter the bar, the nerds are immediately upon you. "What is it with you?" Mia asks. "Is it so hard to make some room for our box? I am fed up with you. Consider our agreement nullified. I hope to never again have business with you." Some angry stares later, the nerds are gone, trying to find another pilot.]]))
      misn.finish(true)
   end
end

-- hooked to leaving the system in stage 2
function nerds_jump()
   player.msg(_("Have the nerds not told you to stay in the system? Mission failed!"))
   misn.finish(true)
end

-- hooked to inappropriately taking off in stage 2
function nerds_takeoff()
   hook.rm(mem.jhook)
   hook.rm(mem.lhook)
   player.msg(_("Have the nerds not told you to pick them up at the bar? Mission failed!"))
   misn.finish(true)
end


-- common prep for the final stage
function nerds_return()
   addNerdCargo()
   misn.osdCreate(_("DIY Nerds"), { fmt.f(_("Return the nerds to {pnt}"), {pnt=mem.srcPlanet} ) })
   mem.lhook = hook.land("nerds_land3", "land")
end

-- hooked to 'land' in the final stage (returning the nerds)
function nerds_land3()
   local cp = planet.cur()
   if cp == mem.srcPlanet then
      if mem.nerdswon then
         tk.msg(_("The End"), fmt.f(_([[The nerds, finally exhausted from all the partying, still smile as they pack up their prize-winning box and leave your ship. Mia beams as she turns to you. "Well done, {player}. You see, since we got loads of prize money, we decided to give you a bonus. After all, we wouldn't have gotten there without your service. Here, have 30,000. Good day to you."]]), {player=player.name()}))
         player.pay( reward )
      else
         if not tk.yesno(_("Minor Complications"), _([[With sagging shoulders, the nerds unload their box. Mia turns to address you, not bold at all this time. "Um, we got a bit of a problem here. You know, we intended to pay the trip from our prize money. Now we don't have no prize money."
    As you're trying to decide what to make of the situation, one of the other nerds creeps up behind Mia and cautiously gestures for her to join the group a few yards away, all the time avoiding your eyes. Strange guy, you think, as if he was not accustomed to be socializing with strangers. Mia joins the group, and some whispering ensues. Mia returns to you after a few hectoseconds.
    "OK, we have just solved our problem. See, that ass of a champion won the contest with a ship's processing unit. We can do it the other way round. We'll modify our box so that it can be used as a ship's core system, and you can have it as a compensation for your troubles. Interested?"]])) then
            tk.msg(_("So what?"), _([["Honestly, there is nothing you can do about it," Mia says impatiently, as if you were a small child complaining about the finiteness of an ice cream cone. "Just stand by while we rig the thing up."]]))
         end
         tk.msg(_("The End"), fmt.f( _([["You can wait for it, won't take longer than half a period," Mia informs you. You stand by as the nerds start to mod their box. As they are going for it, you wonder if they're actually wrecking it and you'll maybe be left with a piece of worthless junk.
    Finally, the modified box is set before you. "Here you are. Now you're the proud owner of the system's only home-made core system. It's gotten a bit bulkier than we thought, with all this rigging for energy and coolant supply, but it should work just fine, about equivalent to the {outfit}. We need to go now and think about something more advanced for the next competition. Have a nice day."
    With that, the nerds leave. Having gotten nothing else out of this, you think you should visit an outfitter to see if the homemade core system may actually be of any use, or if you can at least sell it.]]), {outfit=_(reward_outfit)} ))
         time.inc(time.create(0,0,5000))
         player.outfitAdd(reward_outfit)
         if planet.services(cp)["outfits"] then
            player.landWindow("equipment")
         end
      end
      rmNerdCargo()
      pir.reputationNormalMission(rnd.rnd(2,3))
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
   local c1 = misn.cargoNew(misn_cargo1, misn_cargodesc1)
   local c2 = misn.cargoNew(misn_cargo2, misn_cargodesc2)
   mem.cargo1 = misn.cargoAdd(c1, misn_cargoamount1)
   mem.cargo2 = misn.cargoAdd(c2, misn_cargoamount2)
end

function rmNerdCargo()
   misn.cargoRm(mem.cargo1)
   misn.cargoRm(mem.cargo2)
end

