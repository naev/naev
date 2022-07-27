local fmt = require "format"
local lang = require "language.language"

-- NOT READY!!


-- creates a copy of t1 and t2 joined together
local function join_tables(t1, t2)
	local copy = {}
	for _i, v in ipairs(t1) do
        table.insert(copy, v)
    end
    for _i, v in ipairs(t2) do
        table.insert(copy, v)
    end

    return copy
end


-- pick a random item from the collection
local function pick_one(target)
    local r = rnd.rnd(1, #target)
    return target[r]
end

-- pick a random key from a mapping
local function pick_key(mapping)
    local keys = {}
    for key, _value in pairs(mapping) do
        table.insert(keys, key)
    end

    local chosen_key = pick_one(keys)

    return chosen_key
end

local function getSpaceThing()
    -- just a bunch of things that you could find out in space
    return pick_one(lang.nouns.objects.space)
end

-- gets a random ship or possibly an alternative
local function getRandomShip()
    local ships = ship.getAll()

	-- minor sanitation
    local candidate = string.gsub(string.gsub(pick_one(ships):name(), "Drone \\(", ""), "\\)", "")

    -- don't allow thurion or proteron ships or outposts
    if string.find(candidate, "Thuri") or string.find(candidate, "Proter") or string.find(candidate, "Outpost") then
        -- give it some interesting choices along with some standard ones
        candidate = getSpaceThing()
    end

    return candidate
end

-- gets a random outfit
local function getRandomOutfit()
    local outfits = outfit.getAll()
    return pick_one(outfits):name()
end

-- TODO: if we have facilities on this ship, list those
-- generates a shipboard activity, loosely based on the ship that's being flown
local function getShipboardActivity( activity_type )
	local activities = {}
	-- basic activities: "I'm going to go <do/for [some]> <activity>"
	activities.basic = {
		_("exercise"),
		_("maintenance"),
		_("sanitation"),
		_("inspection"),
		_("thing"),
		_("hydration"),
		_("research"),
		_("science"),
		_("inventory"),
		_("project"),
		_("assignment"),
		fmt.f(_("{fruit} restocking"), { fruit = lang.getRandomFruit() } ),
		fmt.f(_("{fruit} tallying"), { fruit = lang.getRandomFruit() } ),
	}
	-- anyone wanna play some <game>?
	activities.game = join_tables(lang.nouns.activities.games, {
		lang.getMadeUpName(),
		_("squash")
	})
	if player.pilot():ship():size()  > 4 then
	-- these are "places to go" on the cruiser or larger where you go to do some cool activity
		activities.cruiser = lang.nouns.facilities.cruiser
	end
	if not activity_type then
		activity_type = pick_key(activities)
	elseif not activities[activity_type] then
		activity_type = "basic"
	end
	local choices = activities[activity_type]
	return pick_one(choices)
end

-- generate some random things, then picks one out of the hat or
-- (rarely) the name of the category it is from (could be funny)
local function getRandomThing()
    local things = {
        ["ship"] = getRandomShip(),
        -- TODO: generate these...
        ["item"] = pick_one(
            {
                "leather jacket",
                "vintage coat",
                "elegant design", -- okay, not really an item... but still
                "abstract holosculpture",
                "virtual death simulator",
                "synthetic snakeskin applicator",
                "high-quality lip stick",
                "white elephant",
                "red herring",
                "classic video game",
                "optical combustion device",
                "synthetic aquarium",
                "animal figurine",
                "paper plane",
                "telepathically controlled camera drone",
                "baseball bat",
                "baseball hat",
                "basketball",
                "wicker basket",
                "trojan",
                "sock puppet",
                "social network simulator",
                "vintage hand egg",
                "fictional literature",
                "device",
                "gadget",
                "hand-held",
                "portable",
                "Ultra 3000",
                "Neo 7000",
                "0K Elite Edition cup chiller",
                "wholesome book",
                "digital archive",
                "toy",
                "puppet",
                "sock"
            }
        ),
		["anything"] = pick_one(lang.getAll(lang.nouns)),
        ["outfit"] = getRandomOutfit(),
        ["thingymabob"] = pick_one(lang.getMadeUpName()),
        ["spacething"] = getSpaceThing(),
		["fruit"] = lang.getRandomFruit(),
        ["whatever"] = pick_one(lang.getMadeUpName()),
    }
    for key, thing in pairs(things) do
        if rnd.rnd() < 0.22 then
            return thing
        end
        -- interesting alternatives to orthotox flow
        if rnd.rnd() > 0.967 then
            return key
        end
    end

    return "thing"
end

local BASIC_PERSONALITY = function () return {
        -- what I say when I'm doing my job
        ["message"] = {
            _("I'm here."),
            _("I'm doing it."),
            _("I'm on it."),
			_("I'm working on it."),
			_("Stop pressuring me."),
			_("I've got it."),
			_("I'm on this."),
			_("I've got this."),
			_("I hope I got this right."),		
        },
        -- what I say when I'm satisfied
        ["satisfied"] = {
            _("I'm quite happy."),
            _("I am satisfied."),
            _("It's a good day."),
            _("I feel good."),
			_("Not the worst cycle, right?"),
			_("I've seen periods worse than this."),
			_("I'll survive."),
        },
        -- what I say when not satisfied
        ["unsatisfied"] = {
            _("I am unhappy."),
			_("I have been better."),
			_("I will survive."),
            _("The situation is bleak."),
            _("The atmosphere is unfriendly."),
            _("I'm bored."),
            _("I get bored sometimes."),
            _("There's nothing to do around here."),
            _("It's too quiet here."),
			_("The past periods will haunt me."),
			_("The last cycle was harsh."),
			fmt.f(_("The {fruit} situation on the ship could be better."), {fruit = lang.getRandomFruit() } ),
			fmt.f(_("I haven't seen a single {fruit} in cycles."), {fruit = lang.getRandomFruit() } ),
			fmt.f(_("I last saw maybe one {fruit} some periods ago or longer, I don't even remember anymore."), {fruit = lang.getRandomFruit() } ),
			fmt.f(_("I need my {fruit}s."), {fruit = lang.getRandomFruit() } ),
			fmt.f(_("There's never enough {fruit}s when I need one."), {fruit = lang.getRandomFruit() } ),
			fmt.f(_("I last saw maybe one {fruit}... I don't even remember anymore."), {fruit = lang.getRandomFruit() } ),
        },
        -- things I say about {name} when I had a good conversation
        ["good_talker"] = {
            _("You're a good listener, {name}."),
            _("{name} seems nice."),
            _("Right on!"),
            _("I agree with whatever {firstname} says."),
			_("I agree with whatever {article_subject} says."),
            _("I agree with {article_object}, with {name}."),
            _("You know {name}, you're alright."),
            _("Anyone want to give me a hand with this?"),
            _("Say, could you give me a hand with this?"),
            _("Let's go grab a drink with {firstname}."),
            _("Isn't {name} nice?"),
			_("Isn't {article_subject} nice?"),
            _("Isn't that {name} great?"),
            _("Isn't this just wonderful?"),
            _("Everything seems great."),
            _("I'm feeling positive."),
            _("I'm glad I had that talk with {name}."),
            _("I'm glad we had this talk."),
			_("I had a good conversation with {article_object}."),
        },
        -- things I say about {name} when I had a bad conversation
        ["bad_talker"] = {
            _("Come on, {name}."),
            _("{name}, don't be such a downer."),
            _("Brighten up, {name}."),
            _("Get yourself together {name}."),
            _("To hell with it, {article_subject}'s insane."),
            _("Yeah, okay."),
            _("Interesting."),
            _("Whatever..."),
            _("Whatever, jeez."),
            _("Oh come on {name}, don't start with me."),
            _("Not this again, {name}."),
            _("Is {article_subject} being offensive?"),
			_("I think {article_subject}'s having a bad day."),
			_("You'd think {article_subject}'d keep those thoughts to {article_object}self."),
			fmt.f(_("You should keep those thoughts to yourself, shouldn't {article_subject}, {captain}?"), {article_subject="{article_subject}", captain=player.name() } ),
			_("I don't know what {article_subject}'s on about."),
			_("What's gotten into {article_object}?"),
        },
        ["fatigue"] = {
            _("Are we going to get some time off anytime soon?"),
            _("It's really cold out here in space."),
            _("It's so lonely out here."),
            _("I could use a drink."),
            _("I could really use a drink."),
            _("I need a drink."),
            _("I could use a break."),
            _("I'm pretty tired."),
            _("I could use some rest."),
			_("Anyone want to play some cards?"),
			fmt.f(_("Anyone want to play some {game}?"), { game = getShipboardActivity("game") }),
			fmt.f(_("Anyone want to play {game}?"), { game = getShipboardActivity("game") }),
			fmt.f(_("Anyone want to play a game of {game}?"), { game = getShipboardActivity("game") }),
			fmt.f(_("I really want to play {game}."), { game = getShipboardActivity("game") }),
			fmt.f(_("Do I really have to do all that {basic}?"), { basic = getShipboardActivity("basic") }),
			fmt.f(_("Do I really have to do all that {basic}?"), { basic = getShipboardActivity("basic") }),
			fmt.f(_("Do I have to do the {basic}?"), { basic = getShipboardActivity("basic") }),
			fmt.f(_("Oh, I forgot that I have to do my {basic} for today."), { basic = getShipboardActivity("basic") }),
			fmt.f(_("I'm going for some {basic}."), { basic = getShipboardActivity("basic") }),
			fmt.f(_("I'm going to do that {basic} in a bit."), { basic = getShipboardActivity("basic") }),
			_("I hope someone can come cover for me soon, I'm getting a bit tired."),
			_("We've been in space for far too long, we need a break. A good one."),
			_("We need a break, and by break, I mean a big break with big rewards."),
			_("I thought it was all going to be action. But instead it's mostly fear, and a lot of nothing."),
			_("I can't wait for that drink with the crew when we land."),
			_("I wonder if the captain will mind if I take some time for myself at the next stop."),
			_("I haven't been feeling like myself lately."),
			_("Give me a break."),
			_("Give me a drink."),
			_("Get me a drink."),
			_("I need a sandwich or something."),
			_("Do we still have any fruit?"),
			_("Why does the water taste so stale?"),
			_("I can't believe we're out of bananas already, again!"),
			_("No bananas..."),
			_("Where are the bananas?"),
			fmt.f(_("Where are the {fruit}s?"), {fruit = lang.getRandomFruit() } ),
			fmt.f(_("No {fruit} today?"), {fruit = lang.getRandomFruit() } ),
			fmt.f(_("I saw there were some {fruit}s in the break room earlier but I didn't take one."), {fruit = lang.getRandomFruit() } ),
			fmt.f(_("I can't believe I didn't take the last {fruit}."), {fruit = lang.getRandomFruit() } ),
			fmt.f(_("I haven't seen a single {fruit} in cycles."), {fruit = lang.getRandomFruit() } ),
			fmt.f(_("I last saw maybe one {fruit} some periods ago or longer, I don't even remember anymore."), {fruit = lang.getRandomFruit() } ),
        },
        -- special things I know how to say
        ["special"] = {
            ["laugh"] = {
                _("*laughs*"),
                _("Heh."),
                _("*chuckles*"),
                _("Haha."),
                _("Right? *laughs*"),
                _("*laughs*")
            },
            ["hysteria"] = {
                _("I can't take it, I have to get out of here!"),
                _("Why won't the airlock open? I need to get out of here!"),
                _("Oh man, oh boy, oh sister..."),
                _("Oh brother..."),
                _("Wait, what was that?"),
                _("What's going on?"),
                _("What is that?"),
                _("Hello? Is there anybody there?"),
                _("What's that ringing sound?"),
                _("Oh no, space blindness, it actually happened!"),
                _("Why can't I see anything?"),
                _("Is that a floating sponge?")
            },
            ["worry"] = {
                _("I don't know..."),
                _("Oh, wait a minute..."),
                _("Did I get that right?"),
                _("At least I thought so."),
                _("If everything goes to plan."),
                _("Maybe."),
                _("I think."),
                _("I think..."),
                _("I'm pretty sure."),
                _("I hope so.")
            }
        },
        ["smalltalk_positive"] = {
            fmt.f(_("Do you guys remember that {thing} the other day?"), {thing = getRandomThing()}),
            _("Hey. Everything good?"),
            fmt.f(_("My {made_up} is acting up. I'd better go check on it"), {made_up = lang.getMadeUpName()}),
            _("I'll be in my quarters if you need me."),
            _("I'm going to go do inventory."),
            _("Let me know if you need anything."),
            _("I'm going to take stock in a bit."),
            _("Let me know if you need a hand with that thing later."),
            _("I'm going back to get on that maintenance task."),
            _("I'll go get on that maintenance task."),
            _("I'm about to go prime the locking equipment."),
            _("Do you need a hand with that?"),
            _("I could use a hand with this, do you mind?"),
            _("I could use a hand in the back, can you help me?"),
			_("You want a drink?"),
			_("You want this? I got two."),
			fmt.f(_("Would you like to play some {game}?"), { game = getShipboardActivity("game") }),
			fmt.f(_("I'll play some {game} if you want."), { game = getShipboardActivity("game") }),
			fmt.f(_("Would you like to have this {fruit}?"), { fruit = lang.getRandomFruit() }),
			fmt.f(_("I'll give you my {fruit} if you want it."), { fruit = lang.getRandomFruit() }),
			fmt.f(_("I just ate a really nice {fruit}."), { fruit = lang.getRandomFruit() }),
			fmt.f(_("Do you want a piece of this {fruit}?"), { fruit = lang.getRandomFruit() }),
			fmt.f(_("So, {fruit} anyone?"), { fruit = getShipboardActivity() }),
        },
        ["smalltalk_negative"] = {
            _("Sometimes I wonder what I'm even doing on this ship."),
            _("What am I even doing here?"),
            _("This life isn't as glamorous as it was made out to be."),
			_("The atmosphere here is killing me."),
			_("I feel like I am suffocating in here."),
			_("I don't know why I hang out with you."),
			_("You guys are the worst."),
			_("Your company is unappreciated."),
			_("Please stop leaning in my direction."),
			_("Please stop looking in my direction."),
			_("I don't want you to look at me right now."),
			_("I don't want to look at you right now."),
			_("Please stop talking to me."),
			_("Please don't say anything."),
			_("Don't say anything."),
			_("Please don't talk to me."),
			_("Don't talk to me."),
			_("Stay away from me."),
			_("Say nothing."),
			fmt.f(_("Don't say a {thing} thing!"), { thing = lang.getMadeUpName()}),
			fmt.f(_("What are you looking at, you {insult}?"), {insult = lang.getInsultingProperNoun()}),
        },
        -- things we say about things we are indifferent to
        ["default_participation"] = {
            _("Yeah, okay."),
            _("Interesting."),
            _("Great!"),
            _("That sounds good."),
            _("Yeah."),
            _("Nice."),
            _("Alright."),
            _("Cool."),
            _("Oh, that's news to me."),
            _("Oh, cool."),
            _("Oh, sweet."),
            _("I'm down."),
            _("Okay."),
            _("Sure."),
            _("Whatever."),
            _("I don't think so. I know so."),
            _("I think so.")
        },
        -- responses to conversations about topics I don't like
        ["phrases_disliked"] = {
            _("Do we have to talk about this?"),
            _("All you ever talk about is {topic}."),
            _("It's {topic} this, {topic} that, you just can't get enough {topic} can you?"),
            _("Whatever."),
            _("Yeah, okay."),
            _("Right."),
            _("Sure."),
            _("Yeah, because of all the {topic}, of course."),
            fmt.f(_("I'd rather talk about {made_up}s than {topic}."), {made_up = getRandomThing(), topic = "{topic}"}),
			fmt.f(_("Don't be such a {insult}."), {insult = lang.getInsultingProperNoun()}),
        }
    }
end

local PLEASER_PERSONALITY = function ()
	local bp = BASIC_PERSONALITY()
	bp.default_participation = {
		_("Of course."),
		_("Sure."),
		_("Yes."),
		_("Good talk."),
		_("Sounds good."),
		_("Yeah."),
		_("Nice."),
		_("Alright."),
		_("I'm a bit busy, but you can trust me."),
		_("I'll do what I can."),
		_("I always do my best."),
		_("I do my best."),
		_("I'll do my best for you."),
		_("I aim to please."),
		_("You can count on me."),
	}
	return bp
end


local PASSENGER_PERSONALITY = function ()
	local bp = BASIC_PERSONALITY()
	
	bp.smalltalk_positive = {
		fmt.f(_("Do you guys remember that {thing} the other day?"), {thing = getRandomThing()}),
		_("Hey. Everything good?"),
		fmt.f(_("My {made_up} is acting up. I'd better go check on it"), {made_up = lang.getMadeUpName()}),
		_("I'll be in the break room if you need me."),
		fmt.f(_("I'm going to go play some {game}."), { game = getShipboardActivity("game") } ),
		_("Let me know if you need a hand."),
		_("Let me know if you need a hand with that thing later."),
		_("Do you need a hand with that?"),
		_("I could use a hand with this, do you mind?"),
		_("I could use a hand with my stuff, can you help me?"),
		_("You want a drink?"),
		_("You want this? I got two."),
		fmt.f(_("Would you like to play some {game}?"), { game = getShipboardActivity("game") }),
		fmt.f(_("I'll play some {game} if you want."), { game = getShipboardActivity("game") }),
		fmt.f(_("Would you like to have this {fruit}?"), { fruit = lang.getRandomFruit() }),
		fmt.f(_("I'll give you my {fruit} if you want it."), { fruit = lang.getRandomFruit() }),
		fmt.f(_("I just ate a really nice {fruit}."), { fruit = lang.getRandomFruit() }),
		fmt.f(_("Do you want a piece of this {fruit}?"), { fruit = lang.getRandomFruit() }),
		fmt.f(_("So, {fruit} anyone?"), { fruit = getShipboardActivity("game") }),
	}
	-- passenger is fatigued, make it seem like he does stuff he shouldn't be doing
	bp.fatigue = {
		_("Are we going to get some time off anytime soon?"),
		_("It's really cold out here in space."),
		_("It's so lonely out here."),
		_("I could use a drink."),
		_("I could really use a drink."),
		_("I need a drink."),
		_("I could use a break."),
		_("I'm pretty tired."),
		_("I could use some rest."),
		_("Anyone want to play some cards?"),
		fmt.f(_("Anyone want to play some {game}?"), { game = getShipboardActivity("game") }),
		fmt.f(_("Anyone want to play {game}?"), { game = getShipboardActivity("game") }),
		fmt.f(_("Anyone want to play a game of {game}?"), { game = getShipboardActivity("game") }),
		fmt.f(_("I really want to play {game}."), { game = getShipboardActivity("game") }),
		fmt.f(_("Why is everyone busy doing {basic}?"), { basic = getShipboardActivity("basic") }),
		fmt.f(_("Do you really have to do all that {basic} all the time?"), { basic = getShipboardActivity("basic") }),
		fmt.f(_("Do I have to do the {basic}? Or is that just you guys"), { basic = getShipboardActivity("basic") }),
		fmt.f(_("Oh, I forgot that I have to do a little {basic} today."), { basic = getShipboardActivity("basic") }),
		fmt.f(_("I'm going for some {basic}. Can't hurt, right?"), { basic = getShipboardActivity("basic") }),
		_("I'm getting a bit tired of this long voyage."),
		_("I've been in space for far too long.."),
		_("I can't wait for that drink with the bunch later on."),
		_("I haven't been feeling like myself lately."),
		_("Give me a break."),
		_("Give me a drink."),
		_("Get me a drink."),
		_("I need a sandwich or something."),
		_("Do we still have any fruit?"),
		_("Why does the water taste so stale?"),
		_("I can't believe we're out of bananas already, again!"),
		_("No bananas..."),
		_("Where are the bananas?"),
		fmt.f(_("Where are the {fruit}?"), {fruit = lang.getRandomFruit() } ),
		fmt.f(_("No {fruit} today?"), {fruit = lang.getRandomFruit() } ),
		fmt.f(_("I saw there were some {fruit}s in the break room earlier but I didn't take one."), {fruit = lang.getRandomFruit() } ),
		fmt.f(_("I can't believe I didn't take the last {fruit}."), {fruit = lang.getRandomFruit() } ),
		fmt.f(_("I haven't seen a single {fruit} in cycles."), {fruit = lang.getRandomFruit() } ),
		fmt.f(_("I last saw maybe one {fruit} some periods ago or longer, I don't even remember anymore."), {fruit = lang.getRandomFruit() } ),
	}
	return bp
end

local YESMAN_PERSONALITY = function ()
	local bp = BASIC_PERSONALITY()
	bp.default_participation = {
		_("Of course."),
		_("Sure."),
		_("Yes."),
		_("Good talk."),
		_("Sounds good."),
		_("Yeah."),
		_("Nice."),
		_("Alright."),
		_("I've got all the time in the world for you!"),
		_("I'll do what I can."),
		_("I always do my best."),
		_("I'll do my best."),
		_("Absolutely."),
		_("Sure!"),
		_("Yes!"),
		_("Good talk!"),
		_("Sounds fantastic!"),
		_("Yeah!"),
		_("Nice!"),
		_("Alright!"),
		_("You can trust me."),
		_("I'll do what I can as always!"),
		_("I always do my best."),
		_("I do my best."),
		_("I'll do my best for you."),
		_("I aim to please."),
		_("You can count on me."),
	}
	bp.special.hysteria = {
		_("The pressure might be getting to me."),
		_("The workload is getting to me."),
		_("The crew can be hard to manage sometimes."),
		_("There's always a troublemaker in the bunch."),
		_("Wait, what was that?"),
		_("What's going on over here?"),
		_("What is that over there?"),
		_("There's too much going on."),
		_("My ears are ringing."),
		_("I need that drink."),
		_("I could really use a drink right about now."),
		fmt.f(_("Is that a floating {thing}?"), { thing = getSpaceThing() }),
	}
	-- we try to be overly positive no matter what
	bp.special.worry = {
		_("I'm sure there's nothing to worry about."),
		_("Nothing that a drink won't fix."),
		_("Nothing that a little motivation won't fix."),
		_("Nothing that a paradise world won't fix."),
		_("Nothing that a vacation won't fix."),
		_("Nothing that a good payday won't fix."),
		_("I'm sure everything will be alright."),
		_("Everything is fine."),
		_("Everything seems fine to me."),
		_("Things are fine."),
		_("Everything is going to be fine."),
		_("Everything is going to be alright."),
		fmt.f(_("Right {name}?"), {name = player.name()} ),
	}
	-- forced laughter in captain's company
	table.insert(bp.special.laugh, _("*laughs* Right, captain?"))
	table.insert(bp.special.laugh, _("*exaggerated laughter*"))
	table.insert(bp.special.laugh, _("*overbearing laughter*"))
	table.insert(bp.special.laugh, _("*snorting laughter*"))
	return bp
end

local conversation = {}

conversation.basic		= BASIC_PERSONALITY
conversation.passenger	= PASSENGER_PERSONALITY
conversation.pleaser	= PLEASER_PERSONALITY
conversation.yesman		= YESMAN_PERSONALITY

return conversation