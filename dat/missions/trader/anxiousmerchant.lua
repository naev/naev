--[[

   Anxious Merchant
   Author: PhoenixRiver (from an idea on the wiki)

   A merchant with a slow ship suddenly realizes he can't make the delivery and
   implores the player to do it for him. Since he has to look good with his
   employers he'll pay the player a bonus if he does it.

   Note: Variant of the Drunkard and Rush Cargo missions combined

]]--

include "dat/scripts/cargo_common.lua"
include "dat/scripts/numstring.lua"

-- non-gender specific section
bar_desc = _("You see a merchant at the bar in a clear state of anxiety.")

--- Missions details
misn_title = _("Anxious Merchant")
misn_reward = _("Peace of mind (and %d credits)!")

-- OSD
osd_title = _("Help the Merchant")
osd_desc = {}
osd_desc[1] = _("Drop off the goods at %s in the %s system. You have %s remaining.")
osd_desc[2] = _("Drop off the goods at %s in the %s system. You are late.")

-- Cargo Details
cargo = "Goods"

title = {}  --stage titles
text = {}   --mission text
title[1] = _("Spaceport Bar")
title[2] = _("Happy Day")
title[3] = _("Deliver the Goods")
title[4] = _("Deliver the Goods... late")


full_title = _("No Room")
full_text = _([[You don't have enough cargo space to accept this mission.]])

slow_title = _("Too slow")
slow_text = _([[The goods have to arrive in %s but it will take about %s for your ship to reach %s.

Accept the mission anyway?]])

jet_msg = _("You load up the airlock with the last of the %s and hit the big red airlock cycle button - who's to know, right?")

-- choose a gender for the merchant (could be useful for translations into languages that don't have gender neutrality)
if rnd.rnd() >= 0.5 then
   -- Portrait Choice
   portrait = {"neutral/male1",
               "neutral/thief1",
               }

   misn_desc = _("You decided to help a fraught merchant at a bar by delivering some goods to %s for him.")

   text[1] = _([[    As you sit down the merchant looks up at you with a panicked expression, "Ahh! What do you want? Can you see I've enough on my plate as it is?" "Hey, dude, calm down...", you say, "Here, how about I buy you a drink and you tell me about it?" "Jeeze, that's nice of you... ha, maybe I can cut a break today!"
    You grab a couple of drinks and hand one to the slightly more relaxed looking merchant as he starts to talk. "So, I work for Wernet Trading and the deal is I transport stuff for them, they pay me, only I kinda strained my engines running from pirates on the way to the pick-up and now I'm realising that my engines just don't have the speed to get me back to beat the deadline and to top it off I'm late on my bills as it is so those engines are gonna have to last 'til this paycheck comes in... see where this is going? I'm in the Sol nebula without a shield generator."
    "Don't worry so much dude, I'm sure they'll cut you some slack if you're back only a little late", you say reassuringly. "The hell they will! If I don't get this %d tons of %s to %s the next guy's gonna be late and then the customer will be on us and we'll all get it from management, geeze... but that's a thought, if someone else gets it there in time then at least I'll only get a bit of grief... hey, you wouldn't take this stuff for me would you? It's needs to get to the %s system in %s, could you do that for me? I'd sure appreciate it!]])

   text[2] = _([[    The merchant stands up and shakes your hand, "Thank you so much for this, I've a mate at this dock who can sort the cargo transfer and here's the chit you need to give to the cargo guy at %s. Of course payment is done by credit chip so that's yours to keep but at least I'll have a little time to work on those engines before I have to get back on the job! I definietly owe you one for this."]])

   text[3] = _([[    As you touch down at the spaceport you see the Wernet Trading depot surrounded by a hustle and bustle. Once outside you head over to the cargo office and hand over the chit the merchant gave you. The Cargo Inspector looks up at you in surprise and so you explain to him what happened as the cargo is unloaded from your ship. "Wow, thanks for the help, you definitely saved us a ton of grief, here's your payment and, again, thanks for your help - maybe I can buy you a drink some time!" You laugh and part ways.]])

   text[4] = _([[    Landing at the spaceport you see the Wernet Trading depot surrounded by a fraught hum of activity. Once outside you thread your way through the throngs of running and shouting people and finally find the Cargo Inspector. You hand him the chit and he looks at you with surpise and then anger, "What the hell is this!?! This shipment was supposed to be here ages ago! We've been shifting stuff around to make up for it and then you come waltzing in here... where the heck is the guy that was supposed to bring this stuff?? The one who was supposed to bring us this damn stuff???" A group of workers rushes along with the Inspector and you as you try to explain what happened on the way to unload your ship. "Bugger that! He's damn'd well fired and if I see him again there's gonna be another damn Incident!!!"
    You wait to one side as the cargo is hauled off your ship at breakneck speed and wonder if you shouldn't have just dumped the stuff in space. Just as the last of the cargo is taken off your ship the Inspector, who has clearly cooled off a bit, comes up to you and says "Look, I know you were trying to do us a favour but next time don't bother if you can't make it on time, but thank you for delivering the stuff at all; we've had one or two wazzocks who just dumped it all in space", then he snears, "Didn't take long for it to catch up to them though..." As you leave you are happy to be away, even if you didn't get paid.]])
else
   -- Portrait Choice
   portrait = {"neutral/female1",
               "neutral/thief3",
               }

   misn_desc = _("You decided to help a fraught merchant at a bar by delivering some goods to %s for her.")

   text[1] = _([[    As you sit down the merchant looks up at you with a panicked expression, "Ahh! What do you want? Can you see I've enough on my plate as it is?" "Hey, lady, calm down...", you say, "Here, how about I buy you a drink and you tell me about it?" "Jeeze, that's nice of you... ha, maybe I can cut a break today!"
    You grab a couple of drinks and hand one to the slightly more relaxed looking merchant as she starts to talk. "So, I work for Wernet Trading and the deal is I transport stuff for them, they pay me, only I got kinda beat up fighting pirates on the way to the pick-up and now I'm realising that my engines just don't have the speed to get me back to beat the deadline and to top it off I'm late on my bills as it is so those engines are gonna have to last 'til this paycheck comes in... see where this is going? I'm in the Sol nebula without a shield generator."
    "Don't worry so much lady, I'm sure they'll cut you some slack if you're back only a little late", you say reassuringly. "The hell they will! If I don't get this %d tons of %s to %s the next gal's gonna be late and then the customer will be on us and we'll all get it from management, geeze... but that's a thought, if someone else gets it there in time then at least I'll only get a bit of grief... hey, you wouldn't take this stuff for me would you? It's needs to get to the %s system in %s, could you do that for me? I'd sure appreciate it!]])

   text[2] = _([[    The merchant stands up and hugs you, "Thank you so much for this, I've a guy at this dock who can sort the cargo transfer and here's the chit you need to give to the cargo gal at %s. Of course payment is done by credit chip so that's yours to keep but at least I'll have a little time to work on those engines before I have to get back on the job! I definietly owe you one for this."]])

   text[3] = _([[    Landing at the spaceport you see the Wernet Trading depot surrounded by a hustle and bustle. Once outside you head over to the cargo depot and hand over the chit the merchant gave you. The Cargo Inspector looks up at you in surprise and so you explain to her what happened as the cargo is unloaded from your ship. "Wow, thanks for the help, you definitely saved us a ton of trouble, here's your payment and if you're ever looking for a steady job give me a call - it's not great pay but we guarentee you'll get work, assuming you can make it on time, of course!" You laugh and part ways.]])

   text[4] = _([[    Coming down at the spaceport you see the Wernet Trading depot surrounded by a fraught buzz of activity. Once outside you thread your way through the throngs of running and shouting people and finally find the Cargo Inspector. You hand her the chit and she looks at you with contempt and distaste, "What in damnation is this! This shipment was supposed to have been here ages ago! You wouldn't believe what we've had to do to deal with this and then you come waltzing in here... where the heck is the lassie that was supposed to bring this stuff??" A group of workers rushes along with the Inspector as you try to explain what happened on the way to unload your ship. "If I ever get my hands on her she's gonna be mincemeat and I'll stuff her in pies and send them to her family, that bitch better never show her face here again, I'm telling you!"
    You timidly stand to one side as the cargo is hauled off your ship and wonder if you shouldn't have just dumped the stuff in space. Just as the last of the cargo is hauled away the Inspector calmly comes up to you and says "See, I know you were trying to do us a favour but next time don't unless you can make it. I should say 'thank you' for delivering the stuff at all", she adds, "but then the ones who ditch it learn one way or another..." As you leave you are happy to be away, even if you didn't get paid.]])
end

function create()
   -- Note: this mission does not make any system claims.

   -- Calculate the route, distance, jumps and cargo to take
   dest_planet, dest_sys, num_jumps, travel_dist, cargo, tier = cargo_calculateRoute()
   if dest_planet == nil or dest_sys == system.cur() then
      misn.finish(false)
   end

   misn.setNPC(_("Merchant"), portrait[rnd.rnd(1, #portrait)]) -- creates the merchant at the bar
   misn.setDesc(bar_desc) -- merchant's description

   stu_distance = 0.2 * travel_dist
   stu_jumps = 10300 * num_jumps
   stu_takeoff = 10300
   time_limit = time.get() + time.create(0, 0, stu_distance + stu_jumps + stu_takeoff)

    -- Allow extra time for refuelling stops.
    local jumpsperstop = 3 + math.min(tier, 3)
    if num_jumps > jumpsperstop then
        time_limit:add(time.create( 0, 0, math.floor((num_jumps-1) / jumpsperstop) * stu_jumps ))
    end

   payment = 5 * (stu_distance + (stu_jumps / 10))

   -- Range of 5-10 tons for tier 0, 21-58 for tier 4.
   cargo_size = rnd.rnd( 5 + 4 * tier, 10 + 12 * tier )
end

function accept()
   if not tk.yesno(title[1], text[1]:format(cargo_size, cargo, dest_planet:name(), dest_sys:name(), (time_limit - time.get()):str())) then
      misn.finish()
   end
   if player.pilot():cargoFree() < cargo_size then
      tk.msg(full_title, full_text)  -- Not enough space
      misn.finish()
   end
   player.pilot():cargoAdd(cargo, cargo_size)
   local player_best = cargoGetTransit(time_limit, num_jumps, travel_dist)
   player.pilot():cargoRm(cargo, cargo_size)
   if time_limit < player_best then
      if not tk.yesno(slow_title, slow_text:format((time_limit - time.get()):str(), (player_best - time.get()):str(), dest_planet:name())) then
         misn.finish()
      end
   end

   misn.accept()

   -- mission details
   misn.setTitle(misn_title)
   misn.setReward(misn_reward:format(numstring(payment)))
   misn.setDesc(misn_desc:format(dest_planet:name()))
   marker = misn.markerAdd(dest_sys, "low") -- destination
   cargo_ID = misn.cargoAdd(cargo, cargo_size) -- adds cargo

   -- OSD
   osd_msg = {osd_desc[1]:format(dest_planet:name(), dest_sys:name(), (time_limit - time.get()):str())}
   osd = misn.osdCreate(osd_title, osd_msg)

   tk.msg(title[2], text[2]:format(dest_planet:name()))

   intime = true
   faction = faction.get("Trader")
   land_hook = hook.land("land")
   date_hook = hook.date(time.create(0, 0, 42), "tick") -- 42STU per tick
end

function land()
   if planet.cur() == dest_planet then
      if intime then
         faction:modPlayerSingle(1)
         tk.msg(title[3], text[3])
         player.pay(payment)
      else
         faction:modPlayerSingle(-1)
         tk.msg(title[4], text[4])
      end
      misn.cargoRm(cargo_ID)
      misn.osdDestroy(osd)
      misn.markerRm(marker)
      hook.rm(land_hook)
      hook.rm(date_hook)
      misn.finish(true)
   end
end

function tick()
    if time_limit >= time.get() then -- still in time
        osd_msg = {osd_desc[1]:format(dest_planet:name(), dest_sys:name(), (time_limit - time.get()):str())}
    else -- missed deadline
        osd_msg = {osd_desc[2]:format(dest_planet:name(), dest_sys:name())}
        intime = false
        hook.rm(date_hook)
    end
    misn.osdCreate(osd_title, osd_msg)
end

function abort()
   misn.cargoRm(cargo_ID)
   player.msg(jet_msg:format(cargo))
   faction:modPlayerSingle(-5)
   misn.osdDestroy(osd)
   misn.markerRm(marker)
   hook.rm(land_hook)
   hook.rm(date_hook)
   misn.finish(false)
end
