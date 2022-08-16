--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Scavenger Escort Handler1">
 <location>load</location>
 <chance>100</chance>
 <cond>system.cur()~=system.get("Crimson Gauntlet")</cond>
 <unique />
</event>
--]]
--[[
   The player pays a fellow pirate to know where to steal a random ship.

   The player pays to get the position of a ship on a random planet of a random
   faction. When he gets there, the planet is guarded (which means he may have
   to fight his way through, which is the most probable option).

   When he lands on the target planet, he gets a nice message explaining what
   happens, he gets a new ship, is able to refit it, etc.

   Then, when the player wants to leave the planet, and that will eventually
   happen (at least, I hope…) he’ll be pursued by a few fighters.
--]]
-- luacheck: globals enter jumpout land player_rescue player_attacked scavenger_arrives scav_attacked scav_boarding setup_support_fleet scavenger_death fall_in enlist_followers scav_hail detonate_c4 (Hook functions passed by name)
-- luacheck: globals approachScavenger approachHiredScav escort_barConversation (NPC functions passed by name)

local pir = require "common.pirate"
local fmt = require "format"
local portrait = require "portrait"
local pilotname = require "pilotname"
local vntk = require "vntk"
local vn = require "vn"

require "factions.equip.generic"

local logidstr = "Hired Escorts"
local npcs = {}
local max_scavengers = 2
local max_escorts = 6

local chitchat_idle = {
    _("Let's have an adventure, shall we?"),
    _("I'm with you."),
    _("Reporting for duty."),
   _("Reporting in."),
   _("I'm ready."),
   _("Let's go."),
   _("Let's do this."),
   _("Let's get started, shall we?"),
    _("Where are we going again?"),
    _("I'm here."),
    _("{name} reporting for duty."),
   _("{rank} {name} reporting for duty."),
   _("{rank} {name} at your service."),
   _("{rank} in position."),
    _("Greetings."),
   _("Hello."),
   _("Hey."),
   _("Ahoy."),
   _("Ahoy!"),
   _("Hey there."),
   _("Did you miss me?"),
   _("Did I miss anything?"),
   _("It's too quiet here."),
    _("It's pretty silent in space. Makes you thankful for all the electronics."),
    _("What's that buzzing noise?"),
    _("Do you hear drumming?"),
   _("Do you hear that beat?"),
    _("I hope my {ship} is up to code."),
   _("I can feel it."),
   _("Let's see what this {ship} can do."),
   _("Please, don't let my {ship} fail me today."),
   _("Today we'll find out what this {ship} is made of."),
   _("{rank} {ship}son reporting for duty, commander!"),
}

local chitchat_commander = {
    _("It's a beautiful night."),
    _("It's a beautiful day."),
    _("It's been a long cycle."),
    _("I think the crew is itching for a chance to relax."),
    _("I haven't been to Kramer for too long."),
    _("Lieutenant Commander {name}. One day, right?"),
    _("{name} of the {dreamship}. Got a ring to it, doesn't it?"),
    _("Nothing like flying with a {dreamship} in your fleet, am I right?"),
    _("I hope you don't mind, I have an extra passenger in my cargo hold."),
   _("Have I ever told you any stories of my time at Zabween?"),
   _("Have I told you the story about the Cakkak on Taxumi IIIa?"),
   _("You know I used to be a bartender, right? I think this suits me more."),
   _("My mother escaped from Stutee, and well, I don't think I have to tell you what that means, but she'd be stuck at Leszec today."),
    _("I've got to tell you what my medical officer told me earlier the next time we grab a drink."),
   _("Remind me to tell you what my medical officer told me earlier the next time we grab a drink."),
   _("Remind me to tell you what one of my officers told me the next time we grab a drink."),
   _("Remind me to tell you what one of my crew told me the next time we have a minute."),
    _("Sometimes I think I'm the only fool to have ever loved Jayne."),
    _("The lawlessness on Mannannan is getting way out of hand."),
    _(
        "I tried my luck at Minerva Station, but I couldn't stop looking at that chicken. Still gives me the creeps to think about it."
    ),
    _(
        "The best passengers are the youths. They know their place. Anyone past sixty feels as if they've survived three apocalypses. I guess they kind of did."
    ),
    _("I'm your number two. You can count on me."),
    _("{rank} {name}. I like how that sounds."),
    _("{rank} {name}. That's me."),
    _("The name's {name}, {rank} {name}."),
    _("I hope my {ship} will make you happy."),
   _("What we all really want is our fair share of the action."),
}

-- in this one, the argument (spoken subject) is a random non-commander subordinate
local chitchat_commandleader = {
   _("{rank} {name} has been showing some promise recently."),
   _("{rank} {name}'s recent exploits haven't gone unnoticed."),
   _("I had a talk with the {rank}s earlier about that indicent I told you about."),
   _("I don't know how many {ship}s we need in our fleet, but I trust your judgment."),
   _("I don't know If I should tell you what happened in {name}'s {ship} earlier."),
   _("{name} told me something I never want to repeat. I just wanted to get that off my chest."),
   _("{name} told me a secret I didn't need to know. Had to get that off my heart."),
   _("I don't what can be so complicated about the {ship}, it's such a simple ship to fly."),
   _("I remember when I flew a {ship}."),
   _("Come on everyone, let's go."),
   _("Come on crew, let's hustle."),
   _("Come on squad, let's go. That means you too {name}."),
   _("Come on {name}, quit fooling around."),
   _("Settle down kids."),
   _("Sometimes I feel like these fools are my children."),
   _("Come on {name}, even my daughter could keep up in a {ship}."),
   _("Come on {rank}, my daughter could fly that {ship} better than you."),
   _("My kid could fly that {ship} better than you."),
   _("{rank} {name}'s {ship} will cost {replacement_text} to replace."),
   _("{rank} {name}'s performance in a {dreamship} above average."),
   _("When a {rank} is flying around in a {ship} and wants {replacement_text} to cover, do we really need that {rank}?"),
   _("Thanks for that thing I asked for. If anyone asks, it was for {name}."),
   _("One of the {rank}s on the roster can be quite irritating."),
   _("I won't say who did what, but I just wanted to let you all know that I definitely saw that."),
   _("I won't say what happened, but somebody keep an eye on {name}."),
   _("I should be a bit harder on {name}."),
}

local chitchat_rare = {
    _("I once caught a fish on Crylo that was so big that I needed a Gawain to transport it."),
    _("I used to work at Janus Station and dream about Kramer... I think prefer this."),
    _("I've heard most of these bounties we pick up alive end up at Wa'kan."),
    _("I would love one of those herbal teas from Manis."),
    _("It's cold and dark out here."),
    _("There's something eerie about all this."),
    _("I could tell you some stories... But I'd better keep my mouth shut."),
    _("I had an encounter on Mutris during a training mission. I'll spare you the details, but wow."),
    _(
        "In this cold, vast emptiness I often reminisce of my adolescent years, the cycle we spent at Zhang Lu climbing the mountains. I fell in love with the nature. Then I fell in love with Jayne."
    ),
   _("In case I haven't told you, my first name is {first_name}."),
    _("Would you believe my distant aunt used to keep her kid as a pet on Mannannan? Nobody cared back then either."),
    _("I thought I'd try piracy for a while. That's how I lost my first Shark."),
    _("I feel like we are on a quest for fire, but the land is all swamp."),
    _("If we stop at Janus Station, let me know a few jumps in advance so that I can go clean my ship or something."),
    _("I have some friends at Kramer I'd like to catch up with. Maybe we can go there sometime?"),
    _("I once had an escort of my own, but I could only pay for one cycle."),
   _("I could have sworn I saw a picture of one of the pilots in our fleet at the spaceport bar in Zabween."),
   _("I think we might have a rodent problem on my {ship}."),
   _("I feel like there's always something new."),
}

local chitchat_content = {
    _("It's a nice day, isn't it?"),
    _("I feel like I'm onto a winner."),
    _("The last bounty we got was pretty good."),
    _("That last bounty was alright."),
    _("This ship is not bad."),
   _("This is a good {ship}."),
    _("Where to?"),
    _("It's a good day to be gay. In the literal sort of way."),
    _("{name} at your service."),
    _("I hope you're not too bothered about having spent so much on me."),
    _("We're doing alright out here, aren't we?"),
    _("We're doing alright, aren't we?"),
    _("It could be worse."),
    _("Things could always be better, but I'm content."),
    _("Keep up the good work, boss."),
    _("Do you think I talk too much about the {dreamship}?"),
    _("If I were to measure my experience on a scale from one to a lot, I'd estimate it to be at about {experience}."),
    _("I could do this forever."),
    _("Who's up for another round?"),
    _("Who's up for round two?"),
    _("Shall we go again?"),
    _("Let's do that again."),
   _("Let's do that again?"),
   _("Some more?"),
    _("I have to admit, things could have gone worse."),
    _("Starry night."),
   _("I'm a happy {rank}, that's for sure.")
}

local chitchat_negative = {
    _("Is there anything to salvage around here? I'm saving up for a {dreamship}."),
    _("Can we please do something about me not being in a {dreamship} yet?"),
    _("I hope we find something interesting today."),
    _("I hope we find something expensive today."),
    _("I hope we find something worth plundering today."),
    _("The next ship we catch better be loaded!"),
    _("The next ship we disable could be the one!"),
    _("'Become a pilot', they said... Heh."),
    _("'Become a pilot', they said... Yeah, right."),
    _("'Become a pilot', they said... Yeah, right. A pilot!"),
    _("'Become a pilot', they said... Yeah, right. A pilot! In space!"),
    _("I got kids depending on me, you know..."),
    _("What are we even doing here?"),
    _("One day, I'll have my {dreamship}..."),
    _("Everything I do, I do for the chance to fly a {dreamship}."),
    _("Isn't there someone worth plundering around? I really want a {dreamship}."),
    _("My {dreamship} fund could use some bolstering."),
    _("Why is everyone so poor out here?"),
    _("Isn't there anything worth stealing nearby?"),
    _("Well, chop chop! Do a cargo run or something, let's get ambushed."),
    _("I'll be a lot generous with the bounty once I'm sitting in my {dreamship}."),
    _("I hope we don't get ambushed."),
   _("{rank}s like me always get into the most trouble."),
   _("A {dreamship} would be fitting for a {rank}."),
   _("I always miss the best action, or lose my ship trying to get in on it."),
   _("I always miss out on the best action."),
}

-- positive chatter
local chitchat_dreamship = {
    _("Is there any work to do around here?"),
    _("I'll bet we find something interesting today."),
    _("I love my {dreamship}."),
    _("I really love my {dreamship}."),
    _("Have I told you how much I appreciate my {ship}?"),
    _("This is the life."),
    _("Ahhh, this is the life. I owe it all to the {ship}."),
   _("Ah, this is the life. I owe it all to my {ship}."),
   _("This is the life. And I owe it all to this {ship}."),
    _("Everything will be okay, as long as I have my {dreamship}."),
    _(
        "I figure you've probably spent a lot on me by now, but it was all worth it. I'm thankful for every day I get with my {dreamship}."
    ),
    _("I'm thankful for every day I get with my {dreamship}."),
    _("I hope you realize that this thing is going to cost you {replacement_text} if I have to eject."),
    _("My {ship} will cost you {replacement_text} to cover. I hope it lasts, because I love it."),
    _("I'm in a good mood."),
    _("I've got a good feeling about this."),
    _("You know, you're insane but I like it."),
    _("Let's not scratch the paint on my {ship} today."),
    _("I just had this {dreamship} cleaned, can you notice?"),
   _("This {ship} is perfect for a {rank} like me.")
}

local chitchat_favoured = {
    _("Is there anything to do around here?"),
    _("I'll bet we find something interesting here."),
    _("I love my {ship}."),
    _("I really love my {ship}."),
    _("Have I told you how much I appreciate my {ship}?"),
    _("This is the life."),
    _("Ahhh, this is the life. I owe it all to this {ship}."),
   _("Ah, this is the life. I owe it all to my {ship}."),
   _("This is the life. And I owe it all to this {ship}."),
    _("Everything will be okay, as long as I have my {ship}."),
    _(
        "You know, this {ship} has actually kind of grown on me."
    ),
    _("I'm learning to appreciate every day I get with my {ship}."),
    _("I hope you realize that this thing is going to cost you {replacement_text} if I have to eject."),
    _("My {ship} will cost you {replacement_text} to cover. I hope it lasts, because I like it."),
    _("I'm in a good mood."),
    _("I've got a good feeling about this."),
    _("You know you're a bit bonkers but I can dig it."),
    _("Let's not scratch the paint on my {ship} today."),
    _("I just had this {ship} cleaned, can you notice?"),
   _("This {ship} is perfect for a {rank} like me.")
}

local chitchat_broken_dreams = {
   _("I had a {dreamship}. Now I'm in a {ship}."),
   _("I don't even like this {ship}. I miss the {dreamship}."),
   _("I miss my {dreamship}."),
   _("I can't believe I had to eject from that {dreamship}... Damn it!"),
   _("I can't believe I had to eject from that {dreamship}."),
   _("Why am I flying a {ship}?"),
   _("Maybe if you take me to a shipyard that sells the {dreamship} I'll stop being so negative all the time."),
   _("Oh, what I'd do for the chance to fly a {dreamship} again..."),
   _("Surely a new {dreamship} would be fitting for a {rank}...")
}

local chitchat_affirm = {
    _("Affirimative."),
    _("Roger that."),
    _("Ten four."),
    _("I'm with you."),
    _("Okay."),
    _("I understand."),
    _("Aye commander!"),
    _("Gotcha."),
    _("Understood."),
    _("On you."),
    _("With you."),
    _("Yes Sir."),
    _("Aye Sir."),
    _("For the {dreamship}!")
}

local chitchat_join = {
    _("I'll go too."),
    _("Sounds like a plan."),
    _("I'm up for that."),
    _("I will go too."),
    _("I'll join you."),
    _("Want some company?"),
    _("I'll go with {leader}."),
    _("I'm with {leader}."),
    _("Wait for me.")
}

-- we assume this comes from a leader-ish type
local chitchat_brb = {
    _("I'm going to take a look around for a bit."),
    _("I'll catch up with you later."),
   _("I'll catch up with you."),
   _("I'll be around."),
   _("Go on without me."),
    _("I'll be back."),
    _("Call me if you need me."),
    _("I shouldn't be hard to find if you need me..."),
    _("Let's see if there's anything interesting around..."),
   _("I'm on the watch."),
   _("I'm on duty."),
   _("We're rolling."),
   _("Let's board something large."),
   _("Time to show you what I can really do."),
   _("Let's see what this {ship} can do."),
   _("Let's see what I can get done with this {ship}."),
   _("Let's get the crew going."),
   _("Second in command {name}, reporting for duty."),
   _("Second in command {rank} {name}, reporting in."),
   _("{rank} {name} activating combat protocols."),
   _("Alright, let's gear up."),
   _("Ready up."),
   _("Look alive, everyone."),
   _("Look alive, people."),
   _("Look alive, pilots."),
   _("On me."),
   _("I'm on patrol."),
   _("I'm on salvage duty."),
}

local chitchat_beating = {
    _("We've had our fair share of scuffles recently, huh."),
    _("These dings and scratches pile on faster than I can repair them."),
    _("Man, we've really taken a beating haven't we?"),
    _("Let's face it, we took a beating."),
    _("I'll admit, I took a beating."),
    _("They almost had me there for a while."),
    _("Let's face it, we took a beating."),
    _("I hope we don't get ambushed again."),
    _("I hope we don't get ambushed. The last time was rough."),
    _("That was rough."),
    _("It feels great to be alive, but let's not do that again."),
    _("What a pansy."),
    _("A worthy opponent."),
    _("Yeah, I'm out."),
    _("Not the bloody sensors again!"),
    _("My shields are failing me."),
    _("I could use some upgrades to my shields."),
    _("Someone take over, I'm getting pummelled over here."),
    _("Oh, that's not coming out, that's a whole new panel, drats!"),
    _("Oh well, you win some, you lose some."),
    _("Sometimes, the only way to win is to refuse to play.")
}

local chitchat_closecall = {
   _("You know about earlier... That was really close."),
   _("I thought you were gonna let me lose this {ship}."),
   _("That was really close back there. Let's not do that again."),
   _("Let's not do that again."),
   _("That was really close back there."),
   _("For a minute there, I thought we were toast."),
   _("A little close for comfort."),
   _("I've seen quieter days."),
   _("Let's not go back there for a while."),
   _("My {ship} is looking a little ill-equipped."),
   _("I think it's time to upgrade my {ship}."),
   _("For a minute there I thought you were going to pay the {replacement_text} and leave me."),
}

local chitchat_goodhaul = {
    _("That last haul brought in a decent profit."),
    _("I'm quite happy with the loot from earlier."),
    _("What a great haul."),
    _("Made a nice profit today."),
    _("One step closer to my {dreamship}."),
    _("If you check your logs, you'll find that I sold some plunder recently."),
   _("Nothing like finally selling off your haul."),
   _("Nothing like selling off some loot."),
   _("Nothing like selling some plunder."),
   _("I love it when we manage to unload the cargo."),
   _("An empty cargohold makes for a smoother ride."),
   _("Next time we're full of loot and plunder, let's come back here."),
}

local chitchat_cargofull = {
    _("My cargohold is full."),
    _("I'll be happy with the loot if we manage to sell it somewhere."),
    _("What a great haul. If we can sell it."),
    _("Made a nice profit today... If we manage to drop it off somewhere."),
    _("I'll be one step closer to my {dreamship} once I sell these commodities."),
    _("Can we drop this stuff off anywhere?"),
   _("I really want to sell off my haul."),
   _("I need to get rid of this loot."),
   _("I need to sell all this plunder."),
   _("I'd love it if we managed to unload all this cargo."),
   _("An empty cargohold makes for a smoother ride. My cargohold is stuffed."),
   _("We're full of loot and plunder, let's go sell it somewhere."),
}

local chitchat_malfunction = {
    _("My {thing} is malfunctioning, hopefully I can repair it soon."),
    _("There's something wrong with my {thing}."),
    _("Oh man, not this again... It's my {thing}."),
    _("If you see a spare {thing} anywhere, this one needs replacing."),
    _("My ship is acting up a bit, bear with me."),
    _("I'm really in for it now."),
    _("One of my panels is fried, I got this thing second hand."),
    _("One of my panels is fried."),
    _("Most of my crew is busy performing ship repairs."),
    _("What do you do when a {thing} malfunctions?"),
   _("What do you do when a {thing} is malfunctioning?"),
    _("I've had it with all these malfunctions!"),
    _("I really need to get settled into one ship."),
    _("I don't know how to fix the {thing} on my ship."),
    _("I shouldn't have {thing} problems on a {dreamship}."),
    _("I'm still a bit beat mentally from that last bout."),
    _("I need more time to complete repairs."),
   _("I need more time to get used to this ship."),
   _("I need some time to get used to the controls on this ship."),
   _("I need some time to configure the navigation equipment on this ship."),
   _("I need to reconfigure the targeting equipment on this ship."),
   _("I need time to reroute the engines on my ship."),
   _("My sensors are giving me incorrect readings."),
   _("Huh, how about that, this screen is broken."),
   _("You don't want to know what the previous owner did in here."),
   _("You don't want to know what the last owner did to this ship."),
   _("Don't ask what happened in here. Frankly, I don't want to know."),
   _("This shield emitter isn't even plugged into power, what else is wrong with this ship?"),
   _("I don't know how, but my {thing} is keeping my docking clamps locked for some reason."),
   _("I am never going to get used to this thing, am I?"),
   _("I haven't been able to configure everything properly on this ship yet."),
    _("Some of the second hand equipment in here is useless."),
    _("The next time we land, remind me do something about these failures."),
    _("This {thing} is a bad counterfeit. It doesn't even work as advertised."),
   _("My {thing} is counterfeit. I wonder if it even works."),
   _("Oh man, not again... This useless hunk of junk."),
   _("What the... Damn it! This useless second hand space coffin..."),
    _("This second hand hunk of junk, please don't fail me now..."),
    _("Whoever rewired this ship had no idea what they were doing."),
    _("How many {thing}s do I need to get one that just works?")
}

local function pick_one(target)
    local r = rnd.rnd(1, #target)
    return target[r]
end

local function pilot_portrait( commander )
   local choices = {
      "neutral/pilot4.webp",
      "neutral/pilot5.webp",
      "neutral/pilot6.webp",
      "neutral/pilot7.webp",
   }
   if commander then
      choices = {
      "neutral/pilot1.webp",
      "neutral/pilot2.webp",
      "neutral/pilot3.webp",
   }
   end

   return pick_one(choices)
end

local function pick_favorite_ship(me)
    -- egde case create if no exist
    if me.favorite_ships == nil then
        me.favorite_ships = {}
    end

    local max_score = 0
    local choices = {}

    -- lazy method favors first-seen ships
    for ship, score in pairs(me.favorite_ships) do
        if score >= max_score then
            max_score = score
        end
    end

    local min_score = math.floor(max_score / 3)
--    print(fmt.f("the minimum score is {ms}", {ms = min_score}))

    for ship, score in pairs(me.favorite_ships) do
        if score >= min_score then
            table.insert(choices, ship)
--            print(fmt.f("Considering a {ship} worth {points}", {ship = ship, points = score}))
--        else
--            print("Skipping ", ship)
        end
    end

    if next(choices) == nil then
        choices = {"Shark"} -- give it a default
    end

    local choice = choices[rnd.rnd(1, #choices)]

    return choice
end

local function speak(persona, sentiment, arg)
    local ss = sentiment or "idle"
    local spoken = nil
    local force = false
    local speaker = persona.pilot

    if arg ~= nil and arg["speaker"] then
        speaker = arg["speaker"]
    end

    if ss == "idle" then
      local favourite = pick_favorite_ship(persona)
      local myship = persona.ship:nameRaw()
        if persona.last_sentiment then
            local last = persona.last_sentiment
            persona.last_sentiment = nil
            return speak(persona, last)
        elseif rnd.rnd() < 0.042 then
            spoken = pick_one(chitchat_rare)
        elseif persona.commander then
         -- if we have subordinates, also pick from chitchat_commandleader (basically, "I have a reason to criticize my crew")
         local some_subordinate = nil
         -- use randomness here to favor picking persons with a higher index (probably newer crew, but it can be sorted in land() )
         -- also favors picking this kind of chitchat less for smaller crews
         -- use the experience difference as a reason to critcize the subordinate
         for _i, other in ipairs(mem.persons) do
            if other ~= persona and rnd.rnd(0, 1) == 0 and persona.experience > other.experience then
               some_subordinate = other
            end
         end
         -- usually we talk to the player, but sometimes we talk to or about the subordinates
         -- use the chatter variable since a quiet type would rather say hello than try to rally everyone
         if some_subordinate and rnd.rnd() > 0.3 + persona.chatter then
            spoken = pick_one(chitchat_commandleader)
            persona = some_subordinate
         else
            spoken = pick_one(chitchat_commander)
         end

      elseif   -- if this looks like a favourite ship
            string.find(myship, favourite) or
            string.find(favourite, myship)
         then
         if rnd.rnd(0, 1) == 0 then
            spoken = pick_one(chitchat_idle)
         else
            spoken = pick_one(chitchat_favoured)
         end
        else
            spoken = pick_one(chitchat_idle)
        end
    elseif ss == "content" then
        spoken = pick_one(chitchat_content)
    elseif ss == "dreamship" then
        spoken = pick_one(chitchat_dreamship)
    elseif ss == "negative" then
        if string.find(persona.ship:nameRaw(), persona.dreamship) then
            spoken = pick_one(chitchat_dreamship)
        else
         spoken = pick_one(chitchat_negative)
        end
   elseif ss == "broken_dreams" then
      -- make sure you don't complain if you replaced your dream ship with a new dream ship
       if string.find(persona.ship:nameRaw(), persona.dreamship) then
            spoken = pick_one(chitchat_dreamship)
        else
         spoken = pick_one(chitchat_broken_dreams)
        end
    elseif ss == "malfunction" then
        -- if this is the sentiment but there is no argument, that's because we want to stay silent since last time this was our sentiment
        if not arg then
            return
        end
        spoken = fmt.f(pick_one(chitchat_malfunction), {thing = arg.thing, dreamship = "{dreamship}"})
        persona.last_sentiment = "malfunction" -- skip the next idle chatter
    elseif ss == "affirm" then
        spoken = pick_one(chitchat_affirm)
    elseif ss == "join" then
        spoken = fmt.f(pick_one(chitchat_join), {leader = arg.name})
    elseif ss == "brb" then
        force = true
        spoken = pick_one(chitchat_brb)
    elseif ss == "beating" then
        spoken = pick_one(chitchat_beating)
    elseif ss == "goodhaul" then
        spoken = pick_one(chitchat_goodhaul)
      -- stay happy for a while
      persona.last_sentiment = "content"
   elseif ss == "cargofull" then
      spoken = pick_one(chitchat_cargofull)
      -- it's a good thing our cargo was full, but now that we've notified the player we can be happy about it
      persona.last_sentiment = "content"
   elseif ss == "closecall" then
      spoken = pick_one(chitchat_closecall)
      -- deescalate our feelings of almost dying and contemplate the beating we just received
      persona.last_sentiment = "beating"
    end

    if not spoken then
        print("spoken:", sentiment, arg)
    end

    spoken = spoken or _("I'm at a loss for words.")

    if speaker == nil then
        print("error speaker nil:", persona, sentiment, arg)
    end

    -- speak
    speaker:comm(fmt.f(spoken, persona), force)
end

local function rank_name(rank_points)
    local rank = "Novice"
    if rank_points > 100 then
        rank = "Lieutenant"
    elseif rank_points >= 50 then
        rank = "Lieutenant, junior grade"
    elseif rank_points >= 17 then
        rank = "Ensign"
    elseif rank_points > 6 then
        rank = "Cadet"
    end

    return rank
end

local function remember_ship(me, ship, points)
    -- egde case create if no exist
    if me.favorite_ships == nil then
        me.favorite_ships = {}
    end

    -- edge case: never seen the ship before
    if me.favorite_ships[ship] == nil then
        me.favorite_ships[ship] = points
        return
    end


    local current_score = me.favorite_ships[ship]

   -- if we really really love this ship, we should recalculate favorites
   -- we halve every score except our own, but subtract the biggest penalty from ourselves aswell
   -- this way, we still like our 2nd favourite just as much compared to our current favourite until we forget it
   -- seems complicated but allows the player to have the commander switch ships without having to grind the favour back as much
   if current_score > 36 then
      local max_penalty = 5
      for fship, score in pairs(me.favorite_ships) do
         print(fmt.f(
         "adjusting {ship} from {score}",
         { ship = fship, score = score }
         ))
         if ship ~= fship then
            local spenalty = math.ceil(score / 2)
            me.favorite_ships[fship] = score - spenalty
            if 1 == me.favorite_ships[fship] then
               me.favorite_ships[fship] = nil
            elseif spenalty > max_penalty then
               max_penalty = spenalty
            end
         end
      end
      me.favorite_ships[ship] = current_score + points - max_penalty
   else
      -- default: increase
      me.favorite_ships[ship] = current_score + points
      print(
         fmt.f(
            "{name} remembers {ship} is worth {points} points.",
            {name = me.name, ship = ship, points = points + current_score}
         )
      )
   end
    return
end

local function getCredentials(edata)
    local credentials =
        _(
        [[
Pilot name: {first_name} {name}
Rank: {rank} ({experience} merit)
Ship: {ship}
Goal: {dreamship}
Deposit: {deposit_text}
Cover fee: {royalty_percent:.1f}% of deposit]]
    )

   return fmt.f(credentials, edata)
end

local function getFinances(edata)
    local finances =
        _([[
Money: {credits}
Expenses to date: {total}
Escort debt: {debt}
Escort profits: {profits}
Escort paid tribute: {tribute} ]]
    )
   return fmt.f(
      finances,
      {
         credits = fmt.credits(edata.wallet),
         total = fmt.credits(edata.total_cost),
         debt = fmt.credits(edata.debt),
         profits = fmt.credits(edata.total_profit),
         goal = edata.dreamship,
         tribute = fmt.credits(edata.tribute)
      }
   )
end

local function getOfferText(approachtext, edata)
    return (
      approachtext ..
        "\n\n" ..
      getCredentials(edata) ..
      "\n\n" ..
      getFinances(edata)
   )
end

-- note : the pilot doesn't change, but some data might
local function _createReplacementShip(persona, limit_ships)
    local ship_choices = {
        {ship = "Llama", royalty = 0.14},
        {ship = "Hyena", royalty = 0.15},
        {ship = "Koala", royalty = 0.16},
        {ship = "Gawain", royalty = 0.17},
        {ship = "Quicksilver", royalty = 0.2}
    }
    local budget =
        math.floor(
        persona.wallet * (1 - persona.royalty) + (persona.deposit * persona.royalty) +
            math.min(10e6, persona.tribute * persona.experience * 0.01)
    )

    if persona.pilot then
        local pmem = persona.pilot:memory()
        local boarded = pmem.boarded or 0
        --      persona.pilot:broadcast(fmt.f("I have boarded {boarded} times and made {profit} total.", { boarded=boarded, profit=persona.total_profit } ))
        if boarded > 0 then
            persona.experience = persona.experience + 1
            remember_ship(persona, persona.ship:nameRaw(), boarded)
        end
    end

    if persona.experience >= 1 then
        budget = budget + math.floor(math.min(25, persona.experience) * persona.royalty * persona.deposit)
    end

    local dream_budget = ship.get(persona.dreamship):price()

    -- TODO Here: license restrictions and limits
    if limit_ships ~= nil and #limit_ships > 0 then
        local most_expensive = "Shark"
        local most_cost = 1e3
        ship_choices = {}
        -- just add the available ships at standard rates
        local rate
        for i, ship in ipairs(limit_ships) do
            local price = ship:price()
            rate = math.min(0.95, 0.19 + 0.07 * ship:size())
            -- don't buy the ship if it's much more expensive than our dream ship
         -- unless if we are the commander, we want the player to be able to give the commander whatever he wants
            if budget > price and (dream_budget > price - 500e3 or persona.commander) then
                table.insert(ship_choices, {ship = ship:nameRaw(), royalty = rate})
                if price > most_cost then
                    most_cost = price
                    most_expensive = ship:nameRaw()
                end
            end
         -- finally if this looks like our dream ship, add it more to increase our chances of buying it if we can afford it
         if string.find(ship:nameRaw(), persona.dreamship) then
            table.insert(ship_choices, {ship = ship:nameRaw(), royalty = 0.21})
            table.insert(ship_choices, {ship = ship:nameRaw(), royalty = 0.25})
         end
         -- same if this looks like a favorite ship, increase our chances a lot since it's sold at this shipyard
         -- e.g. if we pick a favorite vigilance, we'll want the dvaered vigilance but not vice versa
         local some_favorite = pick_favorite_ship(persona)
         if string.find(ship:nameRaw(), some_favorite) then
            table.insert(ship_choices, {ship = ship:nameRaw(), royalty = 0.22})
            table.insert(ship_choices, {ship = ship:nameRaw(), royalty = 0.24})
            table.insert(ship_choices, {ship = ship:nameRaw(), royalty = 0.27})
         end
        end
      -- add the most expensive ship again
      table.insert(ship_choices, {ship = most_expensive, royalty = 0.50})
        -- if we are the commander, just buy the most expensive ship we can afford at a good rate
        if persona.commander then
            ship_choices = {
                {ship = most_expensive, royalty = 0.10 + 0.05 * rnd.sigma()}
            }
        end
    else
        if budget > 20e6 and persona.commander ~= nil then
            -- give it a rainmaker or kestrel...
            if persona.faction == faction.get("Empire") then
                table.insert(ship_choices, {ship = "Empire Hawking", royalty = 0.45})
                table.insert(ship_choices, {ship = "Empire Rainmaker", royalty = 0.45})
            elseif persona.faction == faction.get("Pirate") then
                table.insert(ship_choices, {ship = "Pirate Kestrel", royalty = 0.74})
                table.insert(ship_choices, {ship = "Pirate Kestrel", royalty = 0.75})
                table.insert(ship_choices, {ship = "Pirate Kestrel", royalty = 0.79})
            else
                table.insert(ship_choices, {ship = "Kestrel", royalty = 0.71})
                table.insert(ship_choices, {ship = "Kestrel", royalty = 0.72})
                table.insert(ship_choices, {ship = "Hawking", royalty = 0.73})
            end
            -- maybe the dream ship
            table.insert(ship_choices, {ship = persona.dreamship, royalty = 0.10})
         -- maybe a favorite ship
         table.insert(ship_choices, {ship = pick_favorite_ship(persona), royalty = 0.15 + rnd.sigma() * 0.05})
        elseif budget > 20e6 then
         -- we have so much money, spend it on something like a zebra or a dream ship
            table.insert(ship_choices, {ship = "Zebra", royalty = 0.50})
            table.insert(ship_choices, {ship = "Zebra", royalty = 0.69})
            table.insert(ship_choices, {ship = persona.dreamship, royalty = 0.14})
         table.insert(ship_choices, {ship = pick_favorite_ship(persona), royalty = 0.25 + rnd.twosigma() * 0.05})
        end
        if budget > 8e6 then
            -- hopefully a starbridge
            if persona.faction == faction.get("Pirate") then
                table.insert(ship_choices, {ship = "Pirate Starbridge", royalty = 0.46})
            elseif persona.faction == faction.get("Dvaered") then
                table.insert(ship_choices, {ship = "Dvaered Vigilance", royalty = 0.55})
                table.insert(ship_choices, {ship = "Dvaered Phalanx", royalty = 0.55})
            elseif persona.faction == faction.get("Empire") then
                table.insert(ship_choices, {ship = "Empire Pacifier", royalty = 0.45})
            else
                table.insert(ship_choices, {ship = "Starbridge", royalty = 0.44})
                table.insert(ship_choices, {ship = "Vigilance", royalty = 0.45})
                table.insert(ship_choices, {ship = "Pacifier", royalty = 0.47})
            end
            -- dream ship chance
            if rnd.rnd() > 0.66 then
                table.insert(ship_choices, {ship = persona.dreamship, royalty = 0.25})
            end
         -- favorite ship chance, decent rate
         table.insert(ship_choices, {ship = pick_favorite_ship(persona), royalty = 0.3 + rnd.threesigma() * 0.05})
        elseif budget > 5e6 then
            -- only get the cargos if you can't afford a starbridge
            if persona.faction == faction.get("Pirate") then
                table.insert(ship_choices, {ship = "Pirate Rhino", royalty = 0.35})
                table.insert(ship_choices, {ship = "Pirate Admonisher", royalty = 0.35})
            elseif persona.faction == faction.get("Empire") then
                table.insert(ship_choices, {ship = "Empire Admonisher", royalty = 0.35})
            else
                table.insert(ship_choices, {ship = "Rhino", royalty = 0.30})
                table.insert(ship_choices, {ship = "Vigilance", royalty = 0.42})
            end
            -- the Rhino dreamer doesn't want a mule at all and won't spend money on it
            if not string.find(persona.dreamship, "Rhino") then
                table.insert(ship_choices, {ship = "Mule", royalty = 0.25})
            end
        end
        if budget >= 2e6 then
            if persona.faction == faction.get("Pirate") then
                table.insert(ship_choices, {ship = "Pirate Shark", royalty = 0.24})
                table.insert(ship_choices, {ship = "Pirate Hyena", royalty = 0.17})
                table.insert(ship_choices, {ship = "Pirate Vendetta", royalty = 0.21})
            elseif persona.faction == faction.get("Dvaered") then
                table.insert(ship_choices, {ship = "Dvaered Vendetta", royalty = 0.23})
                table.insert(ship_choices, {ship = "Dvaered Ancestor", royalty = 0.24})
            elseif persona.faction == faction.get("Empire") then
                table.insert(ship_choices, {ship = "Empire Shark", royalty = 0.22})
            else
                table.insert(ship_choices, {ship = "Shark", royalty = 0.22})
            end
         -- also always allow the favorite ship at this budget at a reasonable rate
         table.insert(ship_choices, {ship = pick_favorite_ship(persona), royalty = 0.35 + rnd.twosigma() * 0.05})
        end
    end

    -- if we removed everything, give standard choices at better rates since we're probably being upgraded
    if next(ship_choices) == nil then
        ship_choices = {
            {ship = "Gawain", royalty = 0.06},
            {ship = "Koala", royalty = 0.07},
            {ship = "Quicksilver", royalty = 0.08}
        }
    end

    local shipchoice = ship_choices[rnd.rnd(1, #ship_choices)]
    -- if we have "enough" then roll the dice for a dream ship (1/3 chance)
   -- if our dreamship is a starbridge and we got a random favorite pirate starbridge, we keep the pirate one
    if budget > dream_budget and not string.find(shipchoice.ship, persona.dreamship) and rnd.rnd() < 0.33 then
        if shipchoice.dreamship ~= "Shark" then
            shipchoice = {ship = persona.dreamship, royalty = 0.15}
        end
    elseif rnd.rnd() < 0.16 then -- let's pick a favorite ship instead, or a shark
        shipchoice = {ship = pick_favorite_ship(persona), royalty = 0.25 + rnd.threesigma() * 0.05}
--        print("Picking a favorite ship ", shipchoice.ship, " for ", persona.name)
    end

    -- final check, don't allow ships that we don't have the license for
    -- TODO

    persona.ship = ship.get(shipchoice.ship)
    local _n, deposit = persona.ship:price()
    persona.outfits = {}
   local used_faction = persona.faction
   if persona.ship:tags("pirate") then
      used_faction = faction.get("Pirate")
   end
    local pppp = pilot.add(shipchoice.ship, used_faction)
    for j, o in ipairs(pppp:outfitsList()) do
        deposit = deposit + o:price()
        persona.outfits[#persona.outfits + 1] = o:nameRaw()
    end
    pppp:rm()
    persona.royalty = (shipchoice.royalty + 0.05 * shipchoice.royalty * rnd.sigma())
    persona.deposit = math.floor(deposit - (deposit * persona.royalty)) / 3
   if string.find(shipchoice.ship, persona.dreamship) then
      -- a discount for dream livers
        persona.deposit = math.max(100e3, persona.deposit - 1e6)
        persona.royalty = math.min(persona.royalty, 0.25 + 0.05 * shipchoice.royalty * rnd.threesigma())
    elseif #mem.persons > 3 and persona then
      -- punish larger fleet additions if we're dreamy
        persona.deposit = persona.deposit + #mem.persons * 37500 * persona.ship:size()
    end
    -- if we are good money makers, we don't want as much deposit
    if persona.total_cost - persona.total_profit * persona.experience >= deposit then
        persona.deposit = math.floor(math.max(persona.deposit, 250e3))
    end

    persona.royalty_percent = persona.royalty * 100
    persona.deposit_text = fmt.credits(persona.deposit)
    persona.replacement_text = fmt.credits(persona.deposit * persona.royalty)

    -- we pay 2/3 of the price at most and sometimes we get a bigger loan
    local payment = math.floor(math.min(rnd.rnd(1, 2) * deposit * persona.royalty, deposit * 0.67))
    -- only pay for it if we can afford it and if it was significant
   -- unless we are filthy rich...
    if deposit > 3e6 and persona.wallet > payment then
        -- and of course, we don't pay for the whole thing in cash now that we have credit
        if persona.pilot ~= nil then
            persona.pilot:credits(-deposit)
        end
        persona.wallet = math.max(0, persona.wallet - payment)
        persona.debt = persona.debt + deposit - payment
    else -- don't pay much for this with escort's money
        persona.debt = persona.debt + deposit
        -- if we bought something huge on credit, lower the money significantly
        local downpayment
        if deposit > 4e6 then
            downpayment = persona.wallet * 0.34
            persona.wallet = math.floor(persona.wallet - downpayment)
        elseif deposit > 2e6 then
            -- we bought something small but kind of expensive
            downpayment = persona.wallet * 0.042
            persona.wallet = math.floor(persona.wallet - downpayment)
        end
    end

   -- check if we were filthy rich and pay off debt
   if persona.wallet > persona.debt * 2 + deposit * 3 then
      persona.wallet = persona.wallet - persona.debt
      persona.debt = 0
   end

    if persona.pilot ~= nil then
        persona.pilot:broadcast(
            fmt.f(
                "My budget was {budget} and I chose a {ship} worth {deposit}. I have {credits} left and will cost up to {fee} to replace ({royalty_percent:.1f}%).",
                {
                    budget = fmt.credits(budget),
                    ship = persona.ship,
                    deposit = fmt.credits(deposit),
                    credits = fmt.credits(persona.pilot:credits()),
                    fee = persona.replacement_text,
                    royalty_percent = persona.royalty_percent
                }
            )
        )
    end

end

local function createReplacementShip(i)
    return _createReplacementShip(mem.persons[i])
end

-- create a pilot with attractive looking numbers
local function create_pilot(fac)
    local ship_choices = {
        {ship = "Llama", royalty = 0.04},
        {ship = "Hyena", royalty = 0.05},
        {ship = "Shark", royalty = 0.09},
        {ship = "Gawain", royalty = 0.07},
        {ship = "Koala", royalty = 0.06},
        {ship = "Quicksilver", royalty = 0.1},
        {ship = "Mule", royalty = 0.15}
    }
    local dream_choices = {
        "Shark",
        "Gawain",
        "Koala",
        "Quicksilver",
        "Lancelot",
        "Vendetta",
        "Ancestor",
        "Admonisher",
        "Starbridge",
        "Pacifier",
        "Vigilance",
        "Rhino",
        "Zebra"
    }

    local big_dreams = {
        "Kestrel",
        "Hawking",
        "Pirate Kestrel",
        "Empire Hawking",
        "Empire Rainmaker",
        "Dvaered Retribution"
    }

    local normal_dreams = {
        "Kestrel",
        "Hawking",
        "Pirate Kestrel",
        "Pirate Rhino"
    }

    local can_dream = true
    -- check if we have any commanders
    for i, persona in ipairs(mem.persons) do
        if persona.commander then
            can_dream = false
        end
    end
    -- "wanna replace your existing commander?" chance
    if rnd.rnd() < 0.12 then
        can_dream = true
    end
    local pf = spob.cur():faction()
    local portrait_arg = nil
    local name_func = pilotname.human

   if rnd.rnd() < 0.3 then
      -- create a random non-faction pilot
      pf = fac
   end

    if pir.factionIsPirate(pf) then
        ship_choices = {
            {ship = "Pirate Hyena", royalty = 0.1},
            {ship = "Pirate Shark", royalty = 0.15},
            {ship = "Pirate Vendetta", royalty = 0.2},
            {ship = "Pirate Ancestor", royalty = 0.25},
            {ship = "Pirate Admonisher", royalty = 0.35},
            {ship = "Pirate Phalanx", royalty = 0.35}
        }
        dream_choices = {
            "Pirate Starbridge",
            "Pirate Rhino",
            "Pirate Admonisher",
            "Pirate Phalanx",
            "Pirate Ancestor"
        }
        if can_dream then
            table.insert(dream_choices, "Pirate Kestrel")
            table.insert(dream_choices, "Pirate Kestrel")
        end
        fac = faction.get("Pirate")
        name_func = pilotname.pirate
        portrait_arg = "Pirate"
    elseif pf == faction.get("Empire") then
        table.insert(ship_choices, {ship = "Empire Shark", royalty = 0.15})
        table.insert(ship_choices, {ship = "Empire Lancelot", royalty = 0.15})
        dream_choices = {
            "Empire Pacifier",
            "Empire Admonisher"
        }
        portrait_arg = "Empire"
        if can_dream and fac:playerStanding() > 30 then
            table.insert(dream_choices, "Empire Hawking")
            table.insert(dream_choices, "Empire Rainmaker")
        end
    elseif pf == faction.get("Dvaered") then
        table.insert(ship_choices, {ship = "Dvaered Ancestor", royalty = 0.4})
        table.insert(ship_choices, {ship = "Dvaered Vendetta", royalty = 0.2})
        dream_choices = {
            "Dvaered Vigilance",
            "Koala",
            "Dvaered Phalanx",
            "Dvaered Ancestor"
        }
        if can_dream and fac:playerStanding() > 30 then
            table.insert(dream_choices, "Dvaered Retribution")
        end
        fac = faction.get("Dvaered")
        portrait_arg = "Dvaered"
    elseif pf == faction.get("Goddard") then
      -- random chance for a Goddard commander if the player has high standings
       if can_dream and fac:playerStanding() >= 30 then
            table.insert(dream_choices, "Goddard")
        end
   elseif pf == faction.get("Thurion") then
        ship_choices = {
            {ship = "Thurion Ingenuity", royalty = 0.15},
            {ship = "Thurion Scintillation", royalty = 0.25},
            {ship = "Thurion Virtuosity", royalty = 0.35},
            {ship = "Thurion Apprehension", royalty = 0.5}
        }
        dream_choices = {
            "Thurion Ingenuity",
            "Thurion Scintillation",
            "Thurion Virtuosity",
            "Thurion Apprehension"
        }
        fac = faction.get("Thurion")
        portrait_arg = "Thurion"
    elseif can_dream then
        -- regular commander (if he picks a big dream)
        for _i, dreamy in ipairs(normal_dreams) do
            table.insert(dream_choices, dreamy)
        end
    end

   local lastname
   local firstname
   lastname, firstname = name_func()
   if not firstname then
      firstname = "The"
      local delim_ind = string.find(lastname, " ")
      if delim_ind then
         firstname = string.sub(lastname, 0, delim_ind - 1)
         lastname = string.sub(lastname, delim_ind + 1)
      end
   end

    local shipchoice = ship_choices[rnd.rnd(1, #ship_choices)]
    local p = pilot.add(shipchoice.ship, fac)
    local _n, deposit = p:ship():price()

    local newpilot = {}

    newpilot.outfits = {}
    for j, o in ipairs(p:outfitsList()) do
        deposit = deposit + o:price()
        newpilot.outfits[#newpilot.outfits + 1] = o:nameRaw()
    end

    newpilot.deposit = math.floor((deposit + 0.1 * deposit * rnd.sigma()) / 4)
    newpilot.debt = math.abs(deposit - newpilot.deposit)
    deposit = newpilot.deposit
    -- after the first two escorts, people start considering you more serious
    if #mem.persons > 2 then
        newpilot.deposit = newpilot.deposit + #mem.persons * 125e3 * p:ship():size()
    end
    newpilot.ship = ship.get(shipchoice.ship)
    newpilot.deposit_text = fmt.credits(newpilot.deposit)
    newpilot.name = lastname
   newpilot.first_name = firstname
    newpilot.faction = fac
    newpilot.replacement_fee = deposit * shipchoice.royalty
    newpilot.replacement_text = fmt.credits(newpilot.deposit * shipchoice.royalty)
    newpilot.royalty = shipchoice.royalty + 0.05 * shipchoice.royalty * rnd.sigma()
    newpilot.royalty_percent = newpilot.royalty * 100
    newpilot.wallet = math.floor(deposit * 0.3)
    newpilot.total_profit = 0
    newpilot.tribute = 0
    newpilot.total_cost = newpilot.deposit
    newpilot.experience = 0
    newpilot.rank = "Recruit"
    newpilot.alive = true
    newpilot.spawning = false
    newpilot.portrait = portrait.get(portrait_arg)
   newpilot.portrait_offduty = newpilot.portrait
   newpilot.portrait_onduty = pilot_portrait()
   newpilot.vncharacter = portrait.getFullPath(newpilot.portrait)
    newpilot.chatter = math.min(0.96, math.max(0.04, 0.5 + 0.3 * rnd.threesigma())) -- add some personality flavour
    newpilot.dreamship = dream_choices[rnd.rnd(1, #dream_choices)]
    for _j, shipname in ipairs(big_dreams) do
        -- check if we are a commander
        if newpilot.dreamship == shipname then
            newpilot.commander = {}
         -- we need a new portrait then, possibly even a new name
         newpilot.portrait = pilot_portrait(true)
         newpilot.vncharacter = pick_one({
            "placeholder.png",
            "scavenger1.png",
            "neutral/female1.webp",
            "neutral/female2.webp",
            "neutral/female2_nogog.webp",
            "neutral/female3.webp",
--            "neutral/female4.webp", -- this one has a baby, would need to be a kid that can stay "the same age" for a while
         })
         newpilot.portrait_offduty = string.gsub(string.gsub(newpilot.vncharacter, ".webp", "n.webp"), "_nogogn", "n_nogog")
         newpilot.portrait_onduty = string.gsub(string.gsub(newpilot.vncharacter, ".webp", "n.webp"), "_nogogn", "n_nogog")
        end
    end

    return newpilot
end

local function createScavNpcs()
    local num_npcs = rnd.rnd(0, max_escorts - #mem.persons + 2 * max_scavengers)

    local cmdr_fudge = 0
    for i = 1, num_npcs do
        local newpilot = create_pilot(faction.get("Affiliated"))
        if newpilot.commander then
            cmdr_fudge = 2
        end
        --      print(fmt.f("created a pilot named {name}", newpilot ) )
        local id =
            evt.npcAdd(
            "approachScavenger",
            _("Pilot for Hire"),
            newpilot.portrait,
            fmt.f(
                _(
                    [[This pilot seems to be looking for work.

Ship: {dreamship}
Initial Deposit: {deposit_text}
Cover fee: {royalty_percent:.1f}% of deposit]]
                ),
                newpilot
            ),
            9 - cmdr_fudge
        )
        npcs[id] = newpilot
    end
end

function create()
    if mem.persons == nil then
        mem.persons = {}
        mem.pilots = {}
    end

    mem.lastsys = system.cur()
    hook.land("land")
    hook.load("land")
    hook.enter("enter")
    hook.jumpout("jumpout")
end

function approachScavenger(npc_id)
    local pdata = npcs[npc_id]
    if pdata == nil then
        evt.npcRm(npc_id)
        npcs[npc_id] = nil
        return
    end

    local rand_quotes = {
        _("I've even heard someone quote Sun Tzu while talking about you."),
        _("Someone said you started out in a Llama, that's unbelievable!"),
        _("Come to think of it, I've heard so many stories that they couldn't possibly all be true!"),
        _("You're practically a legend mate, at least compared to someone like me."),
        _(
            "I heard you boarded and robbed an armed transport so fast that it was still drifting by the time you detached. Is that true?"
        ),
        _("I heard you survived a Caesar barrage from twelve Ancestors at once."),
        _("I heard you survived a Caesar barrage from seven Ancestors at once."),
        _("Apparently you go in guns blazing, too. I respect that."),
        _("I really hope you'll give me this chance."),
        _("Meeting you here is the opportunity of a lifetime."),
        _("If you give me this opportunity, I promise you will not regret it."),
        _("Somehow you are always on contentious ground, that's what I want."),
        _("I'm looking for action, and it seems to follow you around.")
    }
    local part1 = {
        _(
            "I'll stick with you, mostly, and I'll do my part in disabling and boarding our enemies before we kiss them goodnight, permanently."
        ),
        _("I'll try and learn from you and how to serve the common goals of the fleet."),
        _("Just think of all the time you'll save not having to board every single disabled ship!")
    }
    local part2 = {
        _("I might not always be in the best ship, but know that I'm doing my best with what I've got."),
        _("If you don't like my ship, you can always help me out with some upgrades."),
        _("I'll do the best I can with what I've got."),
        _("Let's board some enemies together and split the profits.")
    }

   local companions = {
      _("faithful companion"),
      _("fearsome apprentice"),
      _("generous henchman"),
      _("fearless sidekick"),
      _("feared escort"),
      _("dangerous company"),
      _("gruesome warrior"),
   }

   -- IF YOU WANT A QUIET ESCORT, he has to say "Well, I am eager to fly..."
   -- on the first conversation part, you're welcome :-)
   local random_quote1 = "Well,"   -- said by the strong silent type
   local random_quote2 = "Anyway," -- said by the strong silent type and regular ones

   if pdata.chatter > 0.33 then
      random_quote1 = rand_quotes[rnd.rnd(1, #rand_quotes)] -- said by regular ones and chatterboxes
      if pdata.chatter >= 0.67 then
         random_quote2 = rand_quotes[rnd.rnd(1, #rand_quotes)] -- said only by chatterboxes
      end
   end

   local has_commander = nil
   local anim_msg
   for _i, other in ipairs(mem.persons) do
      if other.commander then
         has_commander = other
         anim_msg = fmt.f(
            _(
               [[I can't fly with {name}. We used to work together at Janus Station. All the talking about dreams of flying a brand new {dreamship}, when clearly a beat-up old {ship} with gigantic bite marks all over it would be a much more appropriate career choice. Maybe someone else will tolerate that insufferable chatterbox.]]
            ),
            has_commander
         )
      end
   end

   local hired = false
   local params = {
                    credits = fmt.credits(pdata.deposit),
                    name = player.name(),
               myname = pdata.first_name .. " " .. pdata.name,
                    ship = player.ship(),
                    shipname = player.pilot():ship():name(),
                    random_quote = random_quote1,
               random_quote2 = random_quote2,
                    part1 = part1[rnd.rnd(1, #part1)],
                    part2 = part2[rnd.rnd(1, #part2)],
                    dreamship = pdata.dreamship,
               companion = companions[rnd.rnd(1, #companions)]
                }
   vn.clear()
   vn.scene()


   local stranger_choices = {
      "neutral/bartender_m1.webp",
      "neutral/bartender_f1.webp",
      "neutral/female4.webp",
      "old_man.png",
      "scavenger1.png",
      "shiplover.webp",
   }
   vn.transition()

   local escort = vn.newCharacter ( pdata.name, {image=pdata.vncharacter } )
   vn.transition()
   escort(
            fmt.f(
                _(
                    [[Greetings Commodore {name} of the {shipname}. My name is {myname} -- I've heard so much about you and your {ship} and your skill in battle. {random_quote} I am eager to fly alongside you, but my dream is to pilot a {dreamship} one day. Help me reach this goal and I will be your most {companion}.]]
                ),
                params
            )
   )

   if pdata.commander and not has_commander then
      escort(
         _([[You might have noticed that working with seasoned drifters such as myself isn't always exactly easy. A lot of the folk in this trade lack discipline; they are washed up military personnel, retired pirates or aspiring young fools... I used to be a commander myself, and should have no problems managing a small squad on your behalf, up to five destroyers in size.]])
      )
   end

   escort(
            fmt.f(
                _(
                    [[The arrangement is simple: you pay an initial deposit and cover a percentage of my replacements and maintenance fees. {part1} {part2}
{random_quote2} I think I'll learn a lot with you, so what do you say...

Will you pay {credits} to kickstart my career as your apprentice?]]
                ),
                params
            )
   )

   vn.menu( {
      { _("You're hired!"), "yes" },
      { _("No thanks"), "end" },
   })

   vn.label("yes")
   vn.func( function ()
       if pdata.deposit and pdata.deposit > player.credits() then
         vn.jump("no_money")
      end
      if has_commander and pdata.commander then
         vn.jump("animosity")
      end
      if pdata.commander and #mem.persons >= max_escorts then
         vn.jump("toomany") -- wait, how could this happen? leaving the branch here just in case...
      elseif not pdata.commander and( #mem.persons >= max_escorts or #mem.persons >= max_scavengers and not has_commander) then
         vn.jump("toomany")
      end
   end )
   vn.func( function()
      hired = true
   end )
   escort(_([[You won't regret it!]])) -- TODO: Generate this
   vn.jump("end")
   vn.label("no_money")
   escort(_([[I can't fly on dreams and steam. Come back when you have enough money.]]))
   vn.jump("end")
   vn.label("animosity")
   vn.na(fmt.f(_([[Before you are able to present payment to {name}, your actions are interrupted by an unexpectedly forceful impact on your left shoulder.
   You look up...]]), pdata))
   escort(anim_msg)
   vn.jump("end")
   vn.label("toomany")
   escort(
      _(
         [[You look like you've got enough problems on your hands already. I think I'll stay out of your way for now.]]
      )
   )
   if not has_commander then
      vn.disappear(escort)
      vn.na(fmt.f(_([[You realize that not even the vagabonds themselves think you can handle more than {number} of them under your wing. If only you had one of those commanding types that was mentioned by the stranger...]]), { number=#mem.persons } ))
   end
   vn.jump("end")
   vn.label("end")
   -- show a helpful stranger the first time the player has spoken to a hired escort
   if not mem.escort_tutorial then
      vn.disappear(escort)
      vn.na(fmt.f(_([[As you finish your conversation with {name}, your attention is captured by a stranger who seems to have been waiting for the conversation to end.]]), pdata))
      local stranger = vn.newCharacter ( _("Eavesdropper"), { image = pick_one(stranger_choices) } )
      vn.appear(stranger)
      stranger(_([[Hey there. I couldn't help but notice you talking to one of those vagabonds. You should be wary of them.
While hiring these drifters as escorts does allow you to command a larger fleet than you normally would be able to, this comes at a rather hidden cost.
Most importantly, they need considerable micromanagement and you'd be wise to hire someone to command them on your behalf.]]))
      stranger(_([[You are unlikely to be able to command the necessary level of respect needed to control these pilots, especially with pilots with whom you have no prior relationship.
Another thing to consider is that they each have their own personality, some of them will talk a lot, and I mean really a lot, while others are relatively quiet.]]))
      stranger(_([[These vagabonds are trained combat personnel, but they aren't working. Do I need to tell you why? Well, the important thing is that they will attempt to eject from their ship to avoid death and may even run away from an active combat situation in some cases.
The good news is, you don't need to hire new pilots if you keep geting into scuffles.]]))
      stranger(_([[As your escort crew and your escort commander successfully board enemy ships and sell plundered commodities, they will earn credits and gain experience.
Experienced escorts generally perform better at their duties, but most importantly they are more likely to buy an appropriate ship on their own as their budget increases with experience.]]))
      stranger(_([[You can help your escorts finance new ships at any shipyard, otherwise they will purchase what they consider a suitable replacement when necessary.
If they lose a ship in battle, don't expect them to come back without some motivation, but a good commander at a shipyard will usually do the trick.]]))
      stranger(_([[One thing you might want to remember is that each one of these pilots is usually only certified to fly a single ship. These hustlers will be stingy about profit sharing and charge excessive replacement rates when they are in ships they don't like.
They will always like their certified ship, but I'm sure that they will start liking whatever you give them as long as you can keep their wallets happy.]]))
--      stranger(_([[Now, if you have any questions, please open an issue on github or comment on the pull request that contains this event so that it can be made more clear in the future.]]))
      vn.na(_([[The zealous eavesdropper pats you on the elbow and leaves you to digest everything that has been said.

      You ponder if anything important was said as your focus is shifted back to your immediate surroundings.]]))

      -- don't show this annoying stranger again
      vn.func( function()
         mem.escort_tutorial = true
      end)
      vn.done()
   end
   vn.done()
   vn.run()

    if not hired then
        return -- player rejected offer or was unable to hire
    end

    local title_choices = {_("Of course"), _("Splendid"), _("Surprise"), _("Hardly unexpected")}

    vntk.msg(
        pick_one(title_choices),
        fmt.f(
            _(
                [[You finish up paying the {vagabond}, who instantly {scurries} towards the shipyard.
As you conduct your affairs on the spaceport, you notice your new {sidekick} in what looks like a {ship} {action}.]]
            ),
            {
                ship = fmt.f("{ship}", pdata),
            vagabond = pick_one({
               _("vagabond"),
               _("drifter"),
               _("pilot"),
               _("eager pilot"),
               _("lonesome drifter"),
               _("rugged vagabond"),
               _("lonesome mercenary"),
            }),
            sidekick = pick_one({
               _("friend"),
               _("understudy"),
               _("buddy"),
               _("sidekick"),
               _("follower"),
               _("henchman"),
               _("assistant"),
               _("troublemaker"),
               _("and eager disciple"),
               _("drifter friend"),
               _("and fearsome vagabond"),
               _("and lonesome mercenary"),
            }),
            scurries = pick_one({
               _("scurries"),
               _("hustles"),
               _("bustles"),
               _("runs"),
               _("skips"),
               _("jogs"),
               _("hops"),
               _("gets up to pay for some drink and heads"),
               _("heads"),
               _([[gets up to leave before turning around, grabbing you by the hand, exclaiming "You're not going to regret this!". The surprising charmer then smoothly exits, probably]]),
               _("inspects the credit chip meticulously before hurrying"),
            }),
            action = pick_one({
               _("shuffling through the pages of what looks like a manual"),
               _("performing some peculiar manoeuvers"),
               _("come pretty close to a disasterous takeoff while performing a safety check"),
               _("struggling with the controls and visibly cursing"),
               _("with bite marks all over it"),
               _("with such discoloration that you can only assume it has been partly repaired several times"),
               _("with bunny ears attached to the top of the cockpit"),
               -- TODO for these: use the nice generators from companions
               fmt.f(_("but it kind of looks like a {thing}"), { thing = _("cargo container") }),
               fmt.f(_("with the letters {thing} written in crimson red on the hull"), { thing = _("Darker") }),
               fmt.f(_("with {thing} written in safety orange on the hull"), { thing = _("cargo container") }),
               fmt.f(_("with {thing} master written in bright yellow on the hull above a rather sizeable decal of a {thing}"), { thing = _("banana") }),
               _("that has seen its fair share of violence over the years"),
               _("doesn't look like it has very long left, but you can only hope for the best"),
               _("has seen better days"),
            }),
            }
        )
    )

    if pdata.deposit then
        player.pay(-pdata.deposit, true)
    end

    local i = #mem.persons + 1

    pdata.alive = true
   pdata.active = true
    mem.persons[i] = pdata
    evt.npcRm(npc_id)
    npcs[npc_id] = nil
    local id = evt.npcAdd("approachHiredScav", pdata.first_name .. " " .. pdata.name, pdata.portrait, _("This is a new pilot under your wing."), 8)
    npcs[id] = pdata
    evt.save(true)

    shiplog.create(logidstr, _("Hired Escorts"), _("Hired Escorts"))
    shiplog.append(
        logidstr,
        fmt.f(
            _(
                "You hired a {dreamship} pilot named '{name}' that was actually flying a {ship} for {deposit_text} at a {royalty_percent:.1f}% initial cover charge."
            ),
            pdata
        )
    )
end

-- player boards an escort, repair the armor and undisable it
-- give it 25% shields regardless of previous shield, maximum restored at this point in time "because of repairs"
function player_rescue(target, _playerpilot, i)
   target:setHealth(100, 25, 99)
   mem.persons[i].last_sentiment = "closecall"

   -- TODO: close the boarding dialog
end

function scav_boarding(plt, target, i)
    local choices = {
        "Thanks, sucker!",
        "I got this.",
        "I've got this one.",
        "I'll get it.",
        "Another one bites the dust.",
        "Payday, baby!",
        "Got it.",
        "Alright, let's beat it.",
        "I found {amount} in here!",
        "Don't worry about this, {player}.",
        "Maybe I'll find something for {flagship} here...",
        "This goes into the {dreamship} fund.",
        "{amount}, huh, how about that.",
        "Oh, {amount}?",
        "{amount}, for me?",
        "{flagship} sends warm regards.",
        "The {flagshipt} sends its regards. Right {player}?",
        "Come on, let's do this quick.",
        "Damn it feels good to be alive.",
        "Let's blow this joint.",
        "Uhhh...",
        "Well...",
        "Let's see...",
        "Make {player} happy...",
        "Come on.",
        "Knock knock, I have a message from {player} of {flagship}.",
        "You see that {flagshipt} over there? Fork over the credits or it's good night, for good!",
        "Finish it before we attract attention.",
        "Destroy it before we attract more attention.",
        "Now blow it up.",
        "Come on {player}.",
        "Boom."
    }
    local payout_choices = {
        "Your share is {your_share}.",
        "I've sent you {your_share}.",
        "You only get {your_share}.",
        "I'm keeping {my_share}, you can have the rest.",
        "You've got your share.",
        "You can have {your_share} of it.",
        "I'll hold on to {my_share}, that leaves... {your_share} for {player}.",
        "After my cut of {my_share}, that leaves... {your_share} for {flagship}.",
        "After my cut... {your_share} for the {flagshipt}.",
        "If I didn't have to give {your_share} to the {flagship} I'd be rich!",
        "Your cut is {your_share}.",
        "You get {your_share}.",
      "You get {your_share} from the {target}.",
        "You can have {your_share}.",
        "{your_share} for {flagship}.",
        "The {flagshipt} gets {your_share}.",
        "Here's your cut.",
      "Here's your cut from the {target}.",
        "That comes down to {your_share} for {player}, {my_share} for me.",
        "I'm keeping {my_share}, that sound about right?",
        "I hope you're happy with {your_share}.",
      "I hope you're happy with the {your_share} from that {target}.",
        "I hope you're happy with your share of {your_share}."
    }
    local notbad_choices = {
        _("Not bad."),
        _("Not too bad, eh?"),
        _("At least it's something."),
        _("Pretty average."),
        _("I'm happy. You happy?"),
        _("You happy? I'm happy."),
        _("Could be worse, right?"),
        _("That's a decent haul."),
        _("It sure is payday."),
        _("Not bad, huh?"),
        _("Seems fair to me."),
        _("I learned something from this."),
        _("We can be satisfied."),
        _("It'll do."),
        _("Could be better."),
        _("It's alright."),
      _("Not so bad for a {target}."),
    }
    local cargo_options = {
        _("{cargo}..."),
        _("What is this, {cargo}?"),
        _("Oh great, now we're going to have to sell this {cargo}."),
        _("It's just {cargo}."),
        _("At least it's {cargo}."),
        _("Where can we sell {cargo}?"),
        _("Where can we get rid of {cargo}?"),
        _("How will we get rid of {cargo}?"),
        _("Who's going to buy {cargo} out here?"),
        _("I guess {cargo} is better than nothing."),
        _("{cargo} -- worthless unless we can find a buyer."),
        _("{cargo} again."),
        _("{cargo} again..."),
        _("Not {cargo} again!"),
        _("Oh, it's just commodities."),
        _("Oh, it's just some cargo."),
        _("It's just commodities."),
        _("Just some cargo."),
        _("Just some cargo. {cargo} and the likes."),
        _("It's random cargo."),
        _("It's random cargo such as {cargo}."),
        _("It had some commodities."),
        _("It had some commodities like {cargo}."),
      _("This {target} was full of cargo.")
    }
       local cargo_full = {
        _("I had to leave the {cargo}..."),
        _("What did I leave behind, {cargo}?"),
        _("Oh great, we had to leave behind all that {cargo}."),
        _("Left the {cargo} behind."),
        _("Had to leave some behind. It's only {cargo}."),
        _("Should I have taken the {cargo}?"),
        _("My hold was full, I left the {cargo}."),
        _("I didn't grab the {cargo} though"),
        _("Who's going to buy {cargo} out here anyway?"),
        _("{cargo} would have been better than nothing."),
        _("{cargo} -- didn't have space for that."),
        _("{cargo}, but my cargo hold is full."),
        _("{cargo} again... My hold is full anyway."),
        _("Not {cargo} again! Left that behind."),
        _("Oh, it's just commodities. I'll leave those."),
        _("Oh, it's just some cargo. I don't have room for that."),
        _("It's just commodities. Nevermind them."),
        _("Just some cargo. I didn't have the space."),
        _("I left some cargo. {cargo} and the likes."),
        _("There's random cargo but I'm all full."),
        _("It's random cargo such as {cargo} but I don't have space."),
        _("There are some commodities. Forget 'em."),
        _("There were some commodities like {cargo} that we left behind."),
      _("This {target} was full of cargo that we had to leave behind.")
    }
    local bounty = target:credits()
    local speak_chance = mem.persons[i].chatter or 0.5
    local exp_boost = math.min(8, math.floor(bounty / 10e3) * 0.1)

    -- try to loot cargo
    local cargofree = plt:cargoFree()
    if cargofree then
      local new_choices = {}
        local clist = target:cargoList()
        for _k, c in ipairs(clist) do
            local n = plt:cargoAdd(c.name, c.q)
         if n > 0 then
            exp_boost = exp_boost + math.floor(c.q / 10) * 0.01
            table.insert(payout_choices, fmt.f(pick_one(cargo_options), {cargo = c.name, target = target}))
         else
            table.insert(new_choices, fmt.f(pick_one(cargo_full), {cargo = c.name, target = target}))
            mem.persons[i].last_sentiment = "cargofull" -- also make sure to pester the player soon
         end
        end
      if #new_choices > 0 then
         speak_chance = 1 -- we definitely want to say something, we had to leave cargo behind!
         payout_choices = new_choices
      end
    end

    if bounty < 2e3 then
        payout_choices = {
            "Worthless.",
            "Better luck next time.",
            "That's it?",
            "Is that all?",
            "{player} won't be pleased.",
            "What a loser.",
            "Nothing in here.",
            "It's pretty empty.",
            "It's too damaged to salvage.",
            "{my_share} won't do me much good.",
            "Oh, nevermind.",
            "Nevermind...",
            "Oh wait, no... Darn it, nevermind.",
            "Oh come on, another one?",
            "Again?",
            "Let's find another target."
        }
        if exp_boost > 0 then
            table.insert(payout_choices, _("There's only cargo."))
            table.insert(payout_choices, _("It's just cargo."))
            table.insert(payout_choices, _("I got the cargo."))
            table.insert(payout_choices, _("At least I took the cargo."))
            table.insert(payout_choices, _("At least we got some cargo."))
         table.insert(payout_choices, _("At least there was cargo in this {target}."))
        end
        speak_chance = speak_chance + 0.1
    elseif bounty > 60e3 then
        table.insert(payout_choices, "Actually, that's not bad, you get {your_share}!")
        table.insert(payout_choices, "Woah! Well I'm keeping {my_share}.")
      table.insert(payout_choices, "We should definitely hunt more {target}s.")
      table.insert(payout_choices, "We should hunt more of these {target}s.")
      table.insert(payout_choices, "This {target} had {amount} in it!")
        -- we will want to speak about this unless we are really experienced
        local experience_fudge = mem.persons[i].experience / (2 * mem.persons[i].experience + 100)
        speak_chance = speak_chance + math.max(0, 0.5 - experience_fudge)
        exp_boost = exp_boost + 0.05
    elseif bounty > 35e3 then
        table.insert(payout_choices, "Not too shabby, check your logs.")
        table.insert(payout_choices, "I'm happy with {my_share}, here's {your_share}.")
      table.insert(payout_choices, "I've seen worse on a {target}.")
        speak_chance = speak_chance + 0.1
        exp_boost = exp_boost + 0.01
    end

    local choice = choices[rnd.rnd(1, #choices)] .. " " .. payout_choices[rnd.rnd(1, #payout_choices)]

   -- if we got a nice bounty and we're excited about it, let's talk about it
   -- don't change speech for this if we're worried about our cargo
    if bounty > 20e3 and mem.persons[i].last_sentiment ~= "cargofull" then
        if bounty < 50e3 then
            choice = choice .. " " .. notbad_choices[rnd.rnd(1, #notbad_choices)]
        end
        mem.persons[i].last_sentiment = "content"
        speak_chance = speak_chance + 0.3
    end

    mem.persons[i].experience = mem.persons[i].experience + exp_boost

    local commanders_greed = 0
    if mem.persons[i].commander then
        commanders_greed = mem.persons[i].experience * 24
    end
   local your_share = math.max(0, math.floor(bounty * mem.persons[i].royalty) - commanders_greed)
   local myship = mem.persons[i].ship:nameRaw()
   -- before we make up our mind, let's decide if we are happy with our current ship
   local favourite = pick_favorite_ship(mem.persons[i])
   if
         string.find(myship, favourite) or
         string.find(favourite, myship)
   then
      -- if this is a favorite ship, let's be more generous because
      -- we chose this ship ourselves even if it isn't the dream ship
      -- note that if we have many favorites, we might not pick what we're flying now
      your_share = math.floor(math.max(bounty / 4, (bounty  * (1 - mem.persons[i].royalty)) - 2e3))
      -- allow a small bounty to give us the chance to resist negativity at the next boarding
      if bounty > 5e3 and mem.persons[i].last_sentiment ~= "cargofull" then
         mem.persons[i].last_sentiment = "content"
      end
   end

   -- obviously, if our ship looks like our dream ship then we are very happy
    if string.find(myship, mem.persons[i].dreamship) then
        -- no greed in a dreamship, generosity instead
        your_share = math.min(bounty, math.floor(bounty * (1 - mem.persons[i].royalty) + commanders_greed))
        -- random special dream ship message
        if rnd.rnd() > 0.95 then
            choice = "{my_share} for {dreamship}, {your_share} for {flagship}."
            speak_chance = speak_chance + 0.3
        end
        mem.persons[i].last_sentiment = "dreamship"
    end



    if bounty > 0 then
        -- we decrease the persons wallet here, but it is increased when the pilot actually boards the victim (after this frame)
        mem.persons[i].wallet = mem.persons[i].wallet - your_share
        player.pay(your_share)
        mem.persons[i].tribute = mem.persons[i].tribute + your_share
        mem.persons[i].total_profit = mem.persons[i].total_profit + bounty
        shiplog.append(
            logidstr,
            fmt.f(
                _("'{name}' of {ship} paid you {credits} from a total bounty of {total} ({target})."),
                {
                    name = mem.persons[i].name,
                    ship = mem.persons[i].ship,
               target = target:ship(),
                    credits = fmt.credits(your_share),
                    total = fmt.credits(bounty)
                }
            )
        )
    else
        shiplog.append(
            logidstr,
            fmt.f(
                _("'{name}' gained {xp} experience boarding a creditless victim ({ship})."),
                {name = mem.persons[i].name, ship = mem.persons[i].ship, xp = exp_boost}
            )
        )
        if mem.persons[i].last_sentiment ~= "content" and mem.persons[i].last_sentiment ~= "cargofull" then
            mem.persons[i].last_sentiment = "negative"
        end
    end

    if rnd.rnd() < speak_chance then
        local info = {
            amount = fmt.credits(bounty),
            player = player.name(),
            flagship = player.ship(),
            dreamship = mem.persons[i].dreamship,
            flagshipt = player.pilot():ship():name(),
            your_share = fmt.credits(your_share),
            my_share = fmt.credits(bounty - your_share),
         target = target
        }
        plt:comm(fmt.f(choice, info))
    end

   -- if we are a commander or high ranked, ruthlessly destroy it
   -- with C4, because we probably have ion cannons
   if mem.persons[i].experience > 32 then
      -- plant extra explosives if we are skilled and have enough time
      for _i=1, 5 do
         local chance = math.min(100, mem.persons[i].experience) / 200
         if rnd.rnd() < chance then
            hook.timer(math.max(20, 135 - (chance * 200)), "detonate_c4", target)
         end
      end
      -- plant some explosives
      hook.timer(rnd.rnd(30, 35), "detonate_c4", target)
      hook.timer(rnd.rnd(30, 35), "detonate_c4", target)
      hook.timer(rnd.rnd(30, mem.persons[i].experience), "detonate_c4", target)
   end

   -- if we are a commander, plant deadly C4
   if mem.persons[i].commander then
      -- plant a lot of explosives in a specific pattern
      hook.timer(10, "detonate_c4", target)
      hook.timer(12, "detonate_c4", target)
      hook.timer(15, "detonate_c4", target)
      hook.timer(16, "detonate_c4", target)
      hook.timer(16, "detonate_c4", target)
      hook.timer(17, "detonate_c4", target)
      hook.timer(17, "detonate_c4", target)
      hook.timer(18, "detonate_c4", target)
   end

    --   target:broadcast(fmt.f("I was boarded by {name} and I had {credits}.", {name = mem.persons[i].name, credits=fmt.credits(bounty)}), true)
end

function detonate_c4( target )
   if target and target:exists() then
      local sound_choices = {
         "medexp1", "medexp0", "crash1", "grenade",
         "explosion0", "explosion1", "explosion2", "tesla"
      }
      local dir_vec = vec2.new(math.floor(rnd.threesigma() * 30), math.floor(rnd.twosigma() * 20))
      target:knockback(800, dir_vec, target:pos()-dir_vec)
      target:setDir(target:dir() + rnd.threesigma() * 0.07)
      local expl_pos = vec2.add(target:pos(), rnd.threesigma() * 2, rnd.twosigma() * 2)
      -- apply the damage (the player gets the credit)
      target:damage(300, 0, 100, "impact", player.pilot())
      -- visual and audio effects?
      audio.soundPlay(pick_one(sound_choices), expl_pos)
   end
end

function scavenger_arrives(arg)
   if var.peek( "hired_escorts_disabled" ) then return end
   if system.cur() == system.get("Crimson Gauntlet") then return end

    local i = arg.i
   if not mem.persons[i].active then
      return
   end
    local f = mem.persons[i].faction
    -- if we are already with the player, don't respawn
    if mem.persons[i].pilot ~= nil and mem.persons[i].pilot:exists() then -- pilot still exist
        print("pilot still exists:", mem.persons[i].name)
        return
    end
    local plt = pilot.add(mem.persons[i].ship, f, arg.spawnpoint, mem.persons[i].name, {naked = true})
    -- calculate what outfits go missing here
    local penalty = 0
    if mem.persons[i].debt > 1e6 then
        penalty = rnd.rnd(2, plt:ship():size())
    end
    for j, o in ipairs(mem.persons[i].outfits) do
        local r = rnd.rnd()
        if j > 4 and r < 0.06 * penalty then
            -- don't fit the thing!
            speak(mem.persons[i], "malfunction", {thing = o, speaker = plt})
            penalty = penalty - 3
        else
            plt:outfitAdd(o)
        end
    end
    plt:fillAmmo()
    plt:setFriendly()
    plt:credits(-plt:credits() + mem.persons[i].wallet)

    -- give it the escort outfit
    local tags = plt:ship():tags()
    if plt:ship():size() < 3 and tags["transport"] then
        plt:outfitAddIntrinsic("Integrated Shielding")
    end

    local temp = 250
    local armor = 100
    local shield = 100
    local stress = 0
    local energy = 100
    if mem.temp ~= nil then
        temp = mem.temp
    end
    if mem.armor ~= nil then
        armor = mem.armor
    end
    if mem.shield ~= nil then
        shield = mem.shield
    end
    if mem.stress ~= nil then
        -- Limit this to 99 so we don't have the weirdness of a
        -- disabled ship warping in.
        stress = math.min(mem.stress, 99)
    end
    if mem.energy ~= nil then
        energy = mem.energy
    end
    plt:setTemp(temp, true)
    plt:setHealth(armor, shield, stress)
    plt:setEnergy(energy)
    plt:setFuel(true)

    local aimem = plt:memory()
    aimem.atk_kill = false
    aimem.atk_board = true
    aimem.aggressive = false
    aimem.careful = true
   aimem.leadermaxdist = 500 * plt:ship():size()
   aimem.enemyclose = 300 * plt:ship():size()

    plt:setVisplayer(true)
    plt:setNoClear(true)
    mem.persons[i].alive = true

    hook.pilot(plt, "death", "scavenger_death", i)
    hook.pilot(plt, "hail", "scav_hail", i)
    hook.pilot(plt, "attacked", "scav_attacked", i)
    hook.pilot(plt, "boarding", "scav_boarding", i)
   hook.pilot(plt, "board", "player_rescue", i)

    --   plt:broadcast(fmt.f("I'm number {i}, had to pay off {credits}.", { i=i , credits=fmt.credits(arg.debt / 2) } ))

    if mem.persons[i].spawning then
        hook.rm(mem.persons[i].spawning)
    end
    mem.persons[i].pilot = plt
    mem.persons[i].spawning = false

    -- carry over cargo from earlier
    if mem.persons[i].cargo ~= nil then
        for _i, v in pairs(mem.persons[i].cargo) do
            plt:cargoAdd(v.name, v.q)
        end
    end

   -- vocal leaders will seem to perform slightly better
   local lead_chance = math.min(0.4, math.floor(mem.persons[i].experience + mem.persons[i].chatter) * 0.01)

    -- usually, we set the leader, but sometimes, don't
    if mem.persons[i].commander then
      aimem.whiteknight = true
      -- if we are a good leader, set up the existing support fleet, it becomes a lazy routine
      if rnd.rnd() < lead_chance * 3 then
         hook.timer(20, "setup_support_fleet", mem.persons[i].pilot)
         hook.timer(60 * mem.persons[i].chatter, "setup_support_fleet", mem.persons[i].pilot)
         hook.timer(60, "setup_support_fleet", mem.persons[i].pilot)
         hook.timer(mem.persons[i].experience, "setup_support_fleet", mem.persons[i].pilot)
      end
      -- I'll catch up with you later
      if rnd.rnd() < lead_chance then
         speak(mem.persons[i], "brb")

         plt:changeAI( "escort_guardian" )

         aimem = plt:memory()
         aimem.stealth = true
         aimem.atk_board = true
--         aimem.atk_kill = true -- this might be bad
         aimem.aggressive = true
         aimem.formation = "wedge"
         plt:memory().atk_board = true -- do we need this??
         -- if I'm in a dream ship (or close), enlist help
         if string.find(mem.persons[i].ship:nameRaw(), mem.persons[i].dreamship) then
            for j, pers in ipairs(mem.persons) do
               if i ~= j and pers.pilot ~= nil then
                  if pers.pilot:exists() then
                     pers.pilot:setLeader(plt)
                     speak(mem.persons[j], "affirm")
                     local mymem = pers.pilot:memory()
                     mymem.leadermaxdist = 225 * pers.pilot:ship():size()
                     mymem.aggressive = true
   --                        mymem.stealth = aimem.stealth
                  end
               end
            end
         end
         -- since we are actively guarding the player, let's make sure we pick up player attacked
         if mem.persons[i].commander == true then mem.persons[i].commander = {} end
         if mem.persons[i].commander.hook then
            hook.rm(mem.persons[i].commander.hook)
         end
         mem.persons[i].commander.hook = hook.pilot(player.pilot(), "attacked", "player_attacked", mem.persons[i].pilot)

         return -- sometimes don't means must not continue down default path
      end
    end
   plt:setLeader(player.pilot())
   -- always speak less and less with experience
   local speak_chance = mem.persons[i].chatter - (math.min(300, mem.persons[i].experience) / 1000)
   if rnd.rnd() > speak_chance then -- if I'm a chatterbox
      if rnd.rnd() > 0.5 then -- be positive
         speak(mem.persons[i], "idle")
      else -- be negative
         speak(mem.persons[i], "negative")
      end
   else
      -- I didn't say anything, let's fall in like a good soldier
      hook.timer(rnd.rnd(4, 32), "fall_in", mem.persons[i])
   end


end

function player_attacked ( player_pilot, attacker, dmg, leader )
   local armor, shield, stress = player_pilot:health()
   if leader and (shield < 80) and leader:exists() then
      local subordinates = leader:followers()
      if #subordinates > 0 then
         leader:msg(pick_one(subordinates), "l_attacked", attacker)
         -- make sure to respond more when appropriate
         if dmg > 100 or armor < 45 or stress > 50 then
            leader:msg(pick_one(subordinates), "e_attack", attacker)
         end
      end
   end
end

local function scav_hook_spawner(plt, i, spawnpoint)
    local arg = {i = i, spawnpoint = spawnpoint, debt = plt.debt}
    local delay = 6 + math.floor(math.max(rnd.rnd(1, 6), plt.debt / 10e4)) * plt.ship:size()

    arg.spawnpoint = spawnpoint or pilot.choosePoint(faction.get("Escort"), true)

    if plt.spawning then
        --         player.pilot():comm(fmt.f("Can't spawn {person} because {spawning} {alive}", { person = v.name, spawning=v.spawning, alive=v.alive} ))
        -- delete the old hook first
        hook.rm(plt.spawning)
    end
    plt.debt = plt.debt - plt.debt / 16 -- decrease the debt
    --      player.pilot():broadcast(fmt.f("{name} arrives in approximately {time} s", {name=plt.name, time=delay}))
    plt.spawning = hook.timer(delay, "scavenger_arrives", arg)
end

function enter()
    local spawnpoint
    if mem.lastsys == system.cur() then
        spawnpoint = mem.lastplanet
    else
        spawnpoint = player.pos()
        for i, sys in ipairs(mem.lastsys:adjacentSystems()) do
            if sys == system.cur() then
                spawnpoint = mem.lastsys
            end
        end
    end

    --   mem.lastsys = system.cur()

    for i, v in ipairs(mem.persons) do
        scav_hook_spawner(mem.persons[i], i, spawnpoint)
        --scavenger_arrives( arg )
    end
end

local function update_wallet(i)
    if mem.persons[i].pilot == nil then
        print("update wallet nil pilot")
        return
    end
    if not mem.persons[i].pilot:exists() then
        print("update wallet pilot not exists")
        return
    end
--    local difference = mem.persons[i].pilot:credits() - mem.persons[i].wallet
    --   mem.persons[i].pilot:broadcast(fmt.f("I currently have {credits}", { credits = mem.persons[i].wallet }))
    --   mem.persons[i].total_profit = mem.persons[i].total_profit + difference
    mem.persons[i].wallet = mem.persons[i].pilot:credits() -- update wallet
    --   mem.persons[i].pilot:broadcast(fmt.f("I updated my wallet to {credits}", { credits = mem.persons[i].wallet }))

    -- disable the comment to prevent double counting
    -- mem.persons[i].wallet = mem.persons[i].pilot:credits() -- update wallet
end

function jumpout()
    mem.lastsys = system.cur()
    for i, persona in ipairs(mem.persons) do
      if persona.commander and persona.commander.hook then
         hook.rm(persona.commander.hook)
         persona.commander.hook = nil
      end
        if persona.pilot ~= nil then
            if persona.pilot:exists() then
                local pmem = persona.pilot:memory()
                local boarded = pmem.boarded or 0
                --         persona.pilot:broadcast(fmt.f("I have boarded {boarded} times and made {profit} total.", { boarded=boarded, profit=persona.total_profit } ))
                if boarded > 0 then
                    persona.experience = persona.experience + 1
                    remember_ship(persona, persona.ship:nameRaw(), boarded)
                end
                update_wallet(i)

                mem.persons[i].temp = mem.persons[i].pilot:temp()
                mem.persons[i].armor, mem.persons[i].shield, mem.persons[i].stress = mem.persons[i].pilot:health()
                mem.persons[i].energy = mem.persons[i].pilot:energy()
                mem.persons[i].cargo = persona.pilot:cargoList()
            end
            --         mem.persons[i].pilot:rm()
            --         mem.persons[i].pilot = nil
         if mem.persons[i].spawning then
            hook.rm(mem.persons[i].spawning)
         end
            mem.persons[i].spawning = false
        --      else
        --         print(fmt.f("person {i} is nil pilot", {i=i}))
        end
    end
end

function land()
   --[[
   -- hack to fix portraits (should be safe to delete)
   for _i, pers in ipairs(mem.persons) do
      if not pers.portrait_onduty then pers.portrait_onduty = pilot_portrait(pers.commander) end
      pers.portrait = pers.portrait_onduty
      if not pers.active then pers.portrait = pers.portrait_offduty end
   end
   --]]
    mem.lastplanet = spob.cur()
   mem.lastsys = system.cur()
   local commander_exists = false
   local commander_active = false
    for i, persona in ipairs(mem.persons) do
      if persona.commander then
         commander_exists = true
         if persona.active then
            commander_active = true
         end
         if persona.commander.hook then
            hook.rm(persona.commander.hook)
            persona.commander.hook = nil
         end
      end
        if persona.alive and persona.pilot ~= nil and persona.pilot:exists() then
            update_wallet(i)
            local pmem = persona.pilot:memory()
            local boarded = pmem.boarded or 0
            --         persona.pilot:broadcast(fmt.f("I have boarded {boarded} times and made {profit} total.", { boarded=boarded, profit=persona.total_profit } ))
            if boarded > 0 then
                persona.experience = persona.experience + 1
                remember_ship(persona, persona.ship:nameRaw(), boarded)
                -- we are now eligible for promotion
                persona.rank = rank_name(persona.experience)
                if persona.commander and persona.experience >= 1000 then
                    persona.rank = "Lieutenant Commander"
                end
            end
            -- sell any cargo
            local price
            local profits = 0
         local available_comms = spob.cur():commoditiesSold()
            for _i, cc in ipairs(persona.pilot:cargoList()) do
                -- does this station even buy this cargo??
            -- ugly extra loop to squash warnings... could be optimized to 2N instead of N^2 but this is tiny and we are landing so I'm lazy
            for _j, dd in ipairs(available_comms) do
               if cc.name == dd:name() then
                  price = commodity.get(cc.name):priceAt(spob.cur())
                  if price > 0 then -- sell the commodity
                     local gain = math.floor(cc.q * price)
                     profits = profits + gain
                     persona.pilot:cargoRm(cc.name, cc.q)
--                     print( fmt.f("sold {q} units of {name}", cc) )
                  end
               end
            end
            end
         persona.cargo = persona.pilot:cargoList()
            -- share some profit with the player
            if profits > 0 then
                local your_share = math.floor(profits * persona.royalty)
            local currents = persona.ship:nameRaw()
            -- if we like our current ship, give the player more
                if string.find(currents, persona.dreamship) or string.find(currents, pick_favorite_ship(persona)) then
               -- if we got a ship that is a favorite but we got it for another reason, then hopefully the royalty is low
                   your_share = math.floor(profits * (1 - persona.royalty))
                else
                if profits > 10e3 then
                        persona.last_sentiment = "goodhaul"
               elseif persona.last_sentiment == "cargofull" then
                  -- ah, well it's always nice to have space after having stuffed it full
                  persona.last_sentiment = "content"
                    end
                end

                local my_share = profits - your_share

                persona.wallet = persona.wallet + my_share
                persona.total_profit = persona.total_profit + profits
                persona.tribute = persona.tribute + your_share
                persona.pilot:credits(my_share)
                player.pay(your_share)
                shiplog.append(
                    logidstr,
                    fmt.f(
                        _("'{name}' paid you {credits} from sold commodities worth {total} ({ship})."),
                        {
                            name = persona.name,
                            ship = persona.ship,
                            credits = fmt.credits(your_share),
                            total = fmt.credits(profits)
                        }
                    )
                )
            end
        end
    end
    local pnt = spob.cur()
   local services = pnt:services()
    -- Clean up dead escorts so it doesn't build up, and create NPCs for
    -- existing escorts.
    npcs = {}
    local new_escorts = {}
    for i, edata in ipairs(mem.persons) do
      -- if we are inactive because we died, allow the commander to resurrect us if there's a shipyard here
      if commander_active and services.shipyard then
         edata.active = true
      end
        if edata.spawning then
            hook.rm(edata.spawning)
            edata.spawning = false
        end
        if edata.alive then
            local j = #new_escorts + 1
            edata.pilot = nil
            edata.temp = nil
            edata.armor = nil
            edata.shield = nil
            edata.stress = nil
            edata.energy = nil
            local this_desc = _("This is one of the {dreamship} pilots currently under your wing.")
         local prio = 8
            if edata.commander then
                this_desc = _("This is the most capable pilot currently under your wing and is certified to fly a {dreamship}.")
            prio = 7
            end
         if edata.ship:nameRaw() ~= edata.dreamship then
            this_desc = this_desc .. _("\nThis pilot was last seen flying a {ship} at a cover fee of {replacement_text}, which will be added to the pilot's personal account of ") .. fmt.f("{credits} towards a replacement.", { credits = fmt.credits(edata.wallet) } )
         end

         this_desc = fmt.f(this_desc, edata)
         -- only the commander goes to the bar, unless if there is no commander (player might need to manage escorts)
         -- also give the unimportant escorts a random chance of appearing even if they are off duty, it's a bar after all
         if (edata.active or not commander_exists) or edata.commander or rnd.rnd(1, 6) >= 5 then
            local id = evt.npcAdd("approachHiredScav", edata.rank .. " " .. edata.name, edata.portrait, this_desc, prio)
            npcs[id] = edata
         end
            new_escorts[j] = edata
        end
    end
    mem.persons = new_escorts
    if #mem.persons <= 0 then
        for i, v in ipairs(npcs) do
            npcs[i] = nil
        end
        evt.save(false)
    end

    -- Ignore on uninhabited and planets without bars


    local flags = pnt:flags()
    if not services.inhabited or not services.bar or flags.nomissionspawn then
        return
    end

    -- Create NPCs for pilots you can hire.
    createScavNpcs()
end

local function pilot_disbanded(edata, i)
    edata.alive = false
    local p = edata.pilot
    if mem.persons[i] == edata then
        mem.persons[i] = nil
    end
    if p and p:exists() then
        p:setLeader(nil)
        p:setVisplayer(false)
        p:setNoClear(false)
        p:setFriendly(false)
        p:hookClear()
    end
end

function scav_attacked(p, attacker, _dmg, i)
    local leave_choices = {
        _("This is getting too hot for me, I'll catch up with you later."),
        _("It's getting a little hot."),
        _("I'll catch up with you later."),
        _("Can we not do this right now?"),
        _("The {flagship} can take care of this."),
        _("Hey {name}, give us a hand will you?"),
        _("I might lose it here."),
        _("Oh shoot."),
      _("I need some help here."),
      _("I need help here!"),
      _("I need some help here..."),
      _("I could use a hand."),
      _("We could use a hand over here."),
      _("We aren't winning this."),
      _("We are being defeated."),
      _("We are being destroyed!"),
      _("We should back off."),
      _("Backing off."),
      _("Oh no..."),
      _("Let's get out of here."),
      _("Let's get outta here!"),
      _("Let's vamoose!"),
      _("Let's scram!"),
      _("Let's buzz off already!"),
    }
    local help_choices = {
        _("Help me, {name}!"),
        _("Where's the {flagship} when you need it?"),
        _("Mayday, we are getting pulverized over here!"),
      _("I'm with you {name}, but I need your help here!"),
      _("We need you {name}!"),
      _("Now is the time to use the {flagship}!"),
      _("Show us what the {flagship} can do!"),
      _("Aren't there any weapons on that {flagship}?")
    }

   local armor, shield, stress = p:health()

    -- check if we should change states
    if rnd.rnd() > 0.98 and shield < 30 then
        if p:leader() then
         if mem.persons[i].experience < 7 or rnd.rnd() < 0.01 then
            p:setLeader(nil)
         end
         p:changeAI("pirate")
         local aimem = p:memory()
         aimem.atk_board = true
         aimem.leadermaxdist = 500 * p:ship():size()
         p:pushtask("runaway", attacker)
            p:broadcast(
                fmt.f(
                    leave_choices[rnd.rnd(1, #leave_choices)],
                    {name = player.name(), flagship = player.pilot():ship():name()}
                )
            )
            mem.persons[i].last_sentiment = "beating"
         -- really low armor
         if armor < 20 then
            mem.persons[i].last_sentiment = "closecall"
         end
        else
            p:setLeader(player.pilot())
            p:broadcast(
                fmt.f(
                    help_choices[rnd.rnd(1, #help_choices)],
                    {name = player.name(), flagship = player.pilot():ship():name()}
                )
            )
         p:memory().leadermaxdist = 500 * p:ship():size()
        end
    elseif shield > 60 and armor > 98 and stress < 20 and mem.persons[i].commander or mem.persons[i].experience > 20 and rnd.rnd() < 0.06 then
      -- just maybe, let's start guarding the player more actively
--      print("changing to escort guardian:", mem.persons[i].name)
      p:changeAI("escort_guardian")
      local aimem = p:memory()
      aimem.atk_board = true
      aimem.leadermaxdist = 500 * p:ship():size()
   end
end

-- setup the support fleet properly so it doesn't run
-- currently just regroups
function setup_support_fleet( leader )
   if not leader or not leader:exists()then
      return
   end
   for _i, pers in ipairs(mem.persons) do
      if pers.pilot ~= nil then
         if pers.pilot:exists() then
            pers.pilot:pushtask("hold")
            leader:msg(pers.pilot, "e_hold")
            player.pilot():msg(pers.pilot, "e_hold")
         end
      end
   end
end

-- Escort got killed
function scavenger_death(p, _attacker, i)
    local edata = mem.persons[i]
   if string.find(edata.ship:nameRaw(), edata.dreamship) then
      edata.last_sentiment = "broken_dreams"
   end

   -- special commander stuff
   if edata.commander then
      if edata.commander.hook then
         hook.rm(edata.commander.hook)
         edata.commander.hook = nil
      end
      -- send the followers to the player
--      if false then -- let's test without first
      for _i, under in edata.pilot:followers() do
         under:setLeader(player.pilot())
--      end
      end
   end

    edata.cargo = nil
    -- check what's going on and if we are dead yet
    if edata.pilot == nil then
        shiplog.append(logidstr, fmt.f(_("'{name}' ({ship}) was lost unexpectedly."), edata))
        return
    end
    update_wallet(i)
    shiplog.append(logidstr, fmt.f(_("'{name}' ({ship}) was lost in combat."), edata))
    -- make the player pay for a replacement
    if edata.deposit then
        if edata.deposit > player.credits() then
            p:broadcast("Wait a minute, why isn't it ejecting? Oh s---", true)
            pilot_disbanded(edata, i)
        else
            -- pay here
            local replacement_fee = math.floor(edata.deposit * edata.royalty)
            player.pay(-replacement_fee, true)
            edata.total_cost = edata.total_cost + replacement_fee
            edata.replacement_text = fmt.credits(replacement_fee)
            shiplog.append(logidstr, fmt.f(_("'{name}' ({ship}) was replaced for {replacement_text}."), edata))
            edata.wallet = edata.wallet + replacement_fee
            p:broadcast(fmt.f(_("You paid {replacement_text} to cover the loss of my {ship}."), edata), true)
            -- assign him a new ship
            createReplacementShip(i)
            -- spawn a new one soon NOTE: Don't actually keep making new ships until the player jumps or lands.
--          scav_hook_spawner(mem.persons[i], i)
         -- we lost our ship, don't come back until the cap'n calls us back
         edata.active = false
        end
    else
        p:comm(player:pilot(), "Wait a minute, why isn't it ejecting? Oh sh--", true)
        pilot_disbanded(edata, i)
    end
end

local function fireEscort(edata, npc_id)
   for k, v in ipairs(mem.persons) do
      if edata.name == v.name then
         mem.persons[k] = mem.persons[#mem.persons]
         mem.persons[#mem.persons] = nil
      end
   end
   if npc_id and npcs[npc_id] == edata then
      evt.npcRm(npc_id)
      npcs[npc_id] = nil
   end
   shiplog.append(logidstr, fmt.f(_("You abandoned '{name}' ({ship})."), edata))
end

-- calculates how much the player has to pay for an upgrade on this character if the thing to buy costs newprice
local function calculateUpgrade( character, newprice )
   -- current balance
   local balance = character.ship:price() + character.wallet - character.debt

   -- how much the player would normally have to cover
   local remaining = balance - newprice
   local upgrade_amount = math.min(newprice / 2, 1475e3) -- minimum cost is half price or 375, whichever is less
   if remaining < 0 then
      return upgrade_amount
   end

   -- this scaling formula makes the player pay more if the escort is poor
   -- so if you're missing 1 out of 5 million you pay 200k (or the minimum), but if you're missing 2 out of 5 you pay 800k
   -- this is to incentivize paying for the cheaper ship if you can't really afford an upgrade
   local rounding_factor = 5e3   -- get nice even numbers

   -- otherwise it will cost as much as is proportionately missing from the balance to cover
   return math.floor(math.min( newprice,
      (math.abs(remaining * (remaining / newprice)) / rounding_factor
   ) * rounding_factor))
end

-- pay for an upgrade for the escort
-- shipchoice should be a { ship: <string>, credits: <full_price> } and can have a royalty field
local function escort_buyUpgrade( persona, shipchoice)
   -- calculate the money
   local the_price = calculateUpgrade( persona, shipchoice.price )

   -- take the money
   player.pay(-the_price)
   -- reset escort debt and reduce funds
   persona.debt = 0
   persona.wallet = persona.wallet / 2
   -- calculate a standard royalty on the ship if none provided
   shipchoice.royalty = shipchoice.royalty or math.min(0.95, 0.19 + 0.07 * shipchoice.actual:size())

   -- get the ship and create a fitting
   persona.ship = ship.get(shipchoice.ship)
   local _n, deposit = persona.ship:price()
   persona.outfits = {}
   local used_faction = persona.faction
   -- pirates have different fittings than regular escorts
   if persona.ship:tags("pirate") then
      used_faction = faction.get("Pirate")
   end
   local pppp = pilot.add(shipchoice.ship, used_faction)
   for j, o in ipairs(pppp:outfitsList()) do
      deposit = deposit + o:price()
      persona.outfits[#persona.outfits + 1] = o:nameRaw()
   end
   pppp:rm()
   -- adjust the royalty and deposit
   persona.royalty = (shipchoice.royalty + 0.05 * shipchoice.royalty * rnd.sigma())
   persona.deposit = math.floor(deposit - (deposit * persona.royalty)) / 3
   -- additional adjustments based on fleet "success" and synergy
   if string.find(shipchoice.ship, persona.dreamship) then
      -- a discount for dream livers
      persona.deposit = math.max(100e3, persona.deposit - 1e6)
      persona.royalty = math.min(persona.royalty, 0.25 + 0.05 * shipchoice.royalty * rnd.threesigma())
   elseif #mem.persons > 3 and persona then
      -- punish larger fleet additions if we're dreamy
      persona.deposit = persona.deposit + #mem.persons * 37500 * persona.ship:size()
   end
   -- if we are good money makers, we don't want too much deposit even if the rest of the fleet is in disarray
   if persona.total_cost - persona.total_profit * persona.experience >= deposit then
      persona.deposit = math.floor(math.max(persona.deposit, 250e3))
   end

   persona.royalty_percent = persona.royalty * 100
   persona.deposit_text = fmt.credits(persona.deposit)
   persona.replacement_text = fmt.credits(persona.deposit * persona.royalty)
end

function escort_barConversation( persona, npc_id )
   -- if we have a shipyard, we can buy a ship from here
   -- if we will be able to buy a ship, we will have to remember
   -- what ships we chose so that the labels make sense
   local buy_ship, upgrade_ship
   local shipyardextra = "" -- we don't want to put nil in formatter
   if spob.cur():services().shipyard then
      shipyardextra = _("There's a shipyard here, maybe we can take a look?")
      -- pick an upgrade
      local ssold = spob.cur():shipsSold()
      local esp = persona.ship:price()
      for ii, sship in ipairs(ssold) do
         if sship ~= persona.ship then
            local ssp = sship:price()
            -- only consider true upgrades or the dream ship
            if ssp > esp or string.find(sship:nameRaw(), persona.dreamship) then
               -- always prefer upgrading to the dream ship
               if string.find(sship:nameRaw(), persona.dreamship) or string.find(persona.dreamship, sship:nameRaw()) then
                  upgrade_ship = { ship = sship:name(), price = sship:price(), actual = sship, index = ii }
               end
               -- don't allow upgrades far past our current ship size
               if not upgrade_ship and ssp > esp and ssp < esp + 1e6 then
                  -- we haven't picked a ship yet, pick this one because it costs more than our ship
                  upgrade_ship = { ship = sship:name(), price = sship:price(), actual = sship, index = ii }
               elseif upgrade_ship and upgrade_ship.price > ssp and ssp > esp then
                  -- that last ship we picked was too expensive, pick this one instead for the upgrade
                  upgrade_ship = { ship = sship:name(), price = sship:price(), actual = sship, index = ii }
               end
            end

         end
      end
      -- we found a ship but it still looks full price, adjust it here
      -- technically it looks nil because we aren't reusing the same variable
      if upgrade_ship then
         upgrade_ship.credits = fmt.credits(calculateUpgrade(persona, upgrade_ship.price))
         table.remove(ssold, upgrade_ship.index) -- remove the upgrade from the purchase options
      end
      -- hopefully we have an upgrade ship, but we can always get a buy ship if there is a shipyard
      -- PSYCHE! The shipyard might not sell anything else (or anything at all)
      -- so let's at least check that first
      if #ssold > 0 then -- now we're talking, but we're not there yet
         local sship = pick_one(ssold) -- pick a random ship from the shipyard
         -- make sure we're not buying the same ship that the escort has,
         -- the same ship as the upgrade ship, or something that's too expensive
         if sship:price() < esp + 1e6 and sship ~= persona.ship then
            -- I know we don't need to nest this but my readability was hurting
            if not upgrade_ship or sship:nameRaw() ~= upgrade_ship.name then
               buy_ship = { ship = sship:name(), credits = fmt.credits(calculateUpgrade(persona, sship:price())), price = sship:price(), actual=sship }
            end
         end
      end
   end

   local message = ""

   -- check if everyone is in their dream ship or at least a favorite ship
   for _i, pers in ipairs(mem.persons) do
      local myship = pers.ship:nameRaw()
      local favourite = pick_favorite_ship(pers)
      if
            not string.find(myship, favourite)
            and not string.find(favourite, myship)
            and not string.find(myship, pers.dreamship)
      then
         if pers == persona and rnd.rnd(0, 1) == 1 then -- commander is more likely to complain about his own dream ship
            message = fmt.f(_("You know, I'd really love a {dreamship}."), persona) .. " " .. shipyardextra
         elseif rnd.rnd(1,3) == 2 and pers ~= persona then -- don't always say it, and randomly pick who is complaining
            message = fmt.f(_("{name} really wants a {dreamship} and isn't very happy with the {ship}, at least not as it stands. "), pers) .. shipyardextra
         end
      end
   end

   -- now figure out which choices to show
   local choices = {}
   local approachtext = fmt.f(_([[Hi {name}. What do you need?]]), {name = player.name()})
   if persona.commander then -- the commander is chummy
      approachtext = fmt.f(_([[Hello {captain}. How can I be of service? {message}]]),
      {
         message = message,
         captain = pick_one({
            _("Captain"),_("Captain"),_("Captain"),_("captain"),_("captain"),
            _("Commander"),_("Commander"),_("commander"),_("commander"),_("commander"),
            _("Commodore"),_("commodore"),_("commodore"),_("commodore"),_("commodore"),
            _("admiral"),
            _("general"),
            _("chief"),_("chief"),_("Chief"),
            _("chief commander"),
            fmt.f(_([["the Legend" of {shipname}]]), {shipname = player.pilot():name()}),
            fmt.f(_("{name}"), {name = player.name()}),
            _("cap'n"),_("cap'n"),_("Cap'n"),
            _("captian"),
            _("captiain"),
            _("captiaine"),
            _("cap'n Hook"),
            _("supreme leader"),
            _("supreme commander"),
            _("you generous general"),
            _("you admirable admiral"),
            _("you cheeky little bugger"),
            _("you heroic slayer"),
            _("you brave lion"),
            _("glorious bastard"),
            _("fearsome fiend"),
            _("fearsome friend"),
         })
      })
      if persona.active then
         table.insert(choices, { _("Take a break"),    "leave_begin" } )
      else
         table.insert(choices, { _("Return to duty"), "leave_end" } )
      end
   else
      if persona.active then
         table.insert(choices, { _("Take a break"), "despawn" } )
      else
         table.insert(choices, { _("Return to duty"), "respawn" } )
      end
   end
   -- don't let the player buy the upgrades if he can't afford a whole new ship
   -- since the escort is going to probably die soon anyway and ask for reimbursement
   if buy_ship and player.credits() > buy_ship.price then
      table.insert(choices, { fmt.f(_("Buy {ship} ({credits})"), buy_ship ), "purchase" })
   end
   if upgrade_ship and player.credits() > upgrade_ship.price then
      table.insert(choices, { fmt.f(_("Upgrade to {ship} ({credits})"), upgrade_ship), "upgrade" })
   end

   -- standard choices
   local pilotlabel = "Pilot"
   if persona.commander then pilotlabel = "Commander" end
   table.insert(choices, { fmt.f(_("Fire {pilot}"), {pilot=pilotlabel} ), "fire" } )
   table.insert(choices, { _("Dismiss"), "end" })

   -- non commander pilots might be disobedient
   local disobedience = pick_one({
      _([[Actually, I think I'll hang back for a bit and read the instruction manual.]]),
      _([[Actually, I think I'll hang back for a bit and read the news.]]),
      _([[Actually, I think I'll hang back a bit.]]),
      _([[I'm on a break.]]),
      _([[What part of "I'm on a break" don't you understand?]]),
      _([[What part of me being on a break don't you understand?]]),
      _([[I'm actually kind of enjoying this break.]]),
      _([[Now that I'm not cooped up in a ship, I think I'll relax for a little while.]]),
      _([[Why don't you come back later, I'm busy.]]),
      _([[Why don't you ask me again later, I'm tired.]]),
      _([[Come back later, I have a video call with someone soon and it's going to be expensive.]]),
      _([[Ask me later, I don't feel like it right now.]]),
      _([[Come back later, my left phalange is defective and needs to be replaced.]]),
      _([[You know, I'd love to help you, but I don't want to.]]),
      _([[Don't you think I could use some time to read the instruction manual?]]),
      _([[You know, I'm actually feeling pretty good over here. I think I'll hang back.]]),
      _([[You're not the boss of me!]]),
      fmt.f(_([[Where's my {dreamship}?]]), persona),
      fmt.f(_([[Did you get my {dreamship}?]]), persona),
      fmt.f(_([[Did you buy me a {dreamship} yet?]]), persona),
      fmt.f(_([[Sure, let me just get in my {dreamship}...]]), persona),
      fmt.f(_([[Sure, let me just get in my {dreamship}... Oh wait a minute, I don't have one.]]), persona),
      fmt.f(_([[Oh, you mean in that {dreamship} that I don't have?]]), persona),
      fmt.f(_([[Listen pal, {name}'s not going back to work until {name} gets a {dreamship}, got it?]]), persona),
   })

   vn.clear()
   vn.scene()
   local escort = vn.newCharacter ( persona.name, {image=persona.vncharacter } )
   vn.transition()
   vn.na(getCredentials(persona))
   escort(approachtext)
   -- maybe here the escort can say something like "I saw <ship> in the shipyard"
   vn.na(approachtext .. "\n" .. getFinances(persona))
   vn.menu( choices )

   vn.label("leave_begin")
   vn.func( function ()
      persona.active = not persona.active
      for _i, pers in ipairs(mem.persons) do
         pers.active = persona.active
         pers.portrait = pers.portrait_offduty
         if not pers.commander then
            pers.vncharacter = portrait.getFullPath(pers.portrait)
         end
      end
      -- commander takes off the uniform
      persona.portrait = persona.portrait_offduty
   end )
   escort(_([[Always nice to take a break for a little while. Just let me know when you need an eye on your back before you need it.]]))
   vn.na(_([[You have placed your commander and the rest of the support fleet on temporary leave.]]))
   vn.done()

   vn.label("leave_end")
   vn.func( function ()
      persona.active = not persona.active
      for _i, pers in ipairs(mem.persons) do
         pers.active = persona.active
         pers.portrait = pers.portrait_onduty
         if not pers.commander then
            pers.vncharacter = portrait.getFullPath(pers.portrait)
         end
      end
      -- commander suits up
      persona.portrait = pilot_portrait(persona.commander)
   end )
   escort(_([[It was nice while it lasted, but vacation doesn't bring in the credits. Point us to the ships and salvage and we'll watch your back and split the profits.]]))
   vn.na(_([[Your commander and the support fleet has been ordered back to active duty.]]))
   vn.done()

   vn.label("despawn")
   vn.func( function()
      if not persona.portrait_offduty and persona.commander then
         persona.portrait_offduty = string.gsub(string.gsub(persona.vncharacter, ".webp", "n.webp"), "_nogogn", "n_nogog")
      end
      persona.portrait = persona.portrait_offduty
      if not persona.commander then
         persona.vncharacter = portrait.getFullPath(persona.portrait)
      end
      persona.active = false
   end )
   escort(_([[Yeah, sure, whatever.]]))
   vn.done()

   vn.label("respawn")
   vn.func( function()
      local has_commander = nil
      for _i, other in ipairs(mem.persons) do
         if other.commander then
            has_commander = other
         end
      end
      -- if we're not in a nice ship, we don't really wanna work, do we?
      if rnd.rnd(1, 7) == 7
         or string.find(persona.ship:nameRaw(), persona.dreamship)
         or string.find(persona.ship:nameRaw(), pick_favorite_ship(persona))
         or string.find(pick_favorite_ship(persona), persona.ship:nameRaw())
         or has_commander and rnd.rnd(1,3) == 3 -- having a commander can increase the odds though
         then
         persona.portrait = persona.portrait_onduty
         persona.vncharacter = portrait.getFullPath(persona.portrait)
         persona.active = true
      else
         vn.jump("disobedient")
      end
   end )
   escort(_([[Oh, okay... You got it, boss.]]))
   vn.done()

   vn.label("purchase")
   vn.func( function ()
      escort_buyUpgrade(persona, buy_ship)
      -- the escort changes outfits after getting a new ship
      persona.portrait_onduty = pilot_portrait(persona.commander)
      persona.portrait = persona.portrait_onduty
   end )
   escort(_([["Always nice to get a change of pace. Hopefully I remember how to fly it!"]]))
   vn.done()
   vn.label("upgrade")
   vn.func( function ()
      escort_buyUpgrade(persona, upgrade_ship)
      -- the escort changes outfits after getting an upgrade
      persona.portrait_onduty = pilot_portrait(persona.commander)
      persona.portrait = persona.portrait_onduty
   end )
   escort(_([["Nice! It's about time I got an upgrade."]]))
   vn.done()
   vn.label("fire")
   escort(_([[Are you sure you want to do that? You know that once I'm gone, I'm never working for you again.]]))
   vn.menu( {
      { _("Fire Pilot") , "fire_yes" },
      { _("Nevermind") , "end" }
   } )
   vn.done()
   vn.label("fire_yes")
   escort(_([[I can't believe it. So ungrateful. I'm better off without you.]]))
   vn.na(fmt.f(_([[{name}'s employment has been terminated.]]), persona))
   vn.func( function ()
      fireEscort(persona, npc_id)
   end )
   vn.done()
   vn.label("disobedient")
   escort(disobedience)
   vn.done()
   vn.label("end")
   escort(_([[Alright Captain. Nice talking to you.]]))
   vn.done()
   vn.run()
end

function approachHiredScav(npc_id)
    local edata = npcs[npc_id]
    if edata == nil then
        evt.npcRm(npc_id)
        npcs[npc_id] = nil
        return
    end

--    pilot_askUpgrades(edata, npc_id)
   escort_barConversation(edata, npc_id)
end


-- searches for a valid leader type in the field
-- returns the leader if found
function fall_in( edata )
   if not edata.pilot or not edata.pilot:exists() then return nil end
   -- find a leader
   for i, persona in ipairs(mem.persons) do
      local candidate = persona.pilot
      if persona.commander and candidate and candidate:exists() and not candidate:leader() and candidate ~= edata.pilot and candidate:leader() ~= edata.pilot then
         -- this "candidate" is our leader, already on a mission, join them
         edata.pilot:setLeader(candidate)
--         speak(edata, "join", persona)
         local candmem = candidate:memory()
         candmem.aggressive = true
         return candidate
      end
   end

   -- we didn't find a leader
   return nil
end

function enlist_followers( edata )
   local enlisted = 0
   -- no other leader found (or we are the commander), let's enlist help ourselves from lower ranks
   for i, persona in ipairs(mem.persons) do
      local follower = persona.pilot
      if follower and rnd.rnd() > 0.5 and follower ~= edata.pilot and follower:exists() and follower:leader() ~= edata.pilot and (edata.commander or edata.experience >= persona.experience) then
         speak(persona, "join", edata)
         follower:setLeader(edata.pilot)
         enlisted = enlisted + 1
      end
   end

   -- we aren't a real commander, so we need anything we can get
   if not edata.commander then
      -- we aren't qualified to lead, request more reinforcements
      for _i, follower in ipairs(player.pilot():followers()) do
         -- we don't enlist fighters and enlistment depends on our experience up to max 2/3 chance
         if not follower:flags("carried") and rnd.rnd() < math.floor(math.min(0.67, edata.experience / 100)) then
            follower:setLeader(edata.pilot)
            enlisted = enlisted + 1
         end
      end
      if enlisted > 0 then
         edata.pilot:comm(fmt.f(_("I'll take these {number} with me then."), {number = enlisted}))
      else
         edata.pilot:comm(fmt.f(_("Oh... Okay."), {number = enlisted}))
      end
   end
end

local function escort_spaceMenu(edata, index)
    local approachtext = _([[Would you like to do something with this pilot?

Pilot credentials:]])
   -- so it costs around 3 million to commend a junior lieutenant in a destroyer
   local tip_amount = edata.ship:size() * 5e3 * math.ceil(3 + edata.experience * (0.11 + (edata.experience * 0.0667)))
   local tip_max = 10e6
    tip_amount = math.min(tip_amount, tip_max)

    if player.pilot():credits() < tip_amount then
        return
    end
   local commendation_label = _("Commend efforts")
   local commendation_prompt = _("An effective commendation of a {rank} of this calibre will cost {price} and increase this {rank}s favor with the {ship}. Are you sure you want to commend the efforts of {rank} {name} of {ship}?")
   if edata.experience < 10 then
      commendation_label = _("Compliment ship")
      commendation_prompt = _("An effective compliment for a {rank} of this calibre will require a symbolic gift worth {price}. This will increase this {rank}s favor with the {ship}, increasing the likelihood of this {rank} purchasing this ship as a replacement. Flying a favorite ship will make the {rank} more happy and generous. Are you sure you want to compliment {rank} {name}'s {ship}, thereby increasing the favor of this ship for this pilot?")
   end
   local aimem = edata.pilot:memory()
    local n, _s =
        tk.choice(
        "",
        getOfferText(approachtext, edata),
        _(commendation_label),
        _("Get to Work"),
        _("Call back"),
        _("Do nothing")
    )
    if
        n == 1 and
            vntk.yesno(
                "",
                fmt.f(
                    commendation_prompt,
                    {name = edata.name, price = fmt.credits(tip_amount), ship=edata.ship, rank=edata.rank}
                )
            )
     then
        --     mem.persons[index].total_profit = mem.persons[index].total_profit - tip_amount -- do the credit side now since debit comes later
        if player.credits() < tip_amount * 1.25 then
         vntk.msg(
                    _("Not Enough Money"),
                    _("It would be financially irresponsible to commend this pilot, as you risk not being able to cover the replacement fee which is necessary to ensure the retrieval of your pilot after emergency ejection.")
                )
         return
      end

      shiplog.append(
            logidstr,
            fmt.f(
                _("You commended '{name}' {credits} ({ship})."),
                {name = edata.name, ship = edata.ship, credits = fmt.credits(tip_amount)}
            )
        )
      edata.experience = edata.experience + 0.25
      remember_ship(edata, edata.ship:nameRaw(), 3)
        player.pilot():credits(-tip_amount)
        edata.wallet = edata.wallet + tip_amount
        if edata.pilot ~= nil and edata.pilot:exists() then
            edata.pilot:credits(tip_amount)
        end
        mem.persons[index].total_cost = mem.persons[index].total_cost + tip_amount
    elseif n == 2 then
      edata.pilot:setLeader(nil)
      local allow_leadership = edata.experience > edata.ship:size() * 16 and rnd.rnd(0, 1) == 1
      -- try to assume leadership, depends on random chance and our experience level unless we are a commander
      if edata.commander or allow_leadership then
         edata.pilot:changeAI("escort_guardian")
         aimem = edata.pilot:memory()
         aimem.aggressive = true
         if edata.experience > 50 then
            aimem.formation = "circle"
         elseif edata.experience > 30 then
            aimem.formation = "wedge"
         elseif edata.experience > 20 then
            aimem.formation = "cross"
         elseif edata.experience > 10 then
            aimem.formation = "vee"
         elseif edata.experience > 2 then
            aimem.formation = "wall"
         else
            aimem.formation = nil
         end
         -- since we are actively guarding the player, let's make sure we pick up player attacked
         -- only the commander can remove a hook manually
         if edata.commander and edata.commander.hook then
            hook.rm(edata.commander.hook)
         end
         -- we can only set a hook if none exists, since the commander may have set one already
         if edata.commander and edata.commander.hook == nil then
            edata.commander.hook = hook.pilot(player.pilot(), "attacked", "player_attacked", edata.pilot)
         end
         -- if we are the chosen commander of our squadron then enlist followers
         if (fall_in(edata)) == nil and edata.pilot and edata.pilot:exists() then
            hook.timer(2, "enlist_followers", edata)
         end
      else
         if fall_in(edata) == nil then
            edata.pilot:setLeader(player.pilot())
         end
         edata.pilot:changeAI("pirate")
         edata.pilot:memory().leadermaxdist = 500 * edata.pilot:ship():size()
      end
      edata.pilot:memory().atk_board = true
    elseif n == 3 then
      local pp = player.pilot()
      for _i, follower in ipairs(edata.pilot:followers()) do
         follower:setLeader(pp)
         follower:changeAI("pirate")
         aimem.leadermaxdist = 500 * follower:ship():size()
      end
      edata.pilot:changeAI("pirate")
        edata.pilot:setLeader(pp)
      aimem.atk_board = true
      aimem.aggressive = false
      aimem.careful = true
      aimem.leadermaxdist = 500 * edata.pilot:ship():size()
      aimem.enemyclose = 300 * edata.pilot:ship():size()
    end
end

-- don't allow regular hailing, escort commands instead
function scav_hail(_p, arg)
    player.commClose()

    local edata = mem.persons[arg]
    escort_spaceMenu(edata, arg)
end