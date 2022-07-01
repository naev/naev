--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Scavenger Escort Handler11">
 <location>load</location>
 <chance>100</chance>
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
local pir = require "common.pirate"
local fmt = require "format"
local portrait = require "portrait"
local pilotname = require "pilotname"

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
    _("It's too quiet here."),
    _("It's pretty silent in space. Makes you thankful for all the electronics."),
    _("What's that buzzing noise?"),
    _("Do you hear drumming?"),
    _("Do you hear that beat?"),
    _("I hope my {ship} is up to code."),
    _("I can feel it.")
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
    _(
        "My mother escaped from Stutee, and well, I don't think I have to tell you what that means, but she'd be stuck at Leszec today."
    ),
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
    _("What we all really want is our fair share of the action.")
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
    _("I could have sworn I saw a picture of one of the pilots in our fleet at the spaceport bar in Zabween.")
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
    _("I hope you're not too bothered about having spent {total_cost} on me."),
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
    _("A {dreamship} would be fitting for a {rank}.")
}

-- positive chatter
local chitchat_dreamship = {
    _("Is there anything to salvage around here?"),
    _("I'll bet we find something interesting today."),
    _("I love my {dreamship}."),
    _("I really love my {dreamship}."),
    _("Have I told you how much I appreciate my {dreamship}?"),
    _("This is the life."),
    _("Ahhh, this is the life. I owe it all to the {dreamship}."),
    _("Everything will be okay, as long as I have my {dreamship}."),
    _(
        "I figure you've probably spent a lot on me by now, but it was all worth it. I'm thankful for every day I get with my {dreamship}."
    ),
    _("I'm thankful for every day I get with my {dreamship}."),
    _("I hope you realize that this thing is going to cost you {replacement_text} if I have to eject."),
    _("My {dreamship} will cost you {replacement_text} to cover. I hope it lasts."),
    _("I'm in a good mood."),
    _("I've got a good feeling about this'"),
    _("You know, you're insane but I like it."),
    _("Let's not scratch the paint on my {dreamship} today."),
    _("I just had this {dreamship} cleaned, can you notice?"),
    _("This {dreamship} is perfect for a {rank} like me.")
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

local chitchat_brb = {
    _("I'm going to take a look around for a bit."),
    _("I'll catch up with you later."),
    _("I'll be back."),
    _("Call me if you need me."),
    _("I might be hard to find if you need me..."),
    _("Let's see if there's anything interesting around...")
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
    _("Next time we're full of loot and plunder, let's come back here.")
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
    _("What do you do when a {thing} stops functioning?"),
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

local function speak(persona, sentiment, arg)
    local ss = sentiment or "idle"
    local spoken = nil
    local force = false
    local speaker = persona.pilot

    if arg ~= nil and arg["speaker"] then
        speaker = arg["speaker"]
    end

    if ss == "idle" then
        if persona.last_sentiment then
            local last = persona.last_sentiment
            persona.last_sentiment = nil
            return speak(persona, last)
        elseif rnd.rnd() < 0.042 then
            spoken = pick_one(chitchat_rare)
        elseif persona.commander then
            spoken = pick_one(chitchat_commander)
        else
            spoken = pick_one(chitchat_idle)
        end
    elseif ss == "content" then
        spoken = pick_one(chitchat_content)
    elseif ss == "dreamship" then
        spoken = pick_one(chitchat_dreamship)
    elseif ss == "negative" then
        if persona.dreamship ~= persona.ship:nameRaw() then
            spoken = pick_one(chitchat_negative)
        else
            spoken = pick_one(chitchat_dreamship)
        end
    elseif ss == "broken_dreams" then
        -- make sure you don't complain if you replaced your dream ship with a new dream ship
        if persona.dreamship ~= persona.ship:nameRaw() then
            spoken = pick_one(chitchat_broken_dreams)
        else
            spoken = pick_one(chitchat_dreamship)
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
    -- get current point value
    local current_score = 0
    -- edge case: never seen the ship before
    if me.favorite_ships[ship] == nil then
        me.favorite_ships[ship] = points
        return
    end

    -- default: increase
    current_score = me.favorite_ships[ship]
    me.favorite_ships[ship] = current_score + points
    print(
        fmt.f(
            "{name} remembers {ship} is worth {points} points.",
            {name = me.name, ship = ship, points = points + current_score}
        )
    )
    return
end

local function pick_favorite_ship(me)
    -- egde case create if no exist
    if me.favorite_ships == nil then
        me.favorite_ships = {}
    end

    local min_score = 1
    local max_score = 0
    local choices = {}

    -- lazy method favors first-seen ships
    for ship, score in pairs(me.favorite_ships) do
        if score >= max_score then
            max_score = score
        end
    end

    min_score = math.floor(max_score / 3)
    print(fmt.f("the minimum score is {ms}", {ms = min_score}))

    for ship, score in pairs(me.favorite_ships) do
        if score >= min_score then
            table.insert(choices, ship)
            print(fmt.f("Considering a {ship} worth {points}", {ship = ship, points = score}))
        else
            print("Skipping ", ship)
        end
    end

    if next(choices) == nil then
        choices = {"Shark"} -- give it a default
    end

    choice = choices[rnd.rnd(1, #choices)]

    return choice
end

local function getOfferText(approachtext, edata)
    local credentials =
        _(
        [[
Pilot name: {name}
Rank: {rank} ({experience} merit)
Ship: {ship}
Goal: {dreamship}
Deposit: {deposit_text}
Cover fee: {royalty_percent:.1f}% of deposit
]]
    )

    local finances =
        _(
        [[
Money: {credits}
Expenses to date: {total}
Escort debt: {debt}
Escort profits: {profits}
Escort paid tribute: {tribute} ]]
    )
    return (approachtext ..
        "\n\n" ..
            fmt.f(credentials, edata) ..
                "\n\n" ..
                    fmt.f(
                        finances,
                        {
                            credits = fmt.credits(edata.wallet),
                            total = fmt.credits(edata.total_cost),
                            debt = fmt.credits(edata.debt),
                            profits = fmt.credits(edata.total_profit),
                            goal = edata.dreamship,
                            tribute = fmt.credits(edata.tribute)
                        }
                    ))
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
            persona.tribute * persona.experience
    )

    if persona.pilot then
        local pmem = persona.pilot:memory()
        local boarded = pmem.boarded or 0
        --		persona.pilot:broadcast(fmt.f("I have boarded {boarded} times and made {profit} total.", { boarded=boarded, profit=persona.total_profit } ))
        if boarded > 0 then
            persona.experience = persona.experience + 1
            remember_ship(persona, persona.ship:nameRaw(), boarded)
        end
    end

    if persona.experience >= 1 then
        budget = budget + math.floor(persona.experience * persona.royalty * persona.deposit)
        if persona.pilot ~= nil then
            persona.pilot:broadcast(
                fmt.f(
                    "My budget was boosted to {budget} due to my experience level of {exp}",
                    {budget = fmt.credits(budget), exp = persona.experience}
                )
            )
        end
    end

    local dream_budget = ship.get(persona.dreamship):price()

    -- TODO Here: license restrictions and limits
    if limit_ships ~= nil and #limit_ships > 0 then
        local most_expensive = "Shark"
        local most_cost = 1e3
        ship_choices = {}
        -- just add the available ships at standard rates
        local rate = 0.19
        for i, ship in ipairs(limit_ships) do
            local price = ship:price()
            rate = math.min(0.95, 0.19 + 0.07 * ship:size())
            -- don't buy the ship if it's much more expensive than our dream ship
            if budget > price and dream_budget > price then
                table.insert(ship_choices, {ship = ship:nameRaw(), royalty = rate})
                if price > most_cost then
                    most_cost = price
                    most_expensive = ship:nameRaw()
                end
            end
        end
        -- if we are the commander, buy the most expensive ship we can afford at a good rate
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
        elseif budget > 20e6 then
            table.insert(ship_choices, {ship = "Zebra", royalty = 0.50})
            table.insert(ship_choices, {ship = "Zebra", royalty = 0.67})
            table.insert(ship_choices, {ship = "Zebra", royalty = 0.69})
            table.insert(ship_choices, {ship = persona.dreamship, royalty = 0.14})
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
            if persona.dreamship ~= "Rhino" and persona.dreamship ~= "Pirate Rhino" then
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
    if budget > dream_budget and shipchoice.ship ~= persona.dreamship and rnd.rnd() < 0.33 then
        if shipchoice.dreamship ~= "Shark" then
            shipchoice = {ship = persona.dreamship, royalty = 0.15}
        end
    elseif rnd.rnd() < 0.16 then -- let's pick a favorite ship instead, or a shark
        shipchoice = {ship = pick_favorite_ship(persona), royalty = 0.25 + rnd.threesigma() * 0.05}
        print("Picking a favorite ship ", shipchoice.ship, " for ", persona.name)
    end

    -- final check, don't allow ships that we don't have the license for
    -- TODO

    persona.ship = ship.get(shipchoice.ship)
    local _n, deposit = persona.ship:price()
    persona.outfits = {}
    local pppp = pilot.add(shipchoice.ship, persona.faction)
    for j, o in ipairs(pppp:outfits()) do
        deposit = deposit + o:price()
        persona.outfits[#persona.outfits + 1] = o:nameRaw()
    end
    pppp:rm()
    persona.royalty = (shipchoice.royalty + 0.05 * shipchoice.royalty * rnd.sigma())
    persona.deposit = math.floor(deposit - (deposit * persona.royalty)) / 3
    if shipchoice.ship == persona.dreamship then
        -- punish larger fleet additions if we're dreamy
        persona.deposit = math.max(100e3, persona.deposit - 1e6)
        persona.royalty = math.min(persona.royalty, 0.25 + 0.05 * shipchoice.royalty * rnd.threesigma())
    elseif #mem.persons > 3 and persona then
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
        local downpayment = 0
        if deposit > 6e6 then
            downpayment = persona.wallet * 0.3
            persona.wallet = math.floor(persona.wallet - downpayment)
        elseif deposit > 2e6 then
            -- we bought something small but kind of expensive
            downpayment = persona.wallet * 0.04
            persona.wallet = math.floor(persona.wallet - downpayment)
        end
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

    if rnd.rnd() < 0.1 then
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
        firstname = "Pilot"
    end

    local shipchoice = ship_choices[rnd.rnd(1, #ship_choices)]
    local p = pilot.add(shipchoice.ship, fac)
    local _n, deposit = p:ship():price()

    local newpilot = {}

    newpilot.outfits = {}
    for j, o in ipairs(p:outfits()) do
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
    newpilot.chatter = 0.5 + 0.1 * rnd.threesigma() -- add some personality flavour
    newpilot.dreamship = dream_choices[rnd.rnd(1, #dream_choices)]
    for _j, shipname in ipairs(big_dreams) do
        -- check if we are a commander
        if newpilot.dreamship == shipname then
            newpilot.commander = true
        end
    end

    return newpilot
end

function createScavNpcs()
    local num_npcs = rnd.rnd(0, max_escorts - #mem.persons + max_scavengers)

    local cmdr_fudge = 0
    for i = 1, num_npcs do
        local newpilot = create_pilot(faction.get("Affiliated"))
        if newpilot.commander then
            cmdr_fudge = 2
        end
        --		print(fmt.f("created a pilot named {name}", newpilot ) )
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
        _("gruesome warrior")
    }

    local random_quote1 = "Well,"
    local random_quote2 = "Anyway,"

    if pdata.chatter > 0.4 then
        random_quote1 = rand_quotes[rnd.rnd(1, #rand_quotes)]
        if pdata.chatter >= 0.6 then
            random_quote2 = rand_quotes[rnd.rnd(1, #rand_quotes)]
        end
    end

    if
        not tk.yesno(
            _("Pilot Apprentice"),
            fmt.f(
                _(
                    [["Greetings Commodore {name} of the {shipname}. My name is {myname} -- I've heard so much about you and your {ship} and your skill in battle. {random_quote} I am eager to fly alongside you, but my dream is to pilot a {dreamship} one day. Help me reach this goal and I will be your most {companion}.

The arrangement is simple: you pay an initial deposit and cover a percentage of my replacement and maintenance fees. {part1} {part2}
{random_quote2} I think I'll learn a lot with you, so what do you say...

Will you pay {credits} to kickstart my career as your apprentice?"]]
                ),
                {
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
            )
        )
     then
        return -- player rejected offer
    end

    local has_commander = nil
    for _i, other in ipairs(mem.persons) do
        if other.commander then
            has_commander = other
        end
    end

    if pdata.commander and has_commander then
        tk.msg(
            _("Unwelcome Animosity"),
            fmt.f(
                _(
                    [["I can't fly with {name}. We used to work together at Janus Station. All the talking about dreams of flying a {dreamship}, when clearly some old {ship} would be a much more appropriate career choice. Maybe someone else will tolerate that insufferable chatterbox."]]
                ),
                has_commander
            )
        )
        return
    end

    -- can't hire too many escorts if you don't have a commander
    if #mem.persons >= max_escorts or #mem.persons >= max_scavengers and not has_commander then
        tk.msg(
            _("Too many affiliates"),
            _(
                [["You look like you've got enough problems on your hands already. I think I'll stay out of your way for now."]]
            )
        )
        return
    end

    if pdata.deposit and pdata.deposit > player.credits() then
        tk.msg(_("Not Enough Money"), _([["I can't fly on dreams and steam. Come back when you have enough money."]]))
        return
    end

    local title_choices = {_("Of course"), _("Splendid"), _("Surprise"), _("Hardly unexpected")}

    tk.msg(
        pick_one(title_choices),
        fmt.f(
            _(
                [[You pay the young fool, who instantly scurries towards the shipyard.
    As you take off, you notice your new sidekick is now in what looks like a {ship}.]]
            ),
            {
                ship = fmt.f("{ship}", pdata)
            }
        )
    )
    if pdata.deposit then
        player.pay(-pdata.deposit, true)
    end

    local i = #mem.persons + 1

    pdata.alive = true
    mem.persons[i] = pdata
    evt.npcRm(npc_id)
    npcs[npc_id] = nil
    local id =
        evt.npcAdd(
        "approachHiredScav",
        pdata.first_name .. " " .. pdata.name,
        pdata.portrait,
        _("This is a new pilot under your wing."),
        8
    )
    npcs[id] = pdata
    evt.save(true)

    shiplog.create(logidstr, _("Hired Escorts"), _("Hired Escorts"))
    shiplog.append(
        logidstr,
        fmt.f(
            _(
                "You hired a {ship} ship named '{name}' for {deposit_text} at a {royalty_percent:.1f}% initial cover charge."
            ),
            pdata
        )
    )
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
        "You can have {your_share}.",
        "{your_share} for {flagship}.",
        "The {flagshipt} gets {your_share}.",
        "Here's your cut.",
        "That comes down to {your_share} for {player}, {my_share} for me.",
        "I'm keeping {my_share}, that sound about right?",
        "I hope you're happy with {your_share}.",
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
        _("It's alright.")
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
        _("Not {cargo}, again!"),
        _("Oh, it's just commodities."),
        _("Oh, it's just some cargo."),
        _("It's just commodities."),
        _("Just some cargo."),
        _("Just some cargo. {cargo} and the likes."),
        _("It's random cargo."),
        _("It's random cargo such as {cargo}."),
        _("It had some commodities."),
        _("It had some commodities like {cargo}.")
    }
    local bounty = target:credits()
    local speak_chance = mem.persons[i].chatter or 0.5
    local exp_boost = math.floor(bounty / 10e3) * 0.1

    -- try to loot cargo
    local cargofree = plt:cargoFree()
    if cargofree then
        local clist = target:cargoList()
        for _k, c in ipairs(clist) do
            n = plt:cargoAdd(c.name, c.q)
            exp_boost = exp_boost + math.floor(c.q / 10) * 0.01
            table.insert(payout_choices, fmt.f(pick_one(cargo_options), {cargo = c.name}))
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
        end
        speak_chance = speak_chance + 0.1
    elseif bounty > 60e3 then
        table.insert(payout_choices, "Actually, that's not bad, you get {your_share}!")
        table.insert(payout_choices, "Woah! Well I'm keeping {my_share}.")
        -- we will want to speak about this unless we are really experienced
        local experience_fudge = mem.persons[i].experience / (2 * mem.persons[i].experience + 100)
        speak_chance = speak_chance + math.max(0, 0.5 - experience_fudge)
        exp_boost = exp_boost + 0.05
    elseif bounty > 35e3 then
        table.insert(payout_choices, "Not too shabby, check your logs.")
        table.insert(payout_choices, "I'm happy with {my_share}, here's {your_share}.")
        speak_chance = speak_chance + 0.1
        exp_boost = exp_boost + 0.01
    end

    local choice = choices[rnd.rnd(1, #choices)] .. " " .. payout_choices[rnd.rnd(1, #payout_choices)]

    if bounty > 20e3 then
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
    if mem.persons[i].ship:nameRaw() == mem.persons[i].dreamship then
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
                _("'{name}' paid you {credits} from a total bounty of {total} ({ship})."),
                {
                    name = mem.persons[i].name,
                    ship = mem.persons[i].ship,
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
        if mem.persons[i].last_sentiment ~= "content" then
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
            my_share = fmt.credits(bounty - your_share)
        }
        plt:comm(fmt.f(choice, info))
    end

    --	target:broadcast(fmt.f("I was boarded by {name} and I had {credits}.", {name = mem.persons[i].name, credits=fmt.credits(bounty)}), true)
end

function scavenger_arrives(arg)
    local i = arg.i
    local f = mem.persons[i].faction
    -- if we are already with the player, don't respawn
    if mem.persons[i].pilot ~= nil and mem.persons[i].pilot:exists() then -- pilot still exist
        print("pilot still exists:", mem.persons[i].name)
        return
    end
    plt = pilot.add(mem.persons[i].ship, f, arg.spawnpoint, mem.persons[i].name, {naked = true})
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

    plt:setVisplayer(true)
    plt:setNoClear(true)
    mem.persons[i].alive = true

    hook.pilot(plt, "death", "scavenger_death", i)
    hook.pilot(plt, "hail", "scav_hail", i)
    hook.pilot(plt, "attacked", "scav_attacked", i)
    hook.pilot(plt, "boarding", "scav_boarding", i)

    --	plt:broadcast(fmt.f("I'm number {i}, had to pay off {credits}.", { i=i , credits=fmt.credits(arg.debt / 2) } ))

    if mem.persons[i].spawning then
        hook.rm(mem.persons[i].spawning)
    end
    mem.persons[i].pilot = plt
    mem.persons[i].spawning = false

    -- usually, we set the leader, but sometimes, don't

    if plt.commander and rnd.rnd() < 0.02 then
        -- I'll catch up with you later
        speak(mem.persons[i], "brb")
        local aimem = plt:memory()
        aimem.stealth = true
        -- if I'm in a dream ship, enlist help
        if mem.persons[i].ship:nameRaw() == mem.persons[i].dreamship then
            for j, pers in ipairs(mem.persons) do
                if i ~= j and pers.pilot ~= nil then
                    if pers.pilot:exists() then
                        pers.pilot:setLeader(plt.pilot)
                        speak(mem.persons[j], "affirm")
                        aimem = pers.pilot:memory()
                        aimem.stealth = true
                    end
                end
            end
        end
    else
        plt:setLeader(player.pilot())
        -- always speak less and less with experience
        local speak_chance = mem.persons[i].chatter - (math.min(300, mem.persons[i].experience) / 1000)
        if rnd.rnd() > speak_chance then -- if I'm a chatterbox
            if rnd.rnd() > 0.5 then -- be positive
                speak(mem.persons[i], "idle")
            else -- be negative
                speak(mem.persons[i], "negative")
            end
        end
    end

    -- carry over cargo from earlier
    if mem.persons[i].cargo ~= nil then
        for i, v in pairs(mem.persons[i].cargo) do
            plt:cargoAdd(v.name, v.q)
        end
    end
end

local function scav_hook_spawner(plt, i, spawnpoint)
    local arg = {i = i, spawnpoint = spawnpoint, debt = plt.debt}
    local delay = 6 + math.floor(math.max(rnd.rnd(1, 6), plt.debt / 10e4)) * plt.ship:size()

    arg.spawnpoint = spawnpoint or pilot.choosePoint(faction.get("Escort"), true)

    if plt.spawning then
        --			player.pilot():comm(fmt.f("Can't spawn {person} because {spawning} {alive}", { person = v.name, spawning=v.spawning, alive=v.alive} ))
        -- delete the old hook first
        hook.rm(plt.spawning)
    end
    plt.debt = plt.debt - plt.debt / 16 -- decrease the debt
    --		player.pilot():broadcast(fmt.f("{name} arrives in approximately {time} s", {name=plt.name, time=delay}))
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
    local difference = mem.persons[i].pilot:credits() - mem.persons[i].wallet
    --	mem.persons[i].pilot:broadcast(fmt.f("I currently have {credits}", { credits = mem.persons[i].wallet }))
    --	mem.persons[i].total_profit = mem.persons[i].total_profit + difference
    mem.persons[i].wallet = mem.persons[i].pilot:credits() -- update wallet
    --	mem.persons[i].pilot:broadcast(fmt.f("I updated my wallet to {credits}", { credits = mem.persons[i].wallet }))

    -- disable the comment to prevent double counting
    -- mem.persons[i].wallet = mem.persons[i].pilot:credits() -- update wallet
end

function jumpout()
    mem.lastsys = system.cur()
    for i, persona in ipairs(mem.persons) do
        if persona.pilot ~= nil then
            if persona.pilot:exists() then
                local pmem = persona.pilot:memory()
                local boarded = pmem.boarded or 0
                --			persona.pilot:broadcast(fmt.f("I have boarded {boarded} times and made {profit} total.", { boarded=boarded, profit=persona.total_profit } ))
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
            --			mem.persons[i].pilot:rm()
            --			mem.persons[i].pilot = nil
            if mem.persons[i].spawning then
                hook.rm(mem.persons[i].spawning)
            end
            mem.persons[i].spawning = false
        --		else
        --			print(fmt.f("person {i} is nil pilot", {i=i}))
        end
    end
end

function land()
    mem.lastplanet = spob.cur()
    mem.lastsys = system.cur()
    for i, persona in ipairs(mem.persons) do
        if persona.alive and persona.pilot ~= nil and persona.pilot:exists() then
            update_wallet(i)
            local pmem = persona.pilot:memory()
            local boarded = pmem.boarded or 0
            --			persona.pilot:broadcast(fmt.f("I have boarded {boarded} times and made {profit} total.", { boarded=boarded, profit=persona.total_profit } ))
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
            local price = 0
            local profits = 0
            for _i, cc in ipairs(persona.pilot:cargoList()) do
                -- does this station even buy this cargo??
                price = commodity.get(cc.name):priceAt(spob.cur())
                if price > 0 then -- sell the commodity
                    local gain = math.floor(cc.q * price)
                    profits = profits + gain
                    persona.pilot:cargoRm(cc.name, cc.q)
                end
            end
            -- share some profit with the player
            if profits > 0 then
                local your_share = math.floor(profits * persona.royalty)

                if persona.ship:nameRaw() ~= persona.dreamship then
                    if profits > 10e3 then
                        persona.last_sentiment = "goodhaul"
                    end
                else
                    your_share = math.floor(profits * (1 - persona.royalty))
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

    -- Clean up dead escorts so it doesn't build up, and create NPCs for
    -- existing escorts.
    npcs = {}
    local new_escorts = {}
    for i, edata in ipairs(mem.persons) do
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
            if edata.commander then
                this_desc =
                    _("This is the most capable pilot currently under your wing and is certified to fly a {dreamship}.")
            end
            if edata.ship:nameRaw() ~= edata.dreamship then
                this_desc =
                    this_desc ..
                    _(
                        "\nThis pilot is currently flying a {ship} at a cover fee of {replacement_text}, which will be added to the pilot's personal account of "
                    ) ..
                        fmt.f("{credits}.", {credits = fmt.credits(edata.wallet)})
            end

            this_desc = fmt.f(this_desc, edata)
            local id = evt.npcAdd("approachHiredScav", edata.rank .. " " .. edata.name, edata.portrait, this_desc, 8)
            npcs[id] = edata
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
    local pnt = spob.cur()
    local services = pnt:services()
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
        _("Let's buzz off already!")
    }
    local help_choices = {
        _("Help me, {name}!"),
        _("Where's the {flagship} when you need it?"),
        _("Mayday, we are getting pulverized over here!"),
        _("I'm with you {name}, but I need your help here!"),
        _("We need you {name}!"),
        _("Now is the time to use {flagship}!"),
        _("Show us what {flagship} can do!"),
        _("Aren't there any weapons on {flagship}?")
    }
    -- check if we should change states
    if rnd.rnd() > 0.98 and p:health() < 67 then
        if p:leader() then
            p:setLeader(nil)
            p:broadcast(
                fmt.f(
                    leave_choices[rnd.rnd(1, #help_choices)],
                    {name = player.name(), flagship = player.pilot():ship():name()}
                )
            )
            mem.persons[i].last_sentiment = "beating"
        else
            p:setLeader(player.pilot())
            p:broadcast(
                fmt.f(
                    help_choices[rnd.rnd(1, #help_choices)],
                    {name = player.name(), flagship = player.pilot():ship():name()}
                )
            )
        end
    end
end

-- Escort got killed
function scavenger_death(p, _attacker, i)
    local edata = mem.persons[i]
    if edata.ship:nameRaw() == edata.dreamship then
        edata.last_sentiment = "broken_dreams"
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
            replacement_fee = math.floor(edata.deposit * edata.royalty)
            player.pay(-replacement_fee, true)
            edata.total_cost = edata.total_cost + replacement_fee
            edata.replacement_text = fmt.credits(replacement_fee)
            shiplog.append(logidstr, fmt.f(_("'{name}' ({ship}) was replaced for {replacement_text}."), edata))
            edata.wallet = edata.wallet + replacement_fee
            p:broadcast(fmt.f(_("I hope you have the {replacement_text} to cover me!"), edata), true)
            -- assign him a new ship
            createReplacementShip(i)
            -- spawn a new one soon
            scav_hook_spawner(mem.persons[i], i)
        end
    else
        p:comm(player:pilot(), "Wait a minute, why isn't it ejecting? Oh sh--", true)
        pilot_disbanded(edata, i)
    end
end

-- Escort boarded something, it doesn't trigger
function scavenger_boarding2(p, target, i)
    local edata = mem.persons[i]
    local plunder_total = target:credits()
    local ps = p:stats()
    local pps = player.pilot()
    local loot_strength = 0.1 * ((10 + pps:stats().crew + pps:shipstat("loot_mod", true)) / (10 + ps.crew))
    local plunder =
        math.floor(
        math.max(
            plunder_total * loot_strength + 1000 * target:ship():size() - (500 + p:ship():size()),
            plunder_total * 0.2 + 100
        )
    )

    p:broadcast(
        fmt.f(
            _("{credits1}! I mean, err, {credits2} in here, sending them your way."),
            {credits1 = fmt.credits(plunder_total), credits2 = fmt.credits(plunder)}
        ),
        true
    )

    -- target:credits(-plunder_total)
    -- player.pay(plunder)
    -- give the scavenger some money
    -- p:credits(plunder_total - plunder)

    --shiplog.append( logidstr, fmt.f(_("'{name}' ({ship}) plundered {amount}."),
    --{ name=edata.name, ship=edata.ship, amount=fmt.credits(plunder) }
    --))
end

-- Asks the player whether or not they want to fire the pilot
local function pilot_askFire(edata, npc_id)
    local approachtext = _([[Would you like to do something with this scavenger?

Pilot credentials:]])
    local upgradechoice = "Settle debts"
    local upgradequery = "Are you sure you want to settle {name}'s debt of {credits}?"
    local upgrade_amount = 250e3 -- negligible effect, a red herring and credit sink, but the escort changes ships/outfits
    local limit_ships = spob.cur():shipsSold()

    -- if there is a shipyard, upgrade for more to buy a new ship from this shipyard
    -- (the player is hoping for a dream ship, let's have the odds favorable)
    if next(limit_ships) ~= nil then
        upgrade_amount = 1e6 -- should be enough for a dreamship after a couple of tries with few experience points
    end

    if edata.debt < 1e6 then
        upgradechoice = "Sponsor ugrades"
        upgradequery = "Are you sure you want to sponsor {name} for upgrades, costing you up to {upgrade_amount}?"
    end
    local n, _s =
        tk.choice("", getOfferText(approachtext, edata), upgradechoice, _("Abandon scavenger"), _("Do nothing"))
    if
        n == 1 and
            tk.yesno(
                "",
                fmt.f(
                    upgradequery,
                    {name = edata.name, credits = fmt.credits(edata.debt), upgrade_amount = fmt.credits(upgrade_amount)}
                )
            )
     then
        if edata.debt >= 1e6 then
            if player.pilot():credits() < edata.debt then
                -- not enough credits
                tk.msg(
                    _("Not Enough Money"),
                    _([["I can't fly on dreams and steam. Try again when you have enough money."]])
                )
                return
            end
            player.pay(-edata.debt)
            edata.total_cost = edata.total_cost + edata.debt
            edata.debt = 0
        else
            -- upgrade the escort
            if player.pilot():credits() < upgrade_amount then
                -- not enough credits, upgrade for less
                upgrade_amount = math.floor(player.pilot():credits() / 2)
            end
            -- pay
            player.pay(-upgrade_amount)
            edata.total_cost = edata.total_cost + upgrade_amount
            -- sell current ship
            local _n, ship_escrow = edata.ship:price()

            -- do the balancing
            edata.wallet = edata.wallet + upgrade_amount + ship_escrow - edata.debt
            edata.debt = 0
            -- get a new ship
            _createReplacementShip(edata, limit_ships)
            -- apply upgrade experience bonus
            edata.wallet = edata.wallet + math.floor(upgrade_amount * edata.experience * 0.05)
            edata.last_sentiment = "content"
        end
    elseif
        n == 2 and tk.yesno("", fmt.f(_("Are you sure you want to get rid of {name}? This cannot be undone."), edata))
     then
        local k = #mem.persons
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
        local pilot_id

        if pilot_id then
        --		pilot_disbanded( edata , pilot_id )
        end
    end
end

function approachHiredScav(npc_id)
    local edata = npcs[npc_id]
    if edata == nil then
        evt.npcRm(npc_id)
        npcs[npc_id] = nil
        return
    end

    pilot_askFire(edata, npc_id)
end

local function scav_askUpgrade(edata, index)
    local approachtext = _([[Would you like to do something with this pilot?

Pilot credentials:]])

    tip_amount = math.min(3e6, player.pilot():credits() / 10)

    if player.pilot():credits() < tip_amount then
        return
    end

    local n, _s =
        tk.choice(
        "",
        getOfferText(approachtext, edata),
        _(fmt.f("Tip Pilot {credits}", {credits = fmt.credits(tip_amount)})),
        _("Go scavange"),
        _("Call back"),
        _("Do nothing")
    )
    if
        n == 1 and
            tk.yesno(
                "",
                fmt.f(
                    _("Are you sure you want to tip {name}? This will cost {price}."),
                    {name = edata.name, price = fmt.credits(tip_amount)}
                )
            )
     then
        --	  mem.persons[index].total_profit = mem.persons[index].total_profit - tip_amount -- do the credit side now since debit comes later
        shiplog.append(
            logidstr,
            fmt.f(
                _("You tipped '{name}' {credits} ({ship})."),
                {name = edata.name, ship = edata.ship, credits = fmt.credits(tip_amount)}
            )
        )
        player.pilot():credits(-tip_amount)
        edata.wallet = edata.wallet + tip_amount
        if edata.pilot ~= nil and edata.pilot:exists() then
            edata.pilot:credits(tip_amount)
        end
        mem.persons[index].total_cost = mem.persons[index].total_cost + tip_amount
    elseif n == 2 then
        edata.pilot:setLeader(nil)
        --for i, follower in ipairs(player.pilot():followers()) do
        for i, persona in ipairs(mem.persons) do
            local follower = persona.pilot
            if follower and rnd.rnd() > 0.5 and follower ~= edata.pilot and follower:exists() then
                speak(persona, "join", edata)
                follower:setLeader(edata.pilot)
            end
        end
    elseif n == 3 then
        edata.pilot:setLeader(player.pilot())
    end
end

function scav_hail(p, arg)
    -- Remove randomness from future calls
    if not mem.hailsetup then
        mem.refuel_base = mem.refuel_base or rnd.rnd(2000, 4000)
        mem.bribe_base = mem.bribe_base or math.sqrt(p:stats().mass) * (300 * rnd.rnd() + 850)
        mem.bribe_rng = rnd.rnd()
        mem.hailsetup = true
    end

    -- Clean up
    mem.refuel = 0
    mem.refuel_msg = nil
    mem.bribe = 0
    mem.bribe_prompt = nil
    mem.bribe_prompt_nearby = nil
    mem.bribe_paid = nil
    mem.bribe_no = nil

    -- Deal with refueling
    local standing = p:faction():playerStanding()
    mem.refuel = mem.refuel_base
    if standing > 60 then
        mem.refuel = mem.refuel * 0.5
    end
    mem.refuel_msg = fmt.f(_([["For you, only {credits} for a jump of fuel."]]), {credits = fmt.credits(mem.refuel)})

    -- Deal with bribeability
    mem.bribe = mem.bribe_base
    if mem.allowbribe or (mem.natural and mem.bribe_rng < 0.95) then
        mem.bribe_prompt = fmt.f(bribe_prompt_list[rnd.rnd(1, #bribe_prompt_list)], {credits = fmt.credits(mem.bribe)})
        mem.bribe_prompt_nearby = bribe_prompt_nearby_list[rnd.rnd(1, #bribe_prompt_nearby_list)]
        mem.bribe_paid = bribe_paid_list[rnd.rnd(1, #bribe_paid_list)]
    else
        mem.bribe_no = _([["You won't be able to slide out of this one!"]])
    end

    player.commClose()

    local edata = mem.persons[arg]
    scav_askUpgrade(edata, arg)
end
