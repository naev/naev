--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Companion Handler2">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
--[[

   Escort Handler Event

   This event runs constantly in the background and manages escorts
   hired from the bar, including generating NPCs at the bar and managing
   escort creation and behavior in space.

--]]
local fmt = require "format"
local portrait = require "portrait"
local pir = require "common.pirate"
local pilotname = require "pilotname"
local vntk = require "vntk"

local npcs  -- Non-persistent state

local logidstr = "log_shipcompanion"

mem.conversation_hook = nil

local entries = {
    ["demoman"] = function(speaker)
        return hook.board("player_boarding_c4", speaker)
    end,
    ["escort"] = function(speaker)
        return hook.land("escort_landing", "land", speaker)
    end
}

local prices = {
    ["equipment"] = 15
}

-- merges (overwrites) a template table t1 with data from t2
local function merge_tables(t1, t2)
    for k, v in pairs(t2) do
        t1[k] = v
    end

    return t1
end

local function append_table(t1, t2)
    for _i, v in ipairs(t2) do
        table.insert(t1, v)
    end

    return t1
end

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

-- picks out a list nested in a table that uses keys instead of numeric indexing
-- used on the conversation object
-- e.g. in topic -> "faith"
-- or    special -> "laugh"
local function pick_map(maptable)
    -- as per example above, "pick the topic"
    local chosen_key = pick_key(maptable)
    -- pick out the list that we can choose from
    local chosen_value = maptable[chosen_key]
    return chosen_value
end

-- picks one element out of a list that is nested inside a table
-- used on the conversation object
-- e.g. in topic -> "faith" -> picks one of phrases
-- or    special -> "laugh" -> picks one of laughters
local function pick_from_map(maptable)
    -- pick one out of the chosen list
    local choices = pick_map(maptable)
    return pick_one(choices)
end

-- pick a random letter out of a string
local function pick_str(str)
    local ii = rnd.rnd(1, str:len()) -- pick a random index from string length
    return string.sub(str, ii, ii) -- returns letter at ii
end

-- for adding a special kind of speech into an utterance
local function add_special(speaker, kind)
    local specials = speaker.conversation.special
    if not specials then
        return ""
    end
    local choice = pick_key(specials)
    if kind and specials[kind] then
        choice = kind
    end
    -- we have something like "laugh" and need to get the list inside
    local options = specials[choice]

    return pick_one(options)
end


local function getSpaceThing()
    -- just a bunch of things that you could find out in space
    return pick_one(
        {
            "drama Llama",
            "Shuttle",
            "Kangaroo",
            "zippy Quicksilver",
            "Koala",
            "Derelict",
            "weird sponge thing",
            "swanky Gawain",
            "cube",
            "hunk of junk",
            "asteroid",
            "meteor",
            "comet",
            "planet",
            "moon",
            "star",
            "space truck",
            "cargo container",
            "battleship",
            "cruiser",
            "spooky frigate",
            "zooming corvette",
            "suicidal fighter",
            "murderous drone"
        }
    )
end

-- gets a random ship or possibly an alternative
local function getRandomShip()
    local ships = ship.getAll()

    local candidate = string.gsub(string.gsub(pick_one(ships):name(), "Drone \\(", ""), "\\)", "")

    -- don't allow thurion or proteron ships
    if string.find(candidate, "Thuri") or string.find(candidate, "Proter") then
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
-- generates a short, made-up capitalized Noun with limited imagination, usually 2-3 syllables or around 7 letters
local function getMadeUpName()
    -- bias some letters to introduce slight consistency for a little depth
    local start = "BCCDDDFGGHJKKLMMNNPPRRRSTTVWX"
    local middle = string.lower(start)
    local vowel = "aeiouy"

    local middle_part = pick_str(middle) .. pick_str(middle)
    if rnd.rnd(0, 1) == 1 then
        -- add an extra syllable
        middle_part = middle_part .. pick_str(vowel) .. pick_str(middle)
        -- maybe add an extra letter
        if rnd.rnd(0, 1) == 0 then
            middle_part = middle_part .. pick_str(middle)
        end
    end

    -- something like Gartok or Termengix or whatever
    return pick_str(start) .. pick_str(vowel) .. middle_part .. pick_str(vowel) .. pick_str(middle)
end


local function getRandomFruit()
	-- must end with an s in plural in English, at least how it's being used now
	local fruits = {
		_("banana"),
		_("apple"),
		_("pear"),
		_("grape"),
		_("orange"),
		_("cantalope"),
		_("melon"),
		_("lemon"),
		_("lime"),
		getMadeUpName()
	}
	
	return pick_one(fruits)
end

-- generates a shipboard activity, loosely based on the ship that's being flown
local function getShipboardActivity( activity_type )
	local activities = {}
	-- basic activities: "I'm going to go <do/for [some]> <activity>"
	activities.basic = {
		_("exercise"),
		_("maintenance"),
		_("sanitation"),
		_("inspection"),
		_("rounds"),
		_("hydration"),
		_("inventory"),
		fmt.f(_("{fruit} restocking"), { fruit = getRandomFruit() } ),
	}
	-- anyone wanna play some <game>?
	activities.games = {
		_("cards"),
		_("chess"),
		_("tic-tac-toe"),
		_("blackjack"),
		_("21"),
		getMadeUpName(),
	}
	if player.pilot():ship():size()  > 4 then
	-- these are "places to go" on the cruiser or larger where you go to do some cool activity
		activities.cruiser = {
			_("theater room"),
			_("virtual experience simulator"),
			_("mess hall"),
			_("recreation room"),
			_("gym"),
			_("spa"),
			_("sauna"),
		}
	end
	if not activity_type then
		activity_type = pick_key(activities)
	elseif not activities[activity_type] then
		activity_type = basic
	end
	return pick_one(activities[activity_type])
end

-- returns some kind of statement describing an insulting proper noun
-- i.e. something you would say about a ship or person that's in disorder
local function getInsultingProperNoun()
    -- things that you can put in the structured syntax and get away with
    -- "smelly smelly momma's boy sniffer" is a perfectly good insult, for instance
    -- as would be "dortetak jarryk Gorkok lebbeler!" because who knows where this person gets their insults from
    local adject2s = {
        _("smelly"),
        _("dirty"),
        _("rusty"),
        _("dated"),
        _("faded"),
        _("outdated"),
        _("expired"),
        _("deprecated"),
        _("overappreciated"),
        _("rotten"),
        _("slow,"), -- that comma is not a typo! it's for the emphasis on the slowness, like a person would do
        _("untrustworthy"),
        _("thieving"),
        _("tiny"),
        _("fat"),
        _("ugly"),
        _("insane"),
        _("senile"),
        _("angry"),
        _("dubious"),
        _("insufferable"),
        _("devious"),
        _("repugnant"),
        getMadeUpName():lower()
    }
    local adjectives = {
        _("old"),
        _("boring"),
        _("inglorious"),
        _("irrational"),
        _("stupid"),
        _("daft"),
        _("thick"),
        _("little"),
        _("demoralizing"),
        _("bloody"),
        _("blue"),
        _("boring"),
        _("ugly"),
        _("fat"),
        _("juice thirsty"),
        _("thirsty"),
        _("bloodthirsty"),
        _("insufferable"),
        getMadeUpName():lower()
    }
    local nouns = {
        _("momma's boy"),
        _("demon"),
        _("angel"),
        _("ass"),
        _("donkey"),
        _("butt"),
        _("fool"),
        _("mouth breather"),
        _("daddy's little angel"),
        _("terrorist"),
        _("violator"),
        _("jerk"),
        _("hat"),
        _("hat stand"),
        _("coat rack"),
        _("miner"),
        _("scientamologist"), -- an idiot saying "scientist" or someone being sarcastic to one
        _("cargo container"),
        _("hyena"),
        _("brick"),
        _("satan"),
        _("shit"),
        _("bucket"),
        _("square"),
        _("child"),
        _("dinosaur"),
        _("mule"),
        _("rhinoceros"),
        _("kangaroo"),
        _("bear"),
        _("sunken dream"),
        _("pirate"), -- insane boring pirate killer... sure, why not!
        _("prawn"),
        _("lobster"),
        _("dishwasher"),
        _("planet"),
        _("seed"),
        _("weed"),
        _("seaweed"),
        _("paper"),
        _("asteroid"),
        _("cinderblock"),
        _("clock"),
        _("hammock"),
        _("rock"),
        _("sock"),
        _("wrench"),
        _("document"),
        getMadeUpName()
    }
    -- these all get the '-er' suffix and don't have to be real, they are used by angry people who say strange things
    local verbs = {
        _("sniff"),
        _("smell"),
        _("lov"),
        _("eat"),
        _("lick"),
        _("huff"),
        _("idoliz"),
        _("smok"),
        _("slay"),
        _("crav"),
        _("sympathiz"),
        _("glorializ"),
        _("slay"),
        _("slay"),
        _("munch"),
        _("slap"),
        _("spit"),
        _("lurk"),
        _("fil"), -- lol, you FILER! LOL'd at IRL for real.
        _("kiss"),
        _("kill"),
        getMadeUpName():lower()
    }

    local params = {}
    params.noun = pick_one(nouns)
    params.adjective = pick_one(adjectives)
    params.adject2 = pick_one(adject2s)
    params.verb = pick_one(verbs)
    local r = rnd.uniform()
    if r < 0.33 then
        return fmt.f("{adjective} {noun}", params)
    elseif r < 0.5 then
        return fmt.f("{noun}", params)
    elseif r < 0.66 then
        return fmt.f("{adject2} {adjective} {noun}", params)
    elseif r < 0.88 then
        return fmt.f("{adjective} {noun} {verb}er", params)
    else
        return fmt.f("{adject2} {adjective} {noun} {verb}er", params)
    end
end

local function getRandomThing()
    -- generate some random things, then pick one
    local things = {
        ["ship"] = getRandomShip(),
        ["outfit"] = getRandomOutfit(),
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
        ["whatever"] = pick_one(getMadeUpName()),
        ["thingymabob"] = pick_one(getMadeUpName()),
        ["spacething"] = getSpaceThing()
    }
    for key, thing in pairs(things) do
        if rnd.rnd() < 0.22 then
            return thing
        end
        -- interesting alternatives to orthotox flow
        if rnd.rnd() > 0.97 then
            return key
        end
    end

    return "thing"
end

local function getBarSituation(character_sheet)
    local bar_actions = {
        {
            ["verb"] = pick_one(
                {
                    _("swirling"),
                    _("sipping"),
                    _("drinking"),
                    _("enjoying"),
                    _("nursing"),
                    _("nursing on")
                }
            ),
            ["descriptor"] = pick_one(
                {
                    _("a"),
                    _("a"),
                    _("some"),
                    _("some kind of")
                }
            ),
            ["adjective"] = pick_one(
                {
                    _("nice"),
                    _("strange"),
                    _("hot"),
                    _("cold"),
                    _("chilled"),
                    _("colourful"),
                    _("warm")
                }
            ),
            ["object"] = pick_one(
                {
                    _("drink"),
                    _("wine"),
                    _("tea"),
                    _("concoction"),
                    _("elixir"),
                    _("mixture of fluids"),
                    _("beverage")
                }
            )
        },
        {
            ["verb"] = pick_one(
                {
                    _("thinking"),
                    _("pondering"),
                    _("wondering"),
                    _("having feelings")
                }
            ),
            ["descriptor"] = pick_one(
                {
                    _("about some"),
                    _("about a"),
                    _("about that")
                }
            ),
            ["adjective"] = pick_one(
                {
                    _("nice"),
                    _("strange"),
                    _("shady"),
                    _("cold"),
                    _("eerie"),
                    _("colourful"),
                    _("distracting")
                }
            ),
            ["object"] = pick_one(
                {
                    _("drink"),
                    _("person"),
                    _("sculpture"),
                    _("plant"),
                    _("piece of machinery"),
                    _("bartender"),
                    _("smell")
                }
            )
        },
        {
            ["verb"] = pick_one(
                {
                    _("looking"),
                    _("peering"),
                    _("squinting"),
                    _("eyeing")
                }
            ),
            ["descriptor"] = pick_one(
                {
                    _("at"),
                    _("towards"),
                    _("in the direction of"),
                    _("vaguely towards")
                }
            ),
            ["adjective"] = pick_one(
                {
                    _("some"),
                    _("a"),
                    _("a mysterious"),
                    _("an unsuspicious"),
                    _("a suspicious looking"),
                    _("an anonymous"),
                    _("another")
                }
            ),
            ["object"] = pick_one(
                {
                    _("drink"),
                    _("patron"),
                    _("person"),
                    _("area"),
                    _("stranger"),
                    _("crowd"),
                    _("group")
                }
            )
        },
        {
            -- an irregular one that adds anxiety to the crew
            ["verb"] = pick_one(
                {
                    _("anxious"),
                    _("desperate"),
                    _("poised"),
                    _("ready")
                }
            ),
            ["descriptor"] = _("to"),
            ["adjective"] = pick_one(
                {
                    _("get back to"),
                    _("return to"),
                    _("retreat to"),
                    _("abscond with")
                }
            ),
            ["object"] = pick_one(
                {
                    _("the ship"),
                    _("a drink"),
                    fmt.f(_("the {ship}"), {ship = player.pilot():name()}),
                    _("some food")
                }
            )
        }
    }
    if character_sheet and character_sheet.conversation and character_sheet.conversation.bar_actions then
        bar_actions = append_table(bar_actions, character_sheet.conversation.bar_actions)
    end

    local chosen_action = pick_one(bar_actions)
    local doing = fmt.f(_("{verb} {descriptor} {adjective} {object}"), chosen_action)
    return doing
end

-- quick wrapper that makes sure our sentiments haven't just been cleared
local function insert_sentiment(character, sentiment)
    if not character.conversation.sentiments then
        character.conversation.sentiments = {}
    end

    table.insert(character.conversation.sentiments, sentiment)
end

-- creates a memory of a certain kind, or a random memory
local function create_memory(character, memory_type, params)
    local topic = "random"
    local new_memory
    if memory_type == "payoff" then
        -- simple case, we got paid, this is a credits memory
        topic = "credits"
        local choices = {
            _("I made {credits} on one of my trips to {planet}."),
            _("I made {credits} on our voyage through {system}."),
            _("I'd like to go to {planet} again sometime.")
        }
        new_memory = pick_one(choices)
    elseif memory_type == "specific" then
        -- we are just learning how to say a new sentence, which should be in our params
        -- NOTE: This isn't even being used yet
        new_memory = params.specific or tostring(params)
    elseif memory_type == "friend" then
        -- learn about the friend by fetching info from params, which must be a character sheet
        -- hopefully these will become catalysts for further conversation
        local actions = {
            _("I witnessed {name} "),
            _("I was with {name} while {article_subject} was "),
            _("Was {name} really "),
            _("I saw {name} "),
            _("I noticed that {name} was ")
        }
        local when = {
            _(" earlier."),
            _(" the other day."),
            _(" at the bar."),
            _(" in the break room."),
            _(" some time ago."),
            _(" recently."),
            _(", but it feels like ages ago.")
        }
        local choices = {
            -- some random memories from the bar
            pick_one(actions) .. getBarSituation(params) .. pick_one(when),
            pick_one(actions) .. getBarSituation(params) .. pick_one(when) .. " " .. add_special(character),
            add_special(character) .. " " .. pick_one(actions) .. getBarSituation(params) .. pick_one(when),
            -- remembering that they like some topic (or dislike)
            pick_one(actions) ..
                " talking a lot about things like the " .. pick_key(params.conversation.topics_liked) .. " or whatever.",
            pick_one(actions) .. " talking about the " .. pick_key(params.conversation.topics_liked) .. " or whatever.",
            pick_one(actions) ..
                " expressing concern when the conversation was focused on the " ..
                    pick_key(params.conversation.topics_disliked) .. ".",
            pick_one(actions) ..
                " expressing concern when the conversation was focused on the " ..
                    pick_key(params.conversation.topics_disliked) .. pick_one(when),
            -- remembering that we had a special moment in this solar system
            fmt.f(_("{name} and I had a special moment in {system}."), {name = params.firstname, system = system.cur()}),
            fmt.f(_("I had a nice time with {name} in {system}."), {name = params.name, system = system.cur()})
        }
        local choice = pick_one(choices)
        -- if we're asking a question, let's fix the punctuation real quick
        if string.find(choice, "Was") then
            choice = string.gsub(choice, ".", "?", 1)
        end

        topic = "friend"
        new_memory = choice
    elseif memory_type == "animosity" then
        -- generate a short insulting phrase
        local starts = {
            "{name}?",
            _("{name}? That"),
            _("That"),
            _("Oh that"),
            _("I should tell you that {name} is nothing but a")
        }
        -- make sure we have a name
        local name = params.name or getMadeUpName()
        local start = fmt.f(pick_one(starts), name)
        local insult = getInsultingProperNoun()
        local punctuation =
            pick_one(
            {
                ".",
                "!",
                "...",
                "!!",
                _("- *sigh*."),
                _(". Yeah. I said it.")
            }
        )
        -- this is an aggressive thought, let's classify it as violence
        topic = "violence"
        new_memory = fmt.f("{start} {insult}{punctuation}", {start = start, insult = insult, punctuation = punctuation})
    elseif memory_type == "violence" then
        -- this is a violence memory, let's classify it as such
        topic = "violence"
        -- we destroyed a ship, a few generic options, a few specific options,
        -- and a couple of "wow, we sure do a lot of <param>"
        local choices = {
            -- insanity
            _("Do you remember that {ship} we got in {system}? It had {credits} in it."),
            _("Man, that {ship}. I still keep thinking about it."),
            _("That {target} never stood a chance."),
            _("Do you remember that {target} we got in {system}? It had {credits} in it."),
            _("Do you remember the {ship}?"),
            _("Do you remember that {target}?"),
            _("Do you remember the {ship}}? The one with {credits} in it."),
            _("I still keep thinking about that {ship} we got in {system}.")
        }
        -- if we got lots of credits
        if params.cred_amt > 25e3 then
            -- wow, that's a lot, actually let's overwrite the default choices
            if params.cred_amt > 187e3 then
                choices = {
                    _("I'll never forget that {ship} in {system}."),
                    _("{credits} is a lot of credits."),
                    _("{system} is actually one of my favourite places to visit."),
                    _(
                        "My favourite ship to board is the {ship}. Well, I can have many favourites, but that's a top contender for sure."
                    ),
                    _("That {target} never stood a chance."),
                    _("That {target} was in the wrong neighborhood, that's for sure."),
                    _("That I don't know what a {ship} like that was doing in {system} with {credits} on board."),
                    _("I still can't stop thinking about that {ship}!")
                }
                -- small chance of a memorable event
                if character.last_kill and character.last_kill == params.ship then
                    table.insert(
                        choices,
                        "{ship} are incredibly lucrative targets! We got a hundreds of thousand from just a few of them!"
                    )
                end
                -- more likely to mention it in a generic manner soon
                insert_sentiment(character, fmt.f(pick_one(choices), params))
                -- this was a lot of credits, this is now a memory about the credits
                topic = "credits"
            end
            -- okay, so if it was a lot we'll still have some other choices, but let's add everything here that's "decent"
            table.insert(choices, _("I like getting big bounties."))
            table.insert(choices, _("Does anyone want to travel to {system} and pray for some credits?"))
            if params.cred_amt > 50e3 then
                table.insert(choices, _("We should hunt more {ship}s."))
                table.insert(choices, _("We should keep hunting {ship}s."))
                table.insert(choices, _("That {ship} we got once had {credits} in it."))
                table.insert(choices, _("Maybe we should go back to {system} to find more {ship}s."))
                table.insert(choices, _("I think we should take a little visit to {system}."))
                -- if we just killed something like this
                if character.last_kill and character.last_kill == params.ship then
                    table.insert(choices, _("We sure like to go after those {ship}s."))
                    table.insert(choices, _("I feel like we are always hunting {ship}s."))
                    table.insert(choices, _("I feel like the only ships we bother with anymore are {ship}s."))
                    table.insert(
                        choices,
                        fmt.f(
                            _("That {ship} was nothing but a {spacething} and its captain a {spacefool}."),
                            {ship = params.ship, spacething = getSpaceThing(), spacefool = getInsultingProperNoun()}
                        )
                    )
                end
                -- let's remember this kill more than usual
                character.last_kill = params.ship
                insert_sentiment(character, fmt.f(pick_one(choices), params))
            end
            -- hopefully we'll get a chance to talk about this, maybe multiple times
            insert_sentiment(character, fmt.f(pick_one(choices), params))
            -- the above is a wrapper that makes sure the table exists, it definitely exists now, don't use the wrapper
            table.insert(character.conversation.sentiments, fmt.f(pick_one(choices), params))
            table.insert(character.conversation.sentiments, fmt.f(pick_one(choices), params))
        end

        new_memory = pick_one(choices)
    elseif memory_type == "hysteria" then
        -- we're having a nervous breakdown and are about to have an unreal experience
        local choices = {
            _("Man, what a {ship], that {target} was a complete waste of {credits}. Am I right?"),
            add_special(character) .. " " .. getMadeUpName() .. " " .. add_special(character, "laugh") .. ".",
            add_special(character) .. "... " .. add_special(character) .. "... " .. add_special(character) .. ".",
            getMadeUpName() .. " " .. getMadeUpName() .. " " .. add_special(character) .. ".",
            _("Why are we traveling to {system} again? I'd much rather go to {ship}."),
            _("Can't you see it out there? Tell me you can see it? You can see it can't you?"),
            _("Tell me you just saw that, I'm not the only one that just saw that, right?"),
            _("{target}na{target} {ship}ra{target} {system}{ship}{system}da..."),
            _("Why does this say our armour is at {armour} percent? These are percentages right?"),
            _("Why does this say our armour is at {armour} percent? Is that right?"),
            _("Why does this say our armour is at {armour} percent?"),
            -- scary thoughts
            _("This is all too real. I want to go home."),
            _("Sometimes I regret signing up for this."),
            _("I didn't sign up for this."),
            _("When did I sign up for this?"),
            _([[This isn't what I meant when I said "hang my hat"...]]),
            _(
                "Sometimes I can't get the thought out of my head... It's just this thin piece of hull between us and nothingness for distances so vast you can't even realistically imagine."
            ),
            -- aggressive ramblings and antisocial tendencies develop
            _("Don't you look at me like that! Don't do it!"),
            _("Hey, watch it!"),
            _("I saw that."),
            _("I know you're about to bring it up, I see that look in your eyes, quit it!"),
            _("Are you looking at me?"),
            _("Maybe all the violence has been getting to me."),
            _("Maybe all the violence is starting to get to me."),
            -- poignant moment such as remembering an old friend or having a epiphany
            fmt.f(_("I miss that rascal {name}. A true friendship never really ends."), {name = pilotname.human()}),
            fmt.f(_("I used to have a friend called {name}."), {name = pilotname.human()}),
            fmt.f(_("I miss my old friend {name}."), {name = getMadeUpName()}),
            _("You know, the best ship is friendship."),
            _(
                "I used to dream of sailing on a pirate ship as a child. Unfortunately, we don't live in that kind of fairy tale world. I did my time on a pirate ship in space, but I realized that what I really wanted was to feel the salty sea air blow across my face and fling my hair into the air."
            ),
            _("Maybe I shouldn't be so closed off about my affairs."),
            _("Maybe I shouldn't be so defensive about my affairs."),
            _("I can be a bit evasive when it comes to my affairs."),
            -- borrowed bits to conform with other memories
            _("That {ship} we got once had {credits} in it."),
            _("Maybe we should go back to {system} to find more {ship}s."),
            _("I think we should take a little visit to {system}."),
            _("I was just doing a routine {ship} inspection. I swear."),
            _("I still have scary thoughts about {system}."),
            _("Do you remember when we almost got stranded in {system}?"),
            _("We should probably avoid {system}.")
        }

        -- have some random thoughts
        insert_sentiment(character, fmt.f(pick_one(choices), params))
        table.insert(character.conversation.sentiments, fmt.f(pick_one(choices), params))
        table.insert(character.conversation.sentiments, fmt.f(pick_one(choices), params))
        -- pick the memory
        new_memory = pick_one(choices)
    else
        -- create a random memory about what we know about
        if not params then
            params = {}
        end
        -- we might need the system, let's make sure we have that
        params.system = params.system or system.cur()

        local pso = player.pilot():outfits()

        -- a bunch of random memories that will start to sound repetitive eventually and cause the crew member to seem senile
		-- TODO: Generate a completely unique memory
        local choices = {
            _("I had a thought in {system} but I forgot it."),
            _("I still think about {system} sometimes."),
            fmt.f(
                _("I think there's an issue with our {outfit}, I'm going to go and check it out."),
                {outfit = pick_one(pso)}
            ),
            fmt.f(_("I was just doing a routine {outfit} inspection. I swear."), {outfit = pick_one(pso)}),
            fmt.f(
                _("Before you go there, I don't want to hear about the {outfit} or the {system} story."),
                {outfit = pick_one(pso), system = "{system}"}
            ),
            _("I see that look you're giving me! I was there too you know, let's just drop it!"),
            fmt.f(_("Oh {swear}, not this again, can we just drop it?"), {swear = getMadeUpName()}),
            _("I'm getting real tired of getting all these looks."),
            fmt.f(_("I'll never forget the flight of {shipname}."), {shipname = player.pilot():name()})
        }

        -- check if this system has places we can refuel
        local can_refuel = false
        for key, spob in ipairs(params.system:spobs()) do
            if spob:canLand() and spob:services()["refuel"] then
                can_refuel = true
            end
        end

        -- check if we are scarily low on fuel and there are no refueling places
        if player.pilot():stats().fuel <= 100 and not can_refuel then
            topic = "travel"
            table.insert(
                choices,
                fmt.f(
                    _("I'll never forget the feeling of looking at the gauge and seeing the value read out as {fuel}."),
                    {fuel = player.pilot():stats().fuel}
                )
            )
            table.insert(choices, _("I thought we were going to die in {system}."))
            table.insert(choices, _("I still have scary thoughts about {system}."))
            table.insert(choices, _("Do you remember when we almost got stranded in {system}?"))
            table.insert(choices, _("We should probably avoid {system}."))
            -- random chance for this stressful situation to classify this memory as fear
            if rnd.rnd() < 0.16 then
                topic = "fear"
            end
        end

        -- see if some irrational fear kicks in and we become slightly traumatized
        if player.pilot():health() < 96 then
            local stressors = {_("fear"), _("death"), _("weapon"), _("shield")}
            topic = pick_one(stressors)
            -- we might start disliking this topic as well now
            if not character.conversation.topics_disliked[topic] then
                -- if we like violence or if we have high xp, we resist disliking the stressor
                if character.conversation.topics_liked.violence and rnd.rnd() > character.xp * character.satisfaction then
                    table.insert(character.conversation.topics_disliked, topic)
                elseif rnd.rnd() * 2 > character.xp * character.satisfaction then
                    table.insert(character.conversation.topics_disliked, topic)
                end
            end
        end

        -- check if we are low on armour
        if player.pilot():health() < 35 then
            -- this is now a violent memory
            topic = "violence"
            table.insert(choices, _("I thought we were going to die in {system}."))
            table.insert(choices, _("I still have scary thoughts about {system}."))
            table.insert(choices, _("Do you remember when we almost got sucked into space that time in {system}?"))
            table.insert(choices, _("We should probably avoid {system}."))
            if player.pilot():health() < 10 then
                -- actually, let's remember this specific system because that's pretty scary
                topic = system.cur():name()
            end
        end

        new_memory = pick_one(choices)
    end

    -- whether we actually like this or not (check dislikes first), this is now a topic we bring up
    if not character.conversation.topics_liked[topic] then
        character.conversation.topics_liked[topic] = {}
    end

    -- if we have memories of this system, put it there instead so we keep talking about it but don't always say the same thing
    local csn = system.cur():name()
    if character.conversation.topics_liked[csn] then
        topic = csn
    end

    print("created memory for about", fmt.f(new_memory, params), character.name, topic)
    -- insert the memory
    table.insert(character.conversation.topics_liked[topic], fmt.f(new_memory, params))

    -- have a chance to talk about this memory (think about this memory soon)
    insert_sentiment(character, fmt.f(new_memory, params))

    -- we created a memory, that gives us some experience
    character.xp = math.min(100, character.xp + 0.01)
end

local function has_interest(character, interest)
    for topic, _phrases in pairs(character.conversation.topics_liked) do
        if topic == interest then
            return true
        end
    end

    return false
end

function speak_notify(speaker)
    local name = speaker.nick or speaker.name
    local message = pick_one(speaker.conversation.message)

    if speaker.conversation.special and rnd.rnd(0, 1) == 1 then
        message = message .. " " .. add_special(speaker)
    end

    pilot.comm(name, message, "F")
end

-- records an interaction about one of reactor's disliked topics brought up by offender
-- TODO: use the topic argument to create a memory like "offender talks about topic too much"
local function dislike_topic(topic, reactor, offender)
    -- reactor doesn't like offender as much, decrease satisfaction and record a sentiment
    reactor.satisfaction = reactor.satisfaction - 0.03

    -- this is a low priority sentiment, use the table instead of the head
    insert_sentiment(reactor, fmt.f(pick_one(reactor.conversation.bad_talker), offender))

    -- try to create a memory about this event
    if rnd.rnd() < reactor.chatter then
        create_memory(reactor, "animosity", {name = offender.name})
    end
end

-- returns the keyword that evaluator didn't want to hear
local function dislikes_phrase(phrase, evaluator)
    for disgust, _phrases in pairs(evaluator.conversation.topics_disliked) do
        if string.find(phrase, disgust) then
            return disgust
        end
    end

    return false
end

-- returns the keyword that evaluator is interseted in
local function doeslike_phrase(phrase, evaluator)
    for interest, _phrases in pairs(evaluator.conversation.topics_liked) do
        if string.find(phrase, interest) then
            return interest
        end
    end

    return false
end

-- returns a list of appropriate responses and the topic that sparked interest
local function appreciate_spoken(spoken, appreciator)
    -- first check if we dislike something
    local disgust = dislikes_phrase(spoken, appreciator)
    if disgust then
        return appreciator.conversation.topics_disliked[disgust], disgust
    end

    -- check if we are do like something
    local interest = doeslike_phrase(spoken, appreciator)
    if interest then
        return appreciator.conversation.topics_liked[interest], interest
    end

    -- we don't care about this at all
    return appreciator.conversation.default_participation, nil
end

-- attempt to generate a list of valid responses based on what was said
local function generate_responses(spoken, crewmate)
    local responses, topic = appreciate_spoken(spoken, crewmate)

    -- this is a topic we like talking about or that we dislike
    if topic then
        return responses
    end

    -- try to figure out if there is something that looks like a question or doubt that we can agree/disagree with
    local doubters = {
        _("who"),
        _("what"),
        _("why"),
        _("when"),
        _("know"),
        _("sure")
    }

    for _, doubt in ipairs(doubters) do
        if string.find(spoken, doubt) then
            return crewmate.conversation.default_participation
        end
    end

    -- we still don't know what this is, generate some responses about our irritation
    local imaginary_topic =
        pick_one(
        {
            _("*inaudible*"),
            _("*expletive*"),
            getMadeUpName() -- pick_word(spoken) -- TODO: implement a smart word picker
        }
    )

    -- are we being shouted at? If so, we can use that as a topic
    if spoken == spoken:upper() then
        table.insert(responses, _("There's no need to shout.")) -- it's funny because there are dialog lines about being hard of hearing
        imaginary_topic = "shouting"
    end

    for _i, phrase in ipairs(crewmate.conversation.phrases_disliked) do
        table.insert(responses, fmt.f(phrase, {topic = imaginary_topic}))
    end

    return responses
end

-- calculate the interaction between starter and partaker
-- note that starer can by anyone here, even the player
-- but the partaker is a proper character.conversation sheeted character
local function interact_topic(topic, starter, partaker)
    local responses = partaker.conversation.topics_liked[topic]
    partaker.satisfaction = partaker.satisfaction + 0.02

    -- we like each other now, maybe remember that, but depends on the conversation partner whether we create a memory or not
    local who = rnd.rnd(0, 5)
    if who == 0 then
        if starter.conversation then -- this check is required to make sure that you don't try to create a memory for the player
            starter.conversation.sentiment = fmt.f(pick_one(starter.conversation.good_talker), partaker)
            if rnd.rnd() < partaker.chatter then
                create_memory(starter, "friend", partaker)
            end
        end
    elseif who == 1 then
        partaker.conversation.sentiment = fmt.f(pick_one(partaker.conversation.good_talker), starter)
        if rnd.rnd() < starter.chatter then
            -- make sure starter isn't the player because we don't know how we'll do those memories
            if starter.conversation then
                create_memory(partaker, "friend", starter)
            end
        end
    end -- partakerwise, we forget about it

    return responses
end

-- talker tries to start a conversation with other about topic
local function converse_topic(topic, talker, other)
    local my_topics = talker.conversation.topics_liked
    local choices = my_topics[topic]

    -- speak the chosen phrase
    pilot.comm(talker.name, pick_one(choices), "F")

    -- other party probably responds
    if other and other.chatter * (0.75 + rnd.rnd()) > rnd.rnd() then
        local responses = other.conversation.default_participation
        local answered_in = rnd.rnd(4, 16)
        -- do we have to adjust our default responses?
        if other.conversation.topics_disliked[topic] then -- no thanks
            -- record the negative interaction about topic with talker
            dislike_topic(topic, other, talker)
            responses = other.conversation.phrases_disliked
        elseif other.conversation.topics_liked[topic] then -- we like this topic (we have memories and we don't dislike it)
            -- let them interact
            responses = interact_topic(topic, talker, other)

            -- the other party will have responded positively, let's end the conversation here with a default response
            hook.timer(
                answered_in + rnd.rnd(2, 5),
                "say_specific",
                {me = talker, message = pick_one(talker.conversation.default_participation)}
            )
        end

        hook.timer(answered_in, "say_specific", {me = other, message = fmt.f(pick_one(responses), {topic = topic})})
    end
end

local function speak(talker, other)
    local colour = "F"
    local choices = {"I have nothing to say."}
    local last_sentiment = talker.conversation.sentiment
	
    -- figure out what to say
    -- do I have a sentiment that I need to get off my chest?
    if talker.conversation.sentiment then
        -- less important thoughts, we store them all and then throw them away after we picked one out of this hat
        choices = {last_sentiment}
        -- do we want to talk about something else later?
        if talker.conversation.sentiments and rnd.rnd() < talker.chatter then
            talker.conversation.sentiment = pick_one(talker.conversation.sentiments)
        else
            talker.conversation.sentiment = nil
        end
    elseif talker.conversation.sentiments then
        choices = talker.conversation.sentiments
        talker.conversation.sentiments = nil
    elseif talker.satisfaction > 5 then
        -- I'm quite satisfied
        choices = talker.conversation.satisfied
    elseif talker.satisfaction < -3 then
        -- I'm noticably unsatisfied
        choices = talker.conversation.unsatisfied
        colour = "r"
    elseif rnd.rnd() < 0.042 then -- let's try to be original, share a fun fact
        choices = talker.conversation.backstory.funfacts
    else
        -- do I want to talk about a favourite topic?
        -- if the crew is a silent type, be more likely to just make smalltalk
        if rnd.rnd() < talker.chatter then
            -- let's consider our liked topics
            local topic
            for ttt, _v in pairs(talker.conversation.topics_liked) do
                -- if we talk a lot, we will talk more randomly about topics by increasing the chance on low chatters
                if rnd.rnd() > talker.chatter then
                    -- if we like the topic, talk about it unless we randomly feel like we have to talk about it
                    -- otherwise we're always going to be negative, but we want negative memories not to be too repetitive
                    if not talker.conversation.topics_disliked[ttt] or rnd.twosigma() > 1 then
                        topic = ttt
                    end
                end
            end
            -- if we picked a topic
            if topic then
                -- rare edge case we talk to ourselves about a topic (talking out loud)
                if not other then
                    other = talker
                end
                return converse_topic(topic, talker, other)
            end
        end
        -- I don't have anything interesting to say, try smalltalk
        if talker.satisfaction > 0 then
            choices = talker.conversation.smalltalk_positive
        else
            --            colour = "y"
            choices = talker.conversation.smalltalk_negative
        end
    end

    local spoken = pick_one(choices)
	print("talker wants to speak", talker.name, spoken)
    -- say it
    pilot.comm(talker.name, spoken, colour)
    local listener = pick_one(mem.companions)
    -- if there's another person in the conversation, let them interact
    -- a response is more likely than striking a conversation
    if other and other.chatter * 1.5 > rnd.rnd() then
        -- nobody replied to the original talker, let the listener chime in
        -- see if we want to strike up a new conversation based on interests
        for interest, _phrases in pairs(other.conversation.topics_liked) do
            if string.find(spoken, interest) then
                print(
                    fmt.f(
                        "detected {topic} from our list of topics {list}",
                        {topic = interest, list = tostring(other.conversation.topics_liked)}
                    )
                )
                return converse_topic(interest, other, talker)
            end
        end

        -- continue with smalltalk, or whatever this is
        local responses = {
            _("Whatever."),
            _("Yeah, okay."),
            fmt.f(_("Okay, {name}."), talker),
            fmt.f(_("Aright, {name}."), talker),
            _("Sure."),
            _("Oh, really?")
        } -- default responses
        responses = append_table(responses, other.conversation.default_participation)

        local answered_in = rnd.rnd(4, 16)
        hook.timer(answered_in, "say_specific", {me = other, message = pick_one(responses)})

        -- if both talkers are happy, increase their satisfaction a little bit from the interaction
        if talker.satisfaction > 0 and other.satisfaction > 0 then
            talker.satisfaction = talker.satisfaction + 0.01 -- it was a good chat
            other.satisfaction = other.satisfaction + 0.01 -- thanks for including me
            listener.satisfaction = listener.satisfaction + 0.01 -- what a nice conversation to witness
            -- we had a good conversation, let's remember our partner maybe
            if rnd.rnd() > talker.chatter / 2 then
                talker.conversation.sentiment = fmt.f(pick_one(talker.conversation.good_talker), other)
                if not other.conversation.sentiment then
                    other.conversation.sentiment = fmt.f(pick_one(other.conversation.good_talker), talker)
                end
            end
            -- if the listener isn't very chatty, they will remember this
            if rnd.rnd() > listener.chatter then
                listener.conversation.sentiment = fmt.f(pick_one(listener.conversation.good_talker), talker)
            end
        elseif talker.satisfaction < 0 and other.satisfaction < 0 then
            -- penalize negative banter quite heavily
            talker.satisfaction = talker.satisfaction - 0.04 -- I didn't get my attention
            other.satisfaction = other.satisfaction - 0.02 -- why you gotta drag me into this
            listener.satisfaction = listener.satisfaction - 0.01 -- what did I just witness
            if not listener.conversation.sentiment then
                if rnd.rnd(0, 7) == 0 then
                    listener.conversation.sentiment = fmt.f(pick_one(listener.conversation.bad_talker), other)
                end
                if rnd.rnd(0, 13) == 0 then
                    listener.conversation.sentiment = fmt.f(pick_one(listener.conversation.bad_talker), talker)
                end
            end
            talker.conversation.sentiment = fmt.f(pick_one(talker.conversation.bad_talker), other)
            if not other.conversation.sentiment then
                other.conversation.sentiment = fmt.f(pick_one(other.conversation.bad_talker), talker)
            end
        else
            -- feel randomly about this interaction, slightly weighted towards negative
            talker.satisfaction = talker.satisfaction + math.floor(10 * rnd.threesigma()) / 1000 - 0.01
            other.satisfaction = other.satisfaction + math.floor(10 * rnd.threesigma()) / 1000 - 0.0075
            listener.satisfaction = listener.satisfaction + math.floor(10 * rnd.twosigma()) / 1000 - 0.005
        end

        -- if we currently have a new sentiment then we just set it because of conversation
        -- so let's express ourselves
        if talker.conversation.sentiment and talker.conversation.sentiment ~= last_sentiment and rnd.rnd(0, 1) == 0 then
            -- TODO here: give the other person a chance to retort?
            -- it's our turn to talk, or at least we think so
            -- TODO: let's try to start a conversation instead
            local our_response = rnd.rnd(6, 12)
            hook.timer(
                answered_in + our_response,
                "say_specific",
                {me = talker, message = talker.conversation.sentiment}
            )
        elseif rnd.rnd() > talker.chatter then
            -- forget about it this new sentiment, we don't need to talk about it that much
            talker.conversation.sentiment = nil
        end
    elseif listener ~= talker then
        -- check if we can appreciate something that was spoken
        local appreciation, interest = appreciate_spoken(spoken, listener)

        -- listener thought about something, let's remember that instead of discarding the useful data
        insert_sentiment(listener, pick_one(appreciation))

        -- listener will mention his interest to the talker and try to start a conversation
        if interest then
            -- see if we have a sentiment
            return converse_topic(interest, listener, talker)
        elseif listener.conversation.sentiment then
            hook.timer(
                rnd.rnd(6, 36), -- it might take us a while to speak up
                "say_specific",
                {me = listener, message = listener.conversation.sentiment}
            )
        end
    end
end

function say_specific(arg)
    local colour = arg.colour or "F"
    pilot.comm(arg.me.name, arg.message, colour)
end

-- the crew starts a conversation
function start_conversation()
    -- TODO: Talk attempts should be something like crew_mates / 3 or similar until you have a crew manager
    -- pick a random person and see if they want to talk
    local talker = pick_one(mem.companions)
    if not talker then
        return
    end
    local other = pick_one(mem.companions)

    print(fmt.f("picked {name} to talk", talker))
    if other == talker then
        -- special case: I will talk to myself
        if talker.satisfaction > -1 and talker.chatter < rnd.rnd() then
            -- don't talk to myself, I'm not crazy
            other = nil
        end
    end

    -- we are more likely to talk if we are not feeling neutral
    local fudge = math.abs(talker.satisfaction / 10)
    if rnd.rnd() < talker.chatter + fudge then
        -- I will speak
        speak(talker, other)
    else
        print(fmt.f("{name} didn't want to talk", talker))
        -- let other talk instead with a smaller chance
        if other and rnd.rnd() < (other.chatter / 2) then
            print(fmt.f("{name} got a chance to speak", other))
            speak(other) -- they just talk to themselves, not expecting an answer
        elseif other and rnd.rnd() < other.chatter then
            -- we actually wanted to talk, this makes us unhappy
            other.satisfaction = other.satisfaction - 0.02
            insert_sentiment(other, fmt.f(pick_one(other.conversation.bad_talker), talker))
        end
    end

    -- hook initiation of a new conversation
    hook.rm(mem.conversation_hook)
    mem.conversation_hook = hook.date(time.create(0, 1, rnd.rnd(0, 300)), "start_conversation")
end

-- randomly picks a planet belonging to a faction, or "my home planet" if it didn't pick one before it ran out
local function getSpobForFaction(faction)
    for _i, place in ipairs(spob.getAll()) do
        if place:faction() == faction and rnd.rnd() < 0.16 then
            return place
        end
    end

    return "my home planet"
end

-- generates a backstory for an incomplete companion
-- requires the typetitle, faction and skill fields to be set
local function generateBackstory(cdata)
    local backstory = {}
    local random_ship = getRandomShip()
    local random_lastname, random_firstname = pilotname.human()
    if cdata.typetitle == "Companion" then
        backstory.intent =
            pick_one(
            {
                _("I provide services for my customers on various worlds."),
                _("I provide my services for customers around the galaxy."),
                _("I do my business on prosperous worlds."),
                _("I conduct business with wealthy clientele."),
                _("I conduct business around major hubs."),
                _("I would like to live on your ship as I conduct my business around the galaxy."),
                _("My business benefits greatly from frequent travel and I'm looking for a ship."),
                _("I'm looking for a ship from which to conduct my business.")
            }
        )
        backstory.origin = getSpobForFaction(cdata.faction)
        backstory.funfacts = {
            _("My clients are the high paying kind."),
            _("My customers keep me happy."),
            _("I enjoy lavishing myself in luxury."),
            fmt.f(_("I own a {ship}, but I don't like to fly it."), {ship = random_ship}),
            fmt.f(_("Did you know that I'm from {origin}?"), backstory),
            fmt.f(_("I don't talk about my customers, but I can tell you about {lover}."), {lover = random_firstname}),
            fmt.f(
                _("{lover} was my first love, we met on {pworld}. Don't ask me what I was doing there."),
                {lover = random_firstname, pworld = getSpobForFaction(faction.get("Raven Clan"))}
            )
        }
    elseif cdata.typetitle == "Engineer" then
        -- TODO: right now this is for explosives expert, but we need a generic engineer one
        backstory.intent =
            pick_one(
            {
                _("I love to blow things up."),
                _("You look like you could use a demolition man!"),
                _("I am an explosives expert."),
                _("I'll make sure enemies you board don't come back."),
                _("I like to make things go boom."),
                _("I will blow it into smithereens. Just point the finger and I'll set the charges."),
                _("We're going to have a lot of fun together, believe me.")
            }
        )
        backstory.origin = getSpobForFaction(cdata.faction)
        backstory.funfacts = {
            fmt.f(_("Did you know that I'm from {origin}?"), backstory),
            _("One day, I'd love to rig a Goddard to blow.."),
            fmt.f(_("One day I'll tell you about my first {ship} and what happened to it."), {ship = random_ship}),
            fmt.f(
                _("My last captain named his ship the {shipname}. What an idiot. It had to go, it had to blow."),
                {shipname = getMadeUpName()}
            ),
            fmt.f(
                _(
                    "One of my last captains almost renamed his ship to {shipname} in a drunken stupor... I made sure that couldn't happen."
                ),
                {shipname = getMadeUpName()}
            ),
            fmt.f(
                _("I used to have a cousin named {cousin}, but don't ask me what happened to him."),
                {cousin = random_firstname}
            ),
            fmt.f(_("You ever heard of the {shipname}? Yeah, that was me."), {shipname = random_lastname}),
            fmt.f(_("You ever heard of the {made_up} explosion? Yeah, I did that."), {made_up = getMadeUpName()})
        }
    else -- something super generic
        backstory.intent =
            pick_one(
            {
                _("I'm looking for a ship to lay low on for as long as you'll have me."),
                _("I'm looking for a ship to lay low on."),
                _("I'm just looking to hang back for a while."),
                _("I was just hoping to catch a ride."),
                _("I'm just hoping to catch a ride to anywhere and maybe get some work along the way."),
                _("I'll hang around for a bit if you don't mind my company."),
                _(
                    "I like to make conversation. Maybe I can join your crew and participate in all the fun and excitement that happens on your ship?"
                ),
                _("I will keep you company. I'm not sure if you need it, though."),
                _("I've been out of work for a while and am looking for a place to hang my hat."),
                _("I'm looking for a new family."),
                _(
                    "I'm looking for a ship for work. I'm not from around here and my last captain traded in his ship to pay off some debt."
                ),
                _("I'm a hard worker. I'll make sure your docking clamps are secure every time."),
                _("I'm a good worker. I'll make sure your cargo bay equipment is in order."),
                _(
                    "I inherited an interplanetary tea house franchise, but the relative ease of spacetravel completely destroyed the market. It only lasted me seven years. I wish it could have been eight."
                )
            }
        )
        backstory.origin =
            pick_one(
            {
                _("somewhere far away"),
                _("a bad place"),
                _("nowhere"),
                "Janus Station",
                spob.get(true), -- a completely random spob! Hah!
                _ "Kramer",
                _("Earth"),
                _("the future"),
                getMadeUpName(),
                _("outer space"),
                _("House Goddard"),
                _("Townstead"),
                ("Eden")
            }
        )

        backstory.funfacts = {
            fmt.f(_("I used to work on a {ship}, that was really something."), {ship = getRandomShip()}),
            fmt.f(_("I used to run a side gig where I repaired {thing}s."), {thing = getRandomThing()}),
            fmt.f(_("If anyone asks where I'm from, just tell them I'm from {origin}."), backstory),
            fmt.f(
                _("If anyone asks you where I'm from, tell them I'm from {origin}, see if they believe you."),
                backstory
            ),
            fmt.f(_("Don't tell anyone that I'm from {origin}, not that they would believe you."), backstory),
            fmt.f(
                _(
                    "I used to work near Crylo and one day, {someone} caught a {fish} so big we were called down to transport it off-world on our Gaiwan."
                ),
                {
                    someone = pick_one(
                        {_("someone"), _("somebody"), _("a youngster"), _("some group"), _("some people")}
                    ),
                    fish = pick_one({_("fish"), _("whale"), _("creature"), _("shark")})
                }
            ),
            fmt.f(
                _("I used to work on a {ship} near {place}."),
                {ship = getRandomShip(), place = spob.get(faction.get("Empire"), faction.get("Za'lek"))}
            ),
            fmt.f(
                _("One of my previous ships had a regular tour of {place}."),
                {place = spob.get(faction.get("Soromid"))}
            ),
            fmt.f(_("My last ship, the {name}, was a {coffin}."), {name = getMadeUpName(), coffin = getSpaceThing()}),
            fmt.f(_("My last captain named his ship the {shipname}. What an idiot."), {shipname = getMadeUpName()}),
            fmt.f(
                _(
                    "My last captain named his ship the {shipname}. The thing was a {bettername}. The captain was a {trashname}."
                ),
                {shipname = getMadeUpName(), bettername = getSpaceThing(), trashname = getInsultingProperNoun()}
            ),
            fmt.f(
                _("One of my last captains almost renamed his ship to {shipname} in a drunken stupor."),
                {shipname = getMadeUpName()}
            ),
            _("I'm not exactly the most useful person, but I'm hoping that nobody will notice."),
            _("I'm not really a hard worker, but I'm hoping that nobody will notice."),
            _("Actually, I am pretty lazy and I think that honesty is a damn good policy.")
        }
    end
    -- The crewmate also gets a rare and unique looking special fact
    local special_facts = {
        fmt.f(
            _(
                "Actually, I killed someone on my last crew. That's why I'm really here. I'll never forget {name} and that last look."
            ),
            {name = pilotname.generic()}
        ),
        fmt.f(
            _(
                "I had a secret love affair on my last post. That's why I'm really here. I'll never forget {name} and that kiss goodbye."
            ),
            {name = pilotname.human()}
        ),
        fmt.f(
            _(
                "I have a large scar on my back. I got it in an altercation between a mighty warlord on {place} due to a misunderstanding."
            ),
            {place = spob.get(faction.get("Dvaered"))}
        ),
        fmt.f(
            _(
                "I have a large scar on my leg. I got it in an altercation with a fiesty {person} on {place} after a slight misunderstanding."
            ),
            {
                person = pick_one({_("woman"), _("youngster"), _("acrobat"), _("man"), _("warrior")}),
                place = spob.get(faction.get("Dvaered"))
            }
        ),
        fmt.f(
            _("I have a hole in my leg. I got it after a minor misunderstanding with a {person} near {place}."),
            {
                person = pick_one(
                    {_("drone"), _("robotic guard"), _("manual firearm"), _("fictional character"), _("badass")}
                ),
                place = spob.get(faction.get("Za'lek"))
            }
        ),
        _("I lived on Kramer for over a year. I'll save you the jealousy and spare you the details."),
        _("I killed a man with my bare hands, but I'll spare you the details."),
        _(
            "I killed a man with my bare hands, it was intense. It was me or him. I was obviously outmatched, but I got lucky."
        ),
        _(
            "I was once a split second from getting blasted into bits by an armed guard near {place} when a masked stranger appeared out of nowhere and swapped out his plasma rifle with an umbrella! Yeah, I didn't believe it when it happened right in front of my eyes either."
        ),
        _(
            "Don't tell anyone I told you this, but I once managed to fool a bounty hunter by masking my Quicksilver as a Kestrel. To this day I can't belive he just trusted his sensors and didn't notice the stark differences between a Quicksilver and a Kestrel through the optical interface. Not to mention the difference in size!"
        ),
        _("One of my lovers near Zinter is a pyromaniac. I'd stay away from that area.")
    }
    table.insert(backstory.funfacts, pick_one(special_facts))

    return backstory
end

local function generateIntroduction(cdata)
    local greetings = {
        _("Hi."),
        _("Hi!"),
        _("Hello."),
        _("Hello!"),
        _("Hi there!"),
        _("Hello there."),
        _("Hi there."),
        _("Greetings."),
        _("Salutations."),
        _("Ni hao."),
        _("Huzzah!"),
        _("Hey!")
    }

    -- I say this if I'm a chatterbox right before asking if I can join the crew
    local preprompts = {
        _("I'd love to join a crew such as yours."),
        _("I'd love to serve on a ship such as yours."),
        _("So..."),
        _("Well anyway..."),
        _("Well, what do you think?")
    }

    local prompts = {
        _("Can I join your crew?"),
        _("What do you say, would you like me on board?"),
        _("Would you like to add me to your roster?"),
        _("Would you like to add me to your crew?"),
        _("Can I live on your ship?"),
        _("What do you say, can I come on board?")
    }

    local reassurances = {
        _("You can count on me!"),
        _("I'll show you, you'll see!"),
        _("You will regret not taking me on!"),
        _("I'll be a good companion."),
        _("I'll be a good crewmate."),
        _("I'll be a hard worker, you'll see."),
        _("Is it obvious that I like to talk? Oh well..."),
        _("I tend to talk a lot, I hope that's okay..."),
        _("My friends say that I'm a bit of a chatterbox...")
    }

    -- just generate a random backstory for now
    local backstory = cdata.conversation.backstory
    local params = {
        greeting = pick_one(greetings),
        prompt = pick_one(prompts),
        preprompt = pick_one(preprompts),
        intent = backstory.intent,
        reassurance = pick_one(reassurances),
        funfact = pick_one(backstory.funfacts),
        skill = cdata.skill,
        name = cdata.name
    }

    -- use chatter to determine how much backstory to give (and maybe.. 'mystery'? nah...)
    local approachtext
    if cdata.chatter < 0.2 then -- I am the strong, silent type
        approachtext = "Hello. {skill}. Need one?"
    elseif cdata.chatter < 0.36 then -- I am a quiet person
        approachtext = "{greeting} {intent} {prompt}"
    elseif cdata.chatter < 0.6 then -- I speak an average amount
        approachtext = "{greeting} I'm {name}. {intent}\n\n{funfact} {prompt}"
    else -- I talk a lot
        if rnd.rnd(0, 1) == 0 then
            approachtext = "{greeting} My name is {name}. {intent} {reassurance}\n\n{funfact} {preprompt} {prompt}"
        else
            approachtext =
                "{greeting} {intent} Oh, did I mention that my name is {name}? {reassurance}\n\n{funfact} {preprompt} {prompt}"
        end
    end

    return fmt.f(approachtext, params)
end

local function generateTopics()
    local topics = {
        -- list of phrases that use the things I can like (or not)
        ["view"] = {
            _("Did you see the view at that last shipyard?"),
            _("Did you notice the spectacular view during that eclipse?"),
            _("What a wonderful view. The stars are amazing."),
            _("What a wonderful view. The galaxy is amazing."),
            _("What a fantastic view."),
            _("What a fascinating display."),
            _("A view like that is worth a lot of credits."),
            _("Wow, get a load of that view!"),
            _("Oh man, what was that? Did anyone else see that?"),
            _("Did anyone else see that?"),
            _("Did you see that?"),
            _("This is why people travel, this is a true life of luxury."),
            _("I like looking out at the stars."),
            _("How could anyone not admire this view?")
        },
        ["luxury"] = {
            fmt.f(_("Do you want to see my {ship}? I keep it in storage."), {ship = getRandomShip()}),
            fmt.f(_("How do you like this {thing}?"), {thing = getRandomThing()}),
            fmt.f(_("I'm thinking about investing in {thing}s."), {thing = getRandomThing()}),
            fmt.f(_("I'm thinking about investing in {thing}s, what do you think?"), {thing = getRandomThing()}),
            _("What do you think about this color?"),
            _("Will you take me to Kramer some day?"),
            _("You should try the soap that I'm using, I don't know where you get your stuff."),
            _("You should try this lotion."),
            _("Here, try this cream."),
            _("Here, try this."),
            _("Here, try this, I know you'll like it."),
            _(
                [[Here, smell this. It's called "the faithful friend", but it cost a lot of credits, almost as much as this ship.]]
            ),
            _(
                [[Here, smell this. It's called "the pacifier of violence", but it cost a lot of credits, almost as much as this ship.]]
            ),
            fmt.f(
                _("You should try this new virtual viewport simulator, it lets you pretend you're on a {ship}."),
                {ship = getRandomShip()}
            ),
            _("Back when I got my first Admonisher at Minerva Station, it cost ten thousand tokens."),
            _("I'll never fly in a Llama again, I much prefer the Gawain."),
            _("I'll never fly in a shuttle again, I much prefer the Gawain."),
            _("The Gawain is probably my favourite ship. I mean, that interior is to die for, literally."),
            _("The Gawain is probably my favourite ship. I mean, that interior is to kill for, literally.")
        },
        ["friend"] = {
            fmt.f(_("Check out this {ship} my friend thinking of buying."), {ship = getRandomShip()}),
            _("Do you want to grab a coffee?"),
            _("I like how close we are."),
            _("I know we've had our differences, but you're alright."),
            _("Hey, buddy! Hows it's going? You good?"),
            fmt.f(_("Check out the custom paintjob on this {ship}!"), {ship = getRandomShip()}),
            fmt.f(_("My friend {name} would love this."), {name = pilotname.human()}),
            fmt.f(_("I'm sure {name} would appreciate this."), {name = pilotname.human()}),
            fmt.f(_("I miss my friend {name}."), {name = pilotname.human()}),
            fmt.f(_("I used to have a friend called {name}."), {name = pilotname.human()}),
            fmt.f(_("I miss my friend {name}."), {name = getMadeUpName()}),
            fmt.f(_("I used to have a friend called {name}."), {name = getMadeUpName()}),
            fmt.f(_("I miss my old friend {name}."), {name = pilotname.human()}),
            fmt.f(_("I miss my other friend {name}."), {name = pilotname.human()}),
            fmt.f(_("I miss my friend's {name}."), {name = getRandomShip()}),
            fmt.f(_("I miss my friend's {name}. It wasn't special, but our friendship was."), {name = getRandomThing()}),
            fmt.f(_("I miss my friend {name}."), {name = getMadeUpName()}),
            fmt.f(_("I miss my friends from the {name} I used to work on."), {name = getRandomShip()})
        },
        ["ship"] = {
            fmt.f(_("Check out this {ship} my friend thinking of buying."), {ship = getRandomShip()}),
            fmt.f(_("Do you want to see my sister's {ship}? She keeps talking about it."), {ship = getRandomShip()}),
            fmt.f(
                _("This is my favourite ship. The {name} of {made_up} was alright but nothing like this."),
                {made_up = getMadeUpName(), name = pilotname.human()}
            ),
            fmt.f(
                _("This is such a nice ship. The {made_up} of {name} was decent but nothing like this."),
                {made_up = getMadeUpName(), name = pilotname.human()}
            ),
            fmt.f(_("I once constructed a {ship} in a single cycle."), {ship = getRandomShip()}),
            fmt.f(
                _("Have I told you about the prototype {ship} I designed during my training?"),
                {ship = getRandomShip()}
            ),
            fmt.f(_("I used to serve on a {ship}. I think I've told you about it, right?"), {ship = getRandomShip()}),
            fmt.f(
                _("I once saw a comet shaped like a {ship} near {place}. It was pretty cool."),
                {ship = getRandomShip(), place = spob.get(true)}
            ),
            fmt.f(
                _("I've heard from {name} the new {ship} is going to be even sleeker than the current model."),
                {name = pilotname.human(), ship = getRandomShip()}
            ),
            fmt.f(
                _(
                    "I've heard from my old friend {name} the next {ship}, code-name {made_up} is going to have a special compartment for {thing}s."
                ),
                {
                    ship = getRandomShip(),
                    name = pilotname.generic(),
                    made_up = getMadeUpName(),
                    thing = getRandomThing()
                }
            ),
            fmt.f(_("Have you seen the new {ship} features?"), {ship = getRandomShip()}),
            fmt.f(_("Have you seen these hidden {ship} features?"), {ship = getRandomShip()}),
            fmt.f(
                _("Did you read that article about the 7 {ship} features no captain knows about?"),
                {ship = getRandomShip()}
            )
        },
        ["visit"] = {
            fmt.f(
                _("I was traveling near {place} for business in my early years. Have you been there?"),
                {place = spob.get(faction.get("Independent"))}
            ),
            fmt.f(
                _("I was traveling near {place} for business in my early years. Have you been there?"),
                {place = spob.get(faction.get("Empire"))}
            ),
            fmt.f(
                _("I was traveling near {place} on a ship in my early years. I hated it, but the view was nice."),
                {place = spob.get(faction.get("Dvaered"))}
            ),
            fmt.f(
                _("I had to travel near {place} for business in my early years. Have you been there?"),
                {place = spob.get(faction.get("Soromid"))}
            ),
            fmt.f(
                _("I got to travel to {place} in my childhood. Unforgettable."),
                {place = spob.get(faction.get("Sirius"))}
            ),
            fmt.f(
                _("I've heard that {place} is known for being good at science."),
                {place = spob.get(faction.get("Za'lek"))}
            ),
            fmt.f(
                _("Of all my travels I must say, I've been too often to {place}."),
                {place = spob.get(faction.get("Soromid"))}
            ),
            fmt.f(
                _("I didn't really like working at {place}, but the pay was good."),
                {place = spob.get(faction.get("Soromid"))}
            ),
            fmt.f(
                _("I used to visit my {relative} {name} regularly on {place}."),
                {
                    place = spob.get(faction.get("Empire")),
                    name = pilotname.human(),
                    relative = pick_one({_("aunt"), _("grandmother"), _("uncle"), _("grandfather"), _("councellor")})
                }
            ),
            fmt.f(
                _("{place} is my favourite pirate outpost. Don't ask me why, it just is."),
                {place = spob.get(faction.get("Raven Clan"))}
            )
        },
        ["faith"] = {
            _("Sometimes you just have to have a little bit of faith."),
            _("Sometimes you just have to let the spirits guide you."),
            _("Sometimes you need to trust the universe."),
            _("I believe that traveling among the stars makes us immortal."),
            _("We're all going to die one day. Let's enjoy the ride."),
            _("I believe that things get worse, then they get better."),
            fmt.f(_("We all have a lot to believe in, but I believe in my lucky {thing}."), {thing = getRandomThing()})
        },
        ["affair"] = {
            fmt.f(
                _("I too have had my fair share of affairs. One day I might tell you the story of {name}."),
                {name = pilotname.human()}
            ),
            fmt.f(_("I had an affair with a dangerous vagabond named {name}."), {name = pilotname.human()}),
            fmt.f(_("I got into a scuffle with a criminal named {name}."), {name = pilotname.human()}),
            fmt.f(
                _("I got into a mighty struggle with {name} of {made_up} before I joined this ship."),
                {made_up = getMadeUpName(), name = pilotname.human()}
            ),
            _("Don't ask me about my affairs."),
            _("Don't ask me about my love affairs."),
            _("Don't ask me about my personal affairs."),
            _("Don't ask me about my previous life."),
            _("Don't ask me about my personal life."),
            _("Please don't talk to me when I'm off duty."),
            _("I don't want to talk right now."),
            _("Not right now."),
            _("I know I can be secretive sometimes, but some things are best left unsaid."),
            _("Actually, can we save this for later?"),
            _("Let's save this for later? I wanted to enjoy the view."),
            _("I want to visit one of my previous lovers. I'm just not sure which."),
            _("Not in this company. Later."),
            _("I'm not sure that whatever we're discussing is appropriate but whatever."),
            fmt.f(
                _(
                    "... I'm not sure that's appropriate but whatever. I'm sure {captain} doesn't mind. Oh, hey {captain}!"
                ),
                {captain = player:name()}
            )
        },
        ["travel"] = {
            fmt.f(
                _("One of my favourite places to visit is {place}. Have you been there?"),
                {place = spob.get(faction.get("Independent"))}
            ),
            fmt.f(
                _("A fascinating place to visit is {place}. Have you been there?"),
                {place = spob.get(faction.get("Za'lek"))}
            ),
            fmt.f(
                _("I heard that {place} is developing a new {made_up}. Have you been there?"),
                {place = spob.get(faction.get("Za'lek")), made_up = getMadeUpName()}
            ),
            fmt.f(
                _("Of all my travels I must say, I've been too often to {place}. Have you been there?"),
                {place = spob.get(faction.get("Empire"))}
            ),
            fmt.f(
                _("I had an affair with a warrior from {place}. I wonder what fearsome {name} is up to these days."),
                {place = spob.get(faction.get("Dvaered")), name = pilotname.human()}
            ),
            fmt.f(
                _(
                    "All the violence and lawlessness on {place} led my cousin {name} towards a path of disastrous affairs."
                ),
                {place = spob.get(faction.get("Dvaered")), name = pilotname.human()}
            ),
            fmt.f(
                _("I went to {place} just to check it out. I haven't had the urge to go since."),
                {place = spob.get(faction.get("Soromid"))}
            )
        },
        ["violence"] = {
            fmt.f(_("I once destroyed a {ship} in a single volley."), {ship = getRandomShip()}),
            fmt.f(_("Have I told you about the {ship} I destroyed during my training?"), {ship = getRandomShip()}),
            _("I've killed a man with my bare hands."),
            _("I could kill you with a spoon."),
            fmt.f(
                _("I could slay you with a defective {thing} in a {thong} battle."),
                {thing = getMadeUpName(), thong = getMadeUpName()}
            ),
            _("You call that a knife?"),
            _("I will bathe in the blood of my enemies."),
            _("What are you looking at?"),
            _("Enjoying the view? Enjoy it while it lasts."),
            _("Give me some credits, or die."),
            _("I'll skin you if you meddle in my personal affairs."),
            _("This part of the ship is mine; you got it, friend?"),
            _(
                "I know we're not supposed to have blades like these in the spaceport but this baby never leaves my side, I take it everywhere."
            ),
            _("I like to strike fear in my enemies."),
            _("I will strike fear in my enemies."),
            _("To run is to lose, to win is to die."),
            _("Sometimes, excessive force is necessary."),
            _("You always have a right to defend yourself. Even if that means shooting someone in the face."),
            fmt.f(_("Check out my {made_up} rifle, pretty neat, huh?"), {made_up = getMadeUpName()}),
            fmt.f(
                _(
                    "Check out this {made_up}. I only paid {amount} for this killer. How many credits is that per ship destroyed?"
                ),
                {made_up = getMadeUpName(), amount = fmt.credits(rnd.rnd(35e3, 725e3))}
            ),
            _("I would never kill my friends, but I could break your leg on a whim."),
            fmt.f(
                _("All the violence and lawlessness on {place} excites me."),
                {place = spob.get(faction.get("Dvaered"))}
            ),
            fmt.f(
                _("I got into a scuffle on {place} some cycles back. That's where I got the small scar."),
                {place = spob.get(faction.get("Dvaered"))}
            ),
            fmt.f(
                _("We all make mistakes. I once killed a man on {place} only to find out I got the wrong guy."),
                {place = spob.get(faction.get("Empire"))}
            ),
            fmt.f(
                _(
                    "I once had our ship travel all the way to {place} only to find out that I forgot to reset the navigation equipment. Let's just say that I would owe them quite a few credits if they had any use for them."
                ),
                {place = spob.get(faction.get("Empire"))}
            )
        },
        ["credits"] = {
            _("Give me some credits."),
            _("Lend me some credits."),
            _("Hand me that credit chip."),
            _("Is this yours? Do you mind?"),
            _("Is that yours? Can I take it?"),
            _("Can I have that?"),
            _("Are you gonna use that?"),
            _("*inaudible*"),
            _("Are we talking about money? That's what I like to call it."),
            _("Did somebody say credits? Yeah I'm listening."),
            _("How many credits is that?"),
            _("Who doesn't love dough?"),
            _("I heard what you said earlier about your business."),
            _("Who doesn't love credits?"),
            _("Who doesn't love moolah?"),
            _("That's a lot of coin."),
            _("I wouldn't want to owe that much."),
            _("I wouldn't want to own that much."),
            _("It sounds like a lot, but if you scratch my back, I'll scratch yours."),
            _("Stop bothering me, I'm trying to think."),
            _("Stop bothering me, I'm trying to think!"),
            _("Why are we changing the subject?"),
            _("I can't stop thinking about the weight of that last credit chip."),
            _("Can't you see I'm trying to count here?"),
            _("Sorry, I'm having a little trouble doing my finances right now.")
        },
        ["science"] = {
            fmt.f(_("Check out the landing gear on this {ship}!"), {ship = getRandomShip()}),
            fmt.f(_("Have you seen the new {ship} features?"), {ship = getRandomShip()}),
            fmt.f(_("Have you seen these hidden {ship} features?"), {ship = getRandomShip()}),
            fmt.f(
                _("Did you read that article about the 7 {ship} features no captain knows about?"),
                {ship = getRandomShip()}
            ),
            fmt.f(_("I heard about some unexplained phenomena at {place}."), {place = spob.get(true)}),
            fmt.f(_("I wonder what the big deal about {place} is, there's no mystery."), {place = spob.get(true)}),
            fmt.f(
                _("I once saw a comet shaped like a {ship} near {place}. It was pretty cool."),
                {ship = getRandomShip(), place = spob.get(true)}
            ),
            fmt.f(
                _("I've heard from {name} the new Admonisher is going to be even sleeker than the current model."),
                {name = pilotname.human()}
            ),
            fmt.f(
                _(
                    "I've heard from {name} the next {ship}, code-name {made_up} is going to have a secret unscannable cargo compartment."
                ),
                {ship = getRandomShip(), name = pilotname.generic(), made_up = getMadeUpName()}
            ),
            fmt.f(
                _("The new '{made_up}' {thing} is supposedly the bee's knees."),
                {made_up = getMadeUpName(), thing = getRandomThing()}
            ),
            fmt.f(
                _("I've heard that an updated {thing} is going to be even sleeker than the current model."),
                {thing = getRandomThing()}
            ),
            fmt.f(
                _("I've heard that the new {thing} is going to be even sleeker than the current model."),
                {thing = getMadeUpName()}
            ),
            fmt.f(_("I just read that the {thing} is getting revamped again."), {thing = getMadeUpName()}),
            fmt.f(_("I've tried that new {thing} and I have to say, I'm amazed."), {thing = getMadeUpName()}),
            fmt.f(
                _("Did you hear that the {thing} now only costs {amount}?"),
                {thing = getMadeUpName(), amount = fmt.credits(rnd.rnd(20e3, 50e3 - 1))}
            ),
            fmt.f(_("I've tried that new {thing} and I have to say, I'm amazed."), {thing = getMadeUpName()}),
            fmt.f(
                _("I don't think those paid science publications are any good. I put all my faith in {thing}."),
                {thing = getMadeUpName()}
            ),
            fmt.f(
                _(
                    "{thing} luxury friendship bracelets are practically being given away for just a single credit if you adopt an abandoned illegal pet iguana and fill out some required forms. The only downside is that we'd have to travel to {place} to go get it."
                ),
                {thing = getMadeUpName(), place = spob.get(faction.get("Empire"))}
            )
        }
    }

    local liked = {}
    local disliked = {}
    -- now go through all the topics, and pick whether we like, dislike, or don't care about each one
    for topic, phrases in pairs(topics) do
        local roll = rnd.rnd(0, 6)
        if roll > 3 then -- yay, we like this
            liked[topic] = phrases
        elseif roll < 3 then -- ouch, we got a 2 or a 3
            table.insert(disliked, topic)
        end
    end
    return liked, disliked
end

local function crewManagerAssessment()
    local troublemaker
    local min_satisfaction = 0
    local max_satisfaction = 0
    local cumul_satisfaction = 0
    for _i, crew in ipairs(mem.companions) do
        cumul_satisfaction = crew.satisfaction + cumul_satisfaction
        if crew.satisfaction > max_satisfaction then
            max_satisfaction = crew.satisfaction
        elseif crew.satisfaction < min_satisfaction then
            min_satisfaction = crew.satisfaction
            troublemaker = crew
        end
    end

    -- if we aren't satisfied to at least 1, let's worry
    if (cumul_satisfaction / #mem.companions) < 1 then
        return "unsatisfied", troublemaker
    end

    -- if someone is at -1, let's worry
    if min_satisfaction <= -1 then
        return "troublemaker", troublemaker
    end

    return "satisfied", troublemaker
end

-- create a generic manager component for crew management (goes into companion.manager)
local function createGenericCrewManagerComponent()
    local manager = {}

    manager.type = "Personnel Management"
    local lines = {}
    manager.cost = 1e3 -- how much it costs to "activate" this manager

    lines.satisfied = {
        _("The crew seems happy."),
        _("The crew is content."),
        _("The crew doesn't look so bad at all."),
        _("The crew seems to be enjoying themselves."),
        _("The crew doesn't need any micromanagement at this point."),
        _("The crew doesn't seem to be having any issues."),
        _("The crew's looking good."),
        _("The crew seems to be doing good."),
        _("The crew is performing within parameters."),
        _("The crew is doing well.")
    }

    lines.unsatisfied = {
        _("The crew seems unhappy."),
        _("The crew isn't happy."),
        _("The crew doesn't look very happy."),
        _("The crew doesn't seem very happy."),
        _("The crew doesn't look happy."),
        _("The crew doesn't seem happy."),
        _("The crew's in a slump."),
        _("The crew could use a turnaround."),
        _("The crew is getting fatigued."),
        _("The crew needs a change of pace.")
    }

    lines.troublemaker = {
        _("We've got some problems with {name}."),
        _("You have a troublemaker in your crew."),
        _("{name} has been performing poorly."),
        _("{name} has been causing some issues."),
        _("{name} has been causing problems."),
        _("{name} has been causing trouble."),
        _("Honestly, we are having {typetitle} problems."),
        _("The {typetitle} situation could be better."),
		_("The {typetitle} situation has been better."),
		_("The {typetitle} problem is getting worse."),
		_("The {typetitle} situation is becoming noticeable."),
		_("The {typetitle} situation needs to be improved."),
        _("I've had some complaints about {name}."),
        _("I've had some complaints."),
        _("We've got some issues with {name}."),
        _("We've got some unresolved tension between {name} and the rest of the crew."),
		_("We've got some unresolved tension some {typetitle} and the rest of the crew."),
		_("We've got some {typetitle} complaining about the rest of the lot."),
        _("Let's not get into it. It's not looking good."),
        _("I don't want to point any fingers."),
        _("Don't say I didn't warn you. Can we leave it at that?")
    }

    manager.lines = lines

    return manager
end

-- create a generic crewmate with no special skills whatsoever
local function createGenericCrewmate(fac)
    fac = fac or faction.get("Independent")
    local portrait_arg = nil
    local pf = spob.cur():faction()
    local lastname, firstname = pilotname.human()
    if pir.factionIsPirate(pf) then
        fac = faction.get("Pirate")
        lastname = pilotname.pirate()
        portrait_arg = "Pirate"
    end
    local portrait_func = portrait.getMale

    local character = {}
    character.gender = "Male"
    character.article_object = "him"
    character.article_subject = "he"
    -- generic character has 50% chance of being male or female
    if rnd.rnd(0, 1) == 1 then
        character.gender = "Female"
        character.article_object = "her"
        character.article_subject = "she"
        portrait_func = portrait.getFemale
    -- TODO: female name
    end

    character.name = lastname
    character.firstname = firstname

    character.typetitle = "Crew"
    character.skill = pick_one({"Cargo Bay", "Janitorial", "Maintenance", "General Duty"})
    character.satisfaction = rnd.rnd(1, 3)
    character.threshold = 1e3 -- how much they need to be happy after doing a paid job
    character.xp = math.floor(10 * (1 + rnd.sigma())) / 10
    character.portrait = portrait_func(portrait_arg)
    character.faction = fac
    character.chatter = 0.5 + rnd.twosigma() * 0.2 -- how likely I am to talk at any given opportunity
    character.deposit = math.ceil(75e3 * character.satisfaction * character.xp + character.chatter * rnd.rnd() * 8e3)
    character.salary = 0
    character.other_costs = "Water" -- if you don't have a cost factor, just cost water
    local liked, disliked = generateTopics()
    character.conversation = {
        ["backstory"] = generateBackstory(character),
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
        },
        -- what I say when not satisfied
        ["unsatisfied"] = {
            _("I am unhappy."),
            _("The situation is bleak."),
            _("The atmosphere is unfriendly."),
            _("I'm bored."),
            _("I get bored sometimes."),
            _("There's nothing to do around here."),
            _("It's too quiet here."),
			_("The past periods will haunt me."),
			_("The last cycle was harsh."),
			fmt.f(_("The {fruit} situation on the ship could be better."), {fruit = getRandomFruit() } ),
			fmt.f(_("I haven't seen a single {fruit} in cycles."), {fruit = getRandomFruit() } ),
			fmt.f(_("I last saw maybe one {fruit} some periods ago or longer, I don't even remember anymore."), {fruit = getRandomFruit() } ),
			fmt.f(_("I need my {fruit}s."), {fruit = getRandomFruit() } ),
			fmt.f(_("There's never enough {fruit}s when I need one."), {fruit = getRandomFruit() } ),
			fmt.f(_("I last saw maybe one {fruit}... I don't even remember anymore."), {fruit = getRandomFruit() } ),
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
            _("To hell with it, {name}."),
            _("Yeah, okay."),
            _("Interesting."),
            _("Whatever..."),
            _("Whatever, jeez."),
            _("Oh come on {name}, don't start with me."),
            _("Not this again, {name}."),
            _("I think {article_subject}'s being offensive."),
			_("I think {article_subject}'s having a bad day."),
			_("You'd think {article_subject}'d keep those thoughts to {article_object}self."),
			fmt.f(_("You should keep those thoughts to yourself, shouldn't {article_subect}, {captain}?"), {article_subject="{article_subject", captain=player.name() } ),
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
			fmt.f(_("Anyone want to play some {game}?"), { game = pick_one(getShipboardActivity("game")) }),
			fmt.f(_("Anyone want to play {game}?"), { game = pick_one(getShipboardActivity("game")) }),
			fmt.f(_("Anyone want to play a game of {game}?"), { game = pick_one(getShipboardActivity("game")) }),
			fmt.f(_("I really want to play {game}."), { game = pick_one(getShipboardActivity("game")) }),
			fmt.f(_("Do I really have to do all that {basic}?"), { basic = pick_one(getShipboardActivity("basic")) }),
			fmt.f(_("Do I really have to do all that {basic}?"), { basic = pick_one(getShipboardActivity("basic")) }),
			fmt.f(_("Do I have to do the {basic}?"), { basic = pick_one(getShipboardActivity("basic")) }),
			fmt.f(_("Oh, I forgot that I have to do my {basic} for today."), { basic = pick_one(getShipboardActivity("basic")) }),
			fmt.f(_("I'm going for some {basic}."), { basic = pick_one(getShipboardActivity("basic")) }),
			fmt.f(_("I'm going to do that {basic} in a bit."), { basic = pick_one(getShipboardActivity("basic")) }),
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
			fmt.f(_("I saw there were some {fruit}s in the break room earlier but I didn't take one."), {fruit = getRandomFruit() } ),
			fmt.f(_("I can't believe I didn't take the last {fruit}."), {fruit = getRandomFruit() } ),
			fmt.f(_("I haven't seen a single {fruit} in cycles."), {fruit = getRandomFruit() } ),
			fmt.f(_("I last saw maybe one {fruit} some periods ago or longer, I don't even remember anymore."), {fruit = getRandomFruit() } ),
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
            fmt.f(_("My {made_up} is acting up. I'd better go check on it"), {made_up = getMadeUpName()}),
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
			fmt.f(_("Would you like to play some {game}?"), { game = getRandomGame() }),
			fmt.f(_("I'll play some {game} if you want."), { game = getRandomGame() }),
			fmt.f(_("Would you like to this {fruit}?"), { fruit = getRandomFruit() }),
			fmt.f(_("I'll give you my {fruit} if you want it."), { fruit = getRandomFruit() }),
			fmt.f(_("I just ate a really nice {fruit}."), { fruit = getRandomFruit() }),
			fmt.f(_("Do you want a piece of this {fruit}?"), { fruit = getRandomFruit() }),
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
			_("Your comany is unappreciated."),
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
			fmt.f(_("Don't say a {thing} thing!"), { thing = getMadeUpName()}),
			fmt.f(_("What are you looking at, you {insult}?"), {insult = getInsultingProperNoun()}),
        },
        -- list of things I like to talk about and what I say about them
        ["topics_liked"] = liked,
        -- list of things I don't like talking about
        ["topics_disliked"] = disliked,
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
            _("I don't think so."),
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
			fmt.f(_("Don't be such a {insult}."), {insult = getInsultingProperNoun()}),
        }
    }

    -- give the character some unique bar actions
    character.conversation.bar_actions = {
        {
            ["verb"] = pick_one(
                {
                    _("swirling"),
                    _("sipping"),
                    _("drinking"),
                    _("enjoying"),
                    _("nursing"),
                    _("nursing on"),
                    _("chugging")
                }
            ),
            ["descriptor"] = pick_one(
                {
                    _("a"),
                    _("another"),
                    _("some"),
                    _("some kind of")
                }
            ),
            ["adjective"] = pick_one(
                {
                    _("cheerful"),
                    _("bizarre"),
                    _("steaming"),
                    _("iced"),
                    _("chilled"),
                    _("extravagant"),
                    _("foaming")
                }
            ),
            ["object"] = pick_one(
                {
                    _("drink"),
                    _("wine"),
                    _("tea"),
                    _("concoction"),
                    _("elixir"),
                    _("mixture of fluids"),
                    _("beverage"),
                    _("beer"),
                    _("coffee"),
                    _("spirit")
                }
            )
        }
    }

    return character
end

local function createGenericManager()
    local crewmate = createGenericCrewmate()
    crewmate.manager = createGenericCrewManagerComponent()
    crewmate.skill = crewmate.manager.type
    crewmate.salary = math.ceil(2e3 * crewmate.xp) -- TODO: use this field somehow
    return crewmate
end

local function createEscortCompanion()
    local crewmate = createGenericCrewmate()
    crewmate.manager = createGenericCrewManagerComponent()
    crewmate.typetitle = "Companion" -- generic type, doesn't do anything but might pay rent
    crewmate.skill = "Escort"
    crewmate.satisfaction = rnd.rnd(1, 3)
    crewmate.threshold = 100e3 -- how much they need to be happy after doing a paid job

    crewmate.cost = math.floor(1e3 * crewmate.xp) -- the escort companion is cheaper than a generic manager, depends on starting xp
    -- should give us a fairly low salary, but we probably won't use this field often, or we'll have an exception or something
    crewmate.salary = math.floor(crewmate.xp * crewmate.satisfaction) * 10

    -- the escort companion is a bit more intimate with the captain and will
    -- talk about previous love affairs with characters of the opposite sex
    -- but for some translations, it might make sense to use something else like "my heart"
    -- used like "I wonder what she's up to these days"
    local opposite_article = _("she")
    -- 95% chance of female character to introduce large bias
    if rnd.rnd() < 0.95 then
        crewmate.gender = _("Female")
        crewmate.article_object = _("her")
        crewmate.article_subject = _("she")
        opposite_article = _("he")
        crewmate.portrait = portrait.getFemale()
    -- TODO give her a new name
    end

    -- customize the conversation table with new backstory
	crewmate.conversation.backstory = generateBackstory(crewmate)

	-- what I say when I'm doing my job
	crewmate.conversation.message = {"I would never kiss and tell."}
	-- what I say when I'm satisfied
	crewmate.conversation.satisfied = append_table(crewmate.conversation.satisfied, {
		_("I'm feeling positive."),
		_("Business is good."),
		_("Keep up the good work."),
		_("There's something about this place."),
		_("I hope you're all having a pleasant time."),
		_("I hope you're having a lovely time."),
		_("I'm really enjoying this ship."),
		_("I love this ship."),
		fmt.f(_("I'm taking a liking to this {ship}."), {ship = player.pilot():ship():name()}),
		_("I've got a good feeling, things are looking up."),
		_("I will lavish myself in luxury tonight."),
		_("I will enjoy some luxuries tonight."),
		_("I will reap my rewards tonight."),
		fmt.f(
			_("I recently acquired a sample of {made_up}'s latest youth serum. Would anyone care to try some?"),
			{made_up = getMadeUpName()}
		),
		_("I'm expecting a call from a customer soon, I'll spare you the details."),
		fmt.f(_("Did I tell you about the bison from {place}?"), {place = spob.get(faction.get("Dvaered"))}),
		_("I'm have a scheduled call with a client soon, I'll spare you the details."),
		_("I feel like we are on a winning streak."),
		_("I feel like we are on a lucky streak."),
		_("I feel like we are on a lucky roll."),
		_("Things are going alright, aren't they?"),
		_("Things are good, huh?"),
		_("Were any of you at the party last night? Wait, when was the party again? My sleep cycle is off again."),
		_("Overall, I'd say things are looking pretty good."),
		_("Sometimes there are bad times, but these aren't the worst of times."),
		_("Things have definitely been worse.")
	})
	-- what I say when not satisfied
	crewmate.conversation.unsatisfied = append_table(crewmate.conversation.unsatisfied, {
		_("I am quite unhappy."),
		_("I've been better."),
		_("I'm not feeling so well."),
		_("I worry about my financial situation."),
		_("I haven't had a good customer in far too long."),
		_("I haven't been able to visit my regular clients."),
		_("My customers are hungry, I don't know what to tell them."),
		_("Are we going anywhere nice soon?"),
		_("Please tell me we are headed towards civilization."),
		_("What in heavens are we even doing out here?"),
		_("My patience is wearing thin."),
		_("I would advise you to tread carefully."),
		_("Don't test me right now."),
		_("My patience is running out.")
	})
	
	-- maybe we don't want the generic things too here, idk
	-- things I say about {name} when I had a good conversation
	crewmate.conversation.good_talker = append_table(crewmate.conversation.good_talker, {
		_("I like talking with {name}."),
		_("I like {name}."),
		_("We've had some good conversations."),
		_("I think {name} likes me."),
		_("{name} is nice."),
		_("{name} seems nice."),
		_("{name} is a dear isn't {article_subject}."),
		_("Isn't {article_subject} a dear."),
		_("{name} is lovely."),
		_("That was pleasant."),
		_("That was pleasant of {article_object}."),
		_("That was nice of {article_object}."),
		_("Don't forget to enjoy the view. Look at that bright one over there. I think it's a moon."),
		_("On a happiness scale to somewhere around ten, I'd put you at around {satisfaction}."),
		_("That was pretty smart for someone with {skill} expertise."),
		_("I wouldn't have expected that from some {typetitle} with {skill} expertise.")
	})
	-- things I say about {name} when I had a bad conversation
	crewmate.conversation.bad_talker = append_table(crewmate.conversation.bad_talker, {
		_("{name} is too negative."),
		_("I dislike {name}."),
		_("I dislike {article_object}."),
		_("I'm not fond of {name}."),
		_("I'm not very fond of {article_object}."),
		_("I am not a fan of {name}."),
		_("I'm not friends with {name}."),
		_("I'm not friends with {article_object}."),
		_("I don't want to associate with {article_object}."),
		_("I don't enjoy my conversations with {article_object}."),
		_("I think {article_subject}'s being offensive."),
		_("Sometimes I wonder if you have any {skill} experience at all.")
	})
	
	crewmate.conversation.fatigue = append_table(crewmate.conversation.fatigue, {
		_("I hope we're going somewhere nice."),
		_("I hope we'll land soon, preferably somewhere nice."),
		_("It can be a bit lonely out here sometimes."),
		_("Will someone join me for a spa?"),
		_("I'm going to take a bath later if someone wants to join."),
		_("I could use a break."),
		_("I'm a bit tired."),
		_("I could use some rest."),
		_(
			"You know I need my wealthy planets, don't let me be the black sheep of the bunch that's bringing everyone down."
		)
	})
	crewmate.conversation.bar_actions = append_table(crewmate.conversation.bar_actions, {
		{
			["verb"] = pick_one(
				{
					_("seducing"),
					_("talking to"),
					_("gesturing at"),
					_("giggling at")
				}
			),
			["descriptor"] = pick_one(
				{
					_("some"),
					_("a"),
					_("a")
				}
			),
			["adjective"] = pick_one(
				{
					_("nice looking"),
					_("strange"),
					_("shady"),
					_("handsome"),
					_("dark"),
					_("mysterious"),
					_("preoccupied"),
					_("unknown")
				}
			),
			["object"] = pick_one(
				{
					_("stranger"),
					_("person"),
					_("man"),
					_("woman"),
					_("interloper"),
					_("dock worker"),
					_("crew member")
				}
			)
		}
	})
	-- special things I know how to say
	-- definitely overwrite whatever was in there before because we might pick
	-- random things from any topic here sometimes and we design the character
	-- "here" and not in the generic template
	crewmate.conversation.special = {
		["laugh"] = {
			_("*giggles*"),
			_("Hah!"),
			_("Haha!"),
			_("*laughs hysterically*"),
			_("*laughs briefly*")
		},
		["worry"] = {
			_("I hope I manage to secure a client on the next world."),
			_("Things just aren't as good as they used to be."),
			_("I'm growing increasingly concerned."),
			_("I feel as if I'm being reduced to nothing."),
			_("We are going to have to do something about that."),
			_("There are situations that need to be addressed."),
			_("From my viewpoint, things could be better."),
			_("I worry about the violence on this ship."),
			_("I'm a bit worried")
		}
	}
	crewmate.conversation.smalltalk_positive = append_table(crewmate.conversation.smalltalk_positive, {
		_("What a lovely view."),
		_("I'll be in my quarters."),
		_("I wish I could tell you about my last customer."),
		_("I like being surrounded by all this science."),
		_("Of all my travels, this is my favourite journey so far.")
	})
	-- unique negative smalltalk to be distinguishable from regular crew
	crewmate.conversation.smalltalk_negative = {
		_("I really need a break... To bathe myself in luxury."),
		_("I've seen better days."),
		_("The viewscreen in my quarters is malfunctioning, could you help me repair it?"),
		_("I'm getting tired of all these backwater worlds."),
		_("I'll be in my quarters."),
		_("Business could be better."),
		_("I wish I could tell you about my last customer..."),
		_("What? I don't want to talk about it."),
		_("I'm not making enough credits to keep up with my luxurious lifestyle.")
	}
	
	-- overwrite whatever topics we liked or disliked in our template
	-- list of things I like to talk about and what I say about them
	-- we'll start with a small set of topics/interests and therefore
	-- "be better at learning" because there's less noise to choose from
	crewmate.conversation.topics_liked = {
		-- list of phrases that use the things I like (or not)
		["luxury"] = {
			_("Do you want to see my new hat?"),
			_("How do you like this vintage neck-scarf?"),
			_("What do you think about this color?")
		},
		-- normally, I call this "friend", but I don't want the companion to always
		-- be talking about their friends or cat, at least not that often
		["friendship"] = {
			fmt.f(_("Check out this {ship} my friend thinking of buying."), {ship = getRandomShip()}),
			_("Do you want to see some pictures of my neice?"),
			_("I like how close we are."),
			_("I know we've had our differences, but you're alright."),
			_("I fear that we are becoming a bit too intimate."),
			_("Have I told you about my cat?"),
			_("Did I tell you about my cat?"),
			_("Do you want to see my kitty?"),
			_("Do you want to see my cat?"),
			_("Don't you just love my kitty?")
		},
		["travel"] = {
			fmt.f(
				_("One of my favourite places to visit is {place}. Have you been there?"),
				{place = spob.get(faction.get("Independent"))}
			),
			fmt.f(
				_("I fell in love with a pirate from {place}. I wonder what {article}'s up to these days."),
				{place = spob.get(faction.get("Raven Clan")), name = pilotname.human(), article = opposite_article}
			),
			fmt.f(
				_("If you've never been to {place}, we should go."),
				{place = spob.get(faction.get("Independent"))}
			),
			fmt.f(
				_("An intriguing place to visit is {place}. Have you been there?"),
				{place = spob.get(faction.get("Za'lek"))}
			),
			fmt.f(
				_("I heard that {place} is developing a new {made_up}. What do you make of that?"),
				{place = spob.get(faction.get("Za'lek")), made_up = getMadeUpName()}
			),
			fmt.f(
				_("Of all my travels I must say, I've been too often to {place}. I don't mind the work."),
				{place = spob.get(faction.get("Empire"))}
			),
			fmt.f(
				_(
					"I had an affair with a servant from {place}. I wonder what meddlesome {name} is up to these days."
				),
				{place = spob.get(faction.get("Dvaered")), name = pilotname.human()}
			),
			fmt.f(
				_(
					"All the violence and lawlessness on {place} led my sister {name} towards a path of disastrous affairs."
				),
				{place = spob.get(faction.get("Dvaered")), name = pilotname.human()}
			),
			fmt.f(
				_("I went to {place} just to check it out. I haven't had the urge to go since."),
				{place = spob.get(faction.get("Soromid"))}
			),
			fmt.f(
				_("I went to {place} just to check it out. I don't recommend it."),
				{place = spob.get(faction.get("Soromid"))}
			)
		},
		["view"] = {
			_("Did you enjoy the view?"),
			_("Did you notice the spectacular view towards the star?"),
			_("What a wonderful view. The stars are amazing."),
			_("What an amazing view!"),
			_("What are you looking at?"),
			_("Are you enjoying the view?"),
			_("Keep your hands to yourself or I'll have to charge you some credits."),
			_("I like looking out at the stars."),
			_("How could anyone not admire this view?")
		}
	}
	-- list of things I don't like talking about
	-- put a bunch of things related to violent thoughts here
	crewmate.conversation.topics_disliked = {
		_("violence"),
		_("credits"),
		_("fear"),
		_("death"),
		_("kill"),
		_("poor"),
		_("trash"),
	}
	-- things we say about things we are indifferent to
	crewmate.conversation.default_participation = append_table(crewmate.conversation.default_participation, {
		_("Of course."),
		_("Sure!"),
		_("That sounds good."),
		_("That sounds nice."),
		_("Sounds good."),
		_("Yeah."),
		_("Nice."),
		_("Alright."),
		_("I'm a bit busy, but I'll do what I can."),
		_("I'll do what I can."),
		_("I always do my best."),
		_("I'll do my best.")
	})
	-- responses to conversations about topics I don't like
	-- generally something dismissive but the companion is a bit diplomatic but can be dramatic
	crewmate.conversation.phrases_disliked = {
		_("Do we have to talk about this?"),
		_("All you ever talk about is about {topic}."),
		_("It's {topic} this, {topic} that, you just can't get enough {topic} can you?"),
		_("Whatever."),
		_("Yeah, okay."),
		_("I am not interested in that at all. Can we talk about something else?"),
		_("Sorry, not interested."),
		_("Right."),
		_("Sure."),
		_("Yeah, because of all the {topic}, of course."),
		_("Please give me some privacy."),
		_("I would like to be dismissed."),
		_("I have something else I have to do.")
	}
    
    crewmate.hook = {
        ["func"] = "escort",
        ["hook"] = nil
    }

    -- fix up the manager lines a bit to add some flavor
    table.insert(crewmate.manager.lines.satisfied, _("I've been noticing a lot of positivity among the crew."))
    table.insert(crewmate.manager.lines.satisfied, _("I think most of the crew is fairly happy."))
    table.insert(
        crewmate.manager.lines.satisfied,
        _("With a captain like you, it's no wonder we're all so happy. There's nothing to worry about.")
    )
    table.insert(
        crewmate.manager.lines.satisfied,
        _("You shouldn't be having any problems with this crew. Everyone seems to be perfectly happy.")
    )
    table.insert(
        crewmate.manager.lines.satisfied,
        _("I think the crew could use a break on a nice luxurious world, but for now there's nothing to worry about.")
    )
    table.insert(
        crewmate.manager.lines.satisfied,
        _("Don't worry about the crew, there's nothing wrong that a spa won't fix.")
    )
    table.insert(crewmate.manager.lines.satisfied, _("The crew seems fine. That's not what I'm worried about."))
    table.insert(crewmate.manager.lines.satisfied, _("I wouldn't worry about the crew, at least not for a while."))

    table.insert(crewmate.manager.lines.unsatisfied, _("The crew seems fine. That's not what I'm worried about."))
    table.insert(
        crewmate.manager.lines.unsatisfied,
        _("I wouldn't worry about the crew, but they are getting kind of tense.")
    )
    table.insert(crewmate.manager.lines.unsatisfied, _("There is a slight chance of some animosity between the crew."))
    table.insert(crewmate.manager.lines.unsatisfied, _("There is a slight chance of some hostility within the crew."))
    table.insert(
        crewmate.manager.lines.unsatisfied,
        _("With a captain like you, it's no wonder we're usually all so happy. I'm sure things will get better.")
    )
    table.insert(
        crewmate.manager.lines.unsatisfied,
        _("With a captain like you, it's a wonder the situation is so dire.")
    )

    crewmate.manager.lines.troublemaker = {
        _("Perhaps you should speak with {name}."),
        _("You need to listen to your crew."),
        _("{name} did complain about some things."),
        _("{name} has been complaining about some issues."),
        _("{name} has been mentioning problems."),
        _("{name} is being followed by trouble."),
        _("{typetitle} problems. Yes. We should talk to them."),
        _("The {typetitle} situation could be better."),
        _("I've heard some backtalk about {name}."),
        _("I have heard rumors."),
        _("We should probably keep a better eye on {name}."),
        _("There is some unresolved tension between {name} and the rest of the crew."),
        _("Let's not get into it right now. Let me just say that it's not looking good."),
        _("I don't want to point any fingers."),
        _("Don't say I didn't warn you. Can we leave it at that?")
    }

    return crewmate
end


local function createExplosivesEngineer(fac)
    local character = {}
    fac = fac or faction.get("Za'lek")

    local portrait_func = portrait.getMale
    character.gender = "Male"
    character.article_object = "him"
    character.article_subject = "he"
    -- only 35% chance of female character to introduce small bias
    if rnd.rnd() < 0.35 then
        character.gender = "Female"
        portrait_func = portrait.getFemale
        character.article_object = "her"
        character.article_subject = "she"
    end
    local _lastname, firstname = pilotname.human()
    character.name = pilotname.generic()
    character.firstname = firstname
    character.typetitle = "Engineer"
    character.skill = "Explosives Expert"
    character.satisfaction = rnd.rnd(1, 3)
    character.threshold = 100e3 -- how much they need to be happy after doing a paid job
    character.xp = math.floor(10 * (1 + rnd.sigma())) / 10
    character.portrait = portrait_func(portrait_arg)
    character.faction = fac
    character.chatter = 0.3 + rnd.threesigma() * 0.1 -- how likely I am to talk at any given opportunity (could be NEVER!)
    character.deposit = math.ceil(100e3 * character.satisfaction * character.xp + character.chatter * rnd.rnd() * 10e3)
    character.salary = 0
    character.other_costs = "equipment"
	-- the explosives expert is a bit of a weirdo and gets his own custom conversation sheet
	-- actually that's a good way for me to find out if refactoring is breaking stuff
    character.conversation = {
        ["backstory"] = generateBackstory(character),
        -- what I say when I'm doing my job
        ["message"] = {
            _("The bomb has been planted."),
            _("The fuse has been lit."),
            _("The fuse is set."),
            _("Explosives in place."),
            _("I've set the charges."),
            _("Bombs in place."),
            _("Kaboom!"),
            _("Boom!"),
            _("Let's go!"),
            _("Bombs away..."),
            _("Can we stay and watch this one? It's gonna blow in a bit."),
            _("Special delivery."),
            _("Who needs stern chasers when you've got explosives?"),
            _("She's set to blow."),
            _("Fire in the hole!"),
            _("Fire in the hole."),
            _("Fire. Hole."),
            _("Tick tick tick..."),
            _("Tick tock...")
        },
        -- what I say when I'm satisfied
        ["satisfied"] = {
            _("I'm happy."),
            _("What can I say? It's a good day."),
            _("I feel alive."),
            _("I feel so alive."),
            _("I feel great."),
            _("I feel fantastic."),
            fmt.f(_("I feel like {million}!"), {million = fmt.credits(1e6 * rnd.rnd(1, 10))})
        },
        -- what I say when not satisfied
        ["unsatisfied"] = {
            _("I am unhappy."),
            _("Everything is bleak."),
            _("I want to use my explosives."),
            _("I'm bored."),
            _("I'm so bored."),
            _("I'm really bored."),
            _("There's nothing to do around here."),
            _("Nothing ever happens around here."),
            _("Nothing ever happens on this ship."),
            _("There's no action on this ship."),
            _("This ship is so boring."),
            _("It's too quiet here.")
        },
        -- things I say about or back to {name} when I had a good conversation
        ["good_talker"] = {
            _("I like talking with {name}."),
            _("{name} is nice."),
            _("Well, {article_subject}'s nice."),
            _("This was nice."),
            _("Bitchin'."),
            _("Cool."),
            _("Awesome."),
            _("Kaboom."),
            _("Kaplow."),
            _("Boom!"),
            _("Bombs away!"),
            _("Radical! Pun intended."),
            _("Well that's pretty sweet."),
            _("Isn't thit nice."),
            _("Isn't thit lovely."),
            _("That's great."),
            _("Yeah, yeah. I know. I'm with you."),
            _("Yeah, I'm with you."),
            _("Yeah, I know. I'm with you."),
            _("Don't look at me, I'm with {article_object} on this.")
        },
        -- things I say about {name} when I had a bad conversation
        ["bad_talker"] = {
            _("{name} is a downer."),
            _("I don't like {name}. How many times do I have to say it?"),
            _("I don't like you, {name}."),
            _("Screw {name}."),
            _("To hell with {name}."),
            _("Bah!"),
            _("Screw it."),
            _("To hell with it."),
            fmt.f(
                _("That little {made_up} can go float {article_object}self."),
                {made_up = getMadeUpName(), article_object = "{article_object}"}
            ),
            _("To hell with {article_object}!"),
            _("Can you see the look I'm giving {article_object}?"),
            _("{name} gets to do anything {article_subject} likes."),
            _("Everything is always about {name}, isn't it?"),
            _("Everything is about {article_object}, isn't it?")
        },
        ["fatigue"] = {
            _("Are we going to do something anytime soon?"),
            _("It's cold out there in space. Even with the explosions."),
            _("I want more explosions."),
            _("I could use a drink."),
            _("I need a drink."),
            _("I need a drink, damn it!"),
            _("I'm kind of beat."),
            _("I could use some shut-eye."),
            _("Where's my hat? Someone bring me my hat!"),
            _(""),
            _(""),
            _(""),
            _(""),
            _(""),
            _(""),
            _(""),
            _("")
        },
        ["bar_actions"] = {
            {
                ["verb"] = pick_one(
                    {
                        _("seducing"),
                        _("talking to"),
                        _("gesturing at"),
                        _("staring at")
                    }
                ),
                ["descriptor"] = pick_one(
                    {
                        _("some"),
                        _("a"),
                        _("a")
                    }
                ),
                ["adjective"] = pick_one(
                    {
                        _("tall"),
                        _("strange"),
                        _("shady"),
                        _("handsome"),
                        _("dark"),
                        _("mysterious"),
                        _("preoccupied")
                    }
                ),
                ["object"] = pick_one(
                    {
                        _("stranger"),
                        _("person"),
                        _("vagabond"),
                        _("piece of art"),
                        _("interloper"),
                        _("hologram"),
                        _("animal")
                    }
                )
            },
            {
                ["verb"] = pick_one(
                    {
                        _("motioning"),
                        _("signaling"),
                        _("gesturing"),
                        _("communicating")
                    }
                ),
                ["descriptor"] = pick_one(
                    {
                        _("with his"),
                        _("by waving his"),
                        _("by moving his")
                    }
                ),
                ["adjective"] = pick_one(
                    {
                        _("hand"),
                        _("hands"),
                        _("arms"),
                        _("legs"),
                        _("fingers"),
                        _("ears"),
                        _("eyes"),
                        _("eyebrows"),
                        _("tongue")
                    }
                ),
                ["object"] = pick_one(
                    {
                        _("in the air"),
                        _("towards a table"),
                        _("over his chest"),
                        _("around his face"),
                        _("inconsistently"),
                        _("like a maniac"),
                        _("without regard to his surroundings")
                    }
                )
            }
        },
        -- special things I know how to say
        ["special"] = {
            ["laugh"] = {
                _("*laughs maniacally*"),
                _("*laughs hysterically*"),
                _("*laughs frantically*"),
                _("Heh."),
                _("*chuckles*"),
                _("*cackles*"),
                _("Hehe."),
                _("Right? *laughs*"),
                _("*laughs*"),
                _("Am I right? Anyone?"),
                _("Right?"),
                _("Am I right or what?"),
                _("Yallahahahaha!"),
                _("*coughing laughter*"),
                _("*laughs while coughing*"),
                _("*asphyxiating laughter*"),
                _("Whoops, where did my lucky cigar go?")
            },
            ["random"] = {
                _("Tick tock!"),
                _("Shakalakalaka!"),
                _("Kaboom!"),
                _("BOOM!"),
                _("Aha!"),
                _("Tick tock..."),
                _("Tickety tock..."),
                _("It's time to say goodnight."),
                _("Close your eyes, it's gonna get bright.")
            },
            ["worry"] = {
                _("I hope I remembered to set the fuse..."),
                _("Oh, wait a minute..."),
                _("Did I get that right?"),
                _("At least I hope I'm right."),
                _("If everything goes to plan."),
                _("Maybe."),
                _("I think."),
                _("I think..."),
                _("I'm pretty sure."),
                _("Time will tell."),
                _("My lucky cigar never fails me.")
            }
        },
        ["smalltalk_positive"] = {
            _("Do you guys remember the last ship we boarded?"),
            _("What was the name of that last ship? She went down beautifully."),
            _("I'll be in my quarters if you need me."),
            _("I'm going to go hang out with the cargo."),
            _("Let me know if you need anything.")
        },
        ["smalltalk_negative"] = {
            _("It's been a while since we paid anyone a special kind of visit, if you know what I mean."),
            _("Why don't we ever get dirty anymore?"),
            _("Do you guys remember the last ship we boarded?")
        },
        -- list of things I like to talk about and what I say about them
        ["topics_liked"] = {
            -- list of phrases that use the things I like (or not)
            ["violence"] = {
                fmt.f(_("Check out this {ship} I got to destroy."), {ship = getRandomShip()}),
                fmt.f(_("Have I told you about the {ship} I destroyed during my training?"), {ship = getRandomShip()})
            },
            ["friendship"] = {
                fmt.f(_("Check out this {ship} I'm thinking of buying."), {ship = getRandomShip()}),
                fmt.f(_("Check out the custom paintjob on this {ship}!"), {ship = getRandomShip()}),
                fmt.f(_("My old friend {name} would love this."), {name = pilotname.human()}),
                fmt.f(_("I'm sure {name} would appreciate this place."), {name = pilotname.human()})
            },
            ["science"] = {
                fmt.f(_("Check out the landing gear on this {ship}!"), {ship = getRandomShip()}),
                fmt.f(_("Have you seen the new {ship} features?"), {ship = getRandomShip()}),
                fmt.f(_("I heard about some unexplained phenomena at {place}."), {place = spob.get(true)}),
                fmt.f(
                    _("I wonder what the mystery about {place} is, maybe I missed something."),
                    {place = spob.get(true)}
                ),
                fmt.f(
                    _(
                        "I thought I saw a {ship} following me past {place}, my sensors were going crazy, but then I saw it with my own eyes. It was a comet!"
                    ),
                    {ship = getRandomShip(), place = spob.get(true)}
                )
            }
        },
        -- list of things I don't like talking about, words I don't like hearing
        ["topics_disliked"] = {
            _("luxury"),
            _("luxurious"),
            _("soap"),
            _("lotion")
        },
        -- things we say about things we are indifferent to
        ["default_participation"] = {
            _("Err.. Yeah!"),
            _("Sure!"),
            _("That sounds good."),
            _("Yeah."),
            _("Nice."),
            _("Boom, baby!")
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
            _("Yeah, because of all the {topic}, of course.")
        }
    }
    character.hook = {
        ["func"] = "demoman",
        ["hook"] = nil
    }

    return character
end

local function createPilotNPCs()
    local fac = spob.cur():faction()

    if spob.cur():tags.nonpc then
        return
    end

    if fac == nil then
        return
    end

    local r = rnd.rnd(0, 3)

    -- the generic crewman disguised as a patron
    if true or rnd.rnd(1, 5) == 3 then
        local crewmate = createGenericCrewmate()
        local id =
            evt.npcAdd(
            "approachGenericCrewmate",
            "Patron",
            crewmate.portrait,
            fmt.f(
                _(
                    [[This person seems to be looking for work, but there are no obvious details as to what they can do.]]
                ),
                crewmate
            ),
            9
        )

        npcs[id] = crewmate
    end

    -- the generic manager disguised as a patron
    if true or rnd.rnd(1, 13) == 7 then
        local crewmate = createGenericManager()
        local id =
            evt.npcAdd(
            "approachGenericCrewmate",
            "Patron",
            crewmate.portrait,
            fmt.f(
                _(
                    [[This person seems to be looking for work, but there are no obvious clues as to what they can do.

		Name: {name}
		]]
                ),
                crewmate
            ),
            8
        )

        npcs[id] = crewmate
    end

    -- the special crewmate, pick 1 or none
    -- the demolition man
    if r == 1 or (fac == faction.get("Za'lek") and r == 0) then
        local character = createExplosivesEngineer()

        local id =
            evt.npcAdd(
            "approachDemolitionMan",
            character.typetitle,
            character.portrait,
            fmt.f(
                _(
                    [[This engineer seems to be looking for work.

		Name: {name}
		Post: {typetitle}
		Expertise: {skill}
		...]]
                ),
                character
            ),
            5
        )

        npcs[id] = character
    end

    -- the companion escort, rare , less rare on criminal worlds
    if r == 2 or spob.cur():tags().criminal and r == 0 then
        local character = createEscortCompanion()
        character.faction = fac
        character.chatter = 0.5 + rnd.threesigma() * 0.1 -- how likely I am to talk at any given opportunity
        character.deposit = math.ceil(200e3 * character.xp + 35e3 * character.satisfaction)
        character.salary = 0 -- credits per cycle? sure.. credits per cycle.
        character.other_costs = "Luxury Goods" -- pay for 100 kg every time you land

        local id =
            evt.npcAdd(
            "approachEscortCompanion",
            character.typetitle,
            character.portrait,
            fmt.f(
                _(
                    [[This person seems charming and charismatic. You get the feeling that you're about to be pursuaded into some business. Perhaps you should strike up a conversation?

		Name: {name}
		]]
                ),
                character
            ),
            5
        )

        npcs[id] = character
    end
end

local function getOfferText(edata)
    local approachtext = generateIntroduction(edata)

    local _credits, scredits = player.credits(2)
    local deposit = edata.deposit
    if not deposit then
        deposit = 0
    end
    local credentials = _([[
Name: {name}
Expertise: {skill}
]])

    local finances = _([[
Money: {credits}
Deposit: {deposit}
Salary: {salary}
Other costs: {other_costs}]])
    return (approachtext ..
        "\n\n" ..
            fmt.f(credentials, edata) ..
                "\n\n" ..
                    fmt.f(
                        finances,
                        {
                            credits = scredits,
                            deposit = fmt.credits(deposit),
                            salary = fmt.credits(edata.salary),
                            other_costs = edata.other_costs
                        }
                    ))
end

function create()
    npcs = {}
    mem.companions = {}
    mem.costs = {}

    hook.land("land")
    hook.load("land")
    hook.enter("enter")
end

function land()
    npcs = {}
    local paid = {}
    for i, edata in ipairs(mem.companions) do
        -- natural satisfaction adjustment gravitates towards zero and adds
        -- a little bit of randomness based on how smooth the landing was or whatever
        local sss = edata.satisfaction
        edata.satisfaction = math.floor(10 * (sss - (sss / 64))) / 10 + 0.01 * rnd.threesigma()

        -- incurr any necessary commodity costs if we have the possibility to "restock"
        if edata.other_costs then
            -- see if this is a commodity we can buy here
            local available_comms = spob.cur():commoditiesSold()
            for _i, ccom in ipairs(available_comms) do
                local name = ccom:name()
                if name == edata.other_costs then
                    -- make the player pay the cost of 1/10th of a ton, which should be enough to last until we land again
                    local price = math.ceil(ccom:priceAt(spob.cur()) / 10)
                    player.pay(-price)
                    -- do some bookkkeeping in case we have a manager
                    local prev_paid = paid[name]
                    if prev_paid == nil then
                        prev_paid = 0
                    end
                    paid[name] = prev_paid + price
                -- found = true -- TODO: if we can't buy it here, incurr costs to some budget or something
                end
            end
        end

        -- if we don't have a manager that called us, some crew doesn't go to the bar
        -- chance of being elsewhere depends on xp and crew size
        local elsewhere_chance =
            (#mem.companions - edata.satisfaction) / (edata.xp + #mem.companions + math.abs(edata.satisfaction))
        if edata.manager then
            elsewhere_chance = 0
        end -- don't let managers go on holiday
        if mem.summon_crew or rnd.rnd() > elsewhere_chance or edata.manager then
            -- add the npc and figure out what he's doing
            local doing = getBarSituation(edata) .. "."
            local description =
                fmt.f(_("This is {firstname} {name}, one of your crew. It seems that {article_subject} is "), edata) ..
                doing
            -- little hack fix here
            local priority = 10
            if edata.manager then
                priority = 6
            end
            local id = evt.npcAdd("approachCompanion", edata.name, edata.portrait, description, priority)
            npcs[id] = edata
        end
    end

    -- if we just summoned the crew, don't do it again next time
    mem.summon_crew = nil

    if #mem.companions <= 0 then
        evt.save(false)
    end

    -- Ignore on uninhabited and planets without bars
    local pnt = spob.cur()
    local services = pnt:services()
    local flags = pnt:flags()
    if not services.inhabited or not services.bar or flags.nomissionspawn then
        return
    end

    -- pay for any other incurred costs

    for item, cost in pairs(mem.costs) do
        player.pay(-cost)
        paid[item] = cost
        mem.costs[item] = 0
    end

    -- if we have a manager, give him the data here
    for _i, crewmate in ipairs(mem.companions) do
        if crewmate.manager then
            -- check if the manager has the finance skill
            if crewmate.manager.skill == "finance" then
                crewmate.manager.paid = paid
            end
        -- check for some other skills / data
        end
    end

    local total_paid = 0
    for _item, ppaid in pairs(paid) do
        total_paid = total_paid + ppaid
    end
    if total_paid > 0 then
        shiplog.append(
            logidstr,
            fmt.f(
                _("You paid {credits} in crew salaries and other costs."),
                {
                    credits = fmt.credits(total_paid)
                }
            )
        )
    end

    -- Create NPCs for pilots you can hire.
    createPilotNPCs()
end

function enter()
    -- if escorts are disabled, our companions are sleeping
    if var.peek("hired_escorts_disabled") then
        return
    end

    if #mem.companions == 0 then
        return
    end

    -- start a conversation
    hook.rm(mem.conversation_hook)
    mem.conversation_hook = hook.timer(rnd.rnd(10, 30), "start_conversation")

    for _i, companion in ipairs(mem.companions) do
        -- reset any hooks
        if companion.hook then
            if companion.hook.hook then
                --				print(fmt.f("removing hook number {number} for {name}", {number = companion.hook.hook, name = companion.name}))
                hook.rm(companion.hook.hook)
            end
            companion.hook.hook = entries[companion.hook.func](companion)
        --			print(fmt.f("registered hook number {number} for {name}", {number = companion.hook.hook, name = companion.name}))
        end
    end

    -- set the fatigue hook
    hook.rm(mem.fatigue_hook)
    mem.fatigue_hook = hook.date(time.create(0, 1, 0), "period_fatigue", nil)
end

-- a period passes in space and the crew feels fatigued
function period_fatigue()
    local hysteria = false
    for _i, companion in ipairs(mem.companions) do
        -- every experience point will give the crewmate 1% chance to resist fatigue
        if rnd.rnd(0, 100) < companion.xp then
            companion.satisfaction = companion.satisfaction - 0.01
            -- if we are a big chatter we might express ourselves about this later
            if rnd.rnd() < companion.chatter then
                local last_sentiment = companion.conversation.sentiment
                companion.conversation.sentiment = pick_one(companion.conversation.fatigue)
                -- if it's the same sentiment, blurt it out soon
                if companion.conversation.sentiment == last_sentiment then
                    hook.timer(7 + rnd.rnd(3, 25), "say_specific", {me = companion, message = last_sentiment})
                    companion.conversation.sentiment = nil
                end
            end
        elseif rnd.threesigma() > 2.5 and not hysteria then
            -- we get a mild case of space hysteria that affects us more the more experienced we are
            companion.satisfaction = companion.satisfaction - companion.xp / (companion.xp + 6)
            print(fmt.f("{name} has hysteria.", companion))

            -- ramble at some victim (could be ourselves, especially on small crews)
            local victim = pick_one(mem.companions)

            -- start rambling about something
            local ramblings

            if rnd.rnd(0, 1) == 0 then
                -- we get lucky, we realize we're just tired, but we're still going to ramble
                ramblings = pick_one(companion.conversation.fatigue)
                -- create a random memory about this scary place
                create_memory(companion)
            else
                -- decide how to ramble
                if rnd.rnd(0, 1) == 0 then
                    -- we'll call the victim bad company for no reason
                    ramblings = fmt.f(pick_one(companion.conversation.bad_talker), victim)
                else -- oh we're really gonna ramble
                    if rnd.rnd(0, 1) == 0 then
                        -- we'll pick anything from our special choices and just say that
                        ramblings = add_special(companion)
                    else -- be a little more incoherent than usual
                        ramblings = add_special(companion) .. " " .. getMadeUpName() .. " " .. add_special(companion)
                    end
                    -- just in case we got no specials for some reason
                    if ramblings:len() == 0 then
                        ramblings = fmt.f(_("This voyage is driving me {made_up} crazy."), {made_up = getMadeUpName()})
                    end
                end
                -- experience melancholia too because we later learn how incoherent we were
                companion.satisfaction = companion.satisfaction - 1
                -- at this point, it's safe to say one of the crewmates is experiencing hysteria, don't add any more
                hysteria = true
                -- create a random memory but supplying some completely incorrect parameters
                local params = {
                    system = getMadeUpName(),
                    target = getMadeUpName(),
                    credits = fmt.credits(-rnd.rnd(3e3, 7e4)),
                    armour = rnd.rnd(44, 132),
                    ship = getRandomShip()
                }
                create_memory(companion, "hysteria", params)
            end

            -- set the sentiment so that we'll tell it to someone
            companion.conversation.sentiment = ramblings
            -- start talking to the victim (remember, could be ourselves, and we could start a conversation with ourselves)
            speak(companion, victim)
        end
    end

    local next_fatigue = rnd.rnd(7500, 9950)
    -- set the next period fatigue timer
    hook.rm(mem.fatigue_hook)
    mem.fatigue_hook = hook.date(time.create(0, 0, next_fatigue), "period_fatigue", nil)
end

-- starts a standard discussion with a crewmate (at the bar, unless the ship has a bridge UI where you can talk to your npcs)
-- this is definitely a place to be excessively wasteful and do computations we might not need if it increases the chance
-- of a more meaningful interaction. If we have to look every word typed by the player up in several tables, then that's what we'll do!
local function startDiscussion(crewmate)
    -- just pick a random thing to say from our interests
    local my_topics = crewmate.conversation.topics_liked
    local topic = nil
    local last_topic = nil
    for ttt, _choices in pairs(crewmate.conversation.topics_liked) do
        if not topic or rnd.rnd(0, 3) == 1 then
            topic = ttt
        end
        last_topic = ttt
    end
    local message
    if not topic then
        message = _("I got nothing to say to you.")
    else
        local choices = my_topics[topic]
        message = pick_one(choices)
    end

    if not message then
        message = _("I don't know what to say.")
    end

    -- to make it more interesting, sometimes pick another thing to say as well
    if rnd.rnd(0, 2) == 0 then
        local sep = "\n"
        -- our last topic, which is probably going to be the one that's last in the list in this file that got chosen, will be preferred and more likely
        if rnd.rnd(0, 2) == 1 then
            last_topic = topic -- use the same topic again
            sep = " " -- don't always seperate with newline, it's the same topic for sure ihere so lets avoid it
        end
        local choices = my_topics[last_topic]
        local other_message = pick_one(choices)
        -- don't say the exact same thing twice though
        if message ~= other_message then
            message = message .. sep .. other_message
        end
    end

    local name_label = fmt.f("{firstname} {name}", crewmate)
    vntk.msg(name_label, message)
    local spoken = tk.input(_("Conversation"), 0, 64, _("What do you say back?"))
    if spoken then
        local appreciation, understood = appreciate_spoken(spoken, crewmate)
        if not understood then
            -- we didn't even understand this, lets increase the chance of an appropriate response
            local responses = {
                _("Whatever."),
                _("Yeah, okay."),
                fmt.f(_("Okay, {name}."), {name = player.name()}),
                fmt.f(_("Aright, {name}."), {name = player.name()}),
                _("Sure."),
                _("Oh, really?"),
                _("Sorry?"),
                _("What?"),
                _("Huh?"),
                _("I'm a little hard of hearing."),
                _("Oh, sure."),
                _("I didn't quite catch that."),
                _("I'm afraid I don't really know what you're saying."),
                _("I'm afraid that I don't quite understand you."),
                _("I don't know."),
                _("Yeah... It is what it is."),
                _("Carpe diem!"),
                _("Let's check in with the others."),
                _("*sips drink*"),
                add_special(crewmate, "laugh"),
                add_special(crewmate),
                _("Sorry, are you talking to me?"),
                _("Sorry, can you repeat that?"),
                fmt.f(_("I'm sorry {name}, I wasn't listening."), {name = player.name()}),
                fmt.f(_("I'm sorry {name}, can you rephrase that?"), {name = player.name()}),
                _("Sometimes you just gotta... Yeah, I don't know, sorry, I wasn't really listening."),
                _("Sorry, I'm a bit distracted."),
                _(
                    "Look, can we talk about something I'm actually knowledgable about? I feel like you're trying to set me up to look like a fool."
                ),
                _("Yeah, I don't know much about that."),
                _("I don't know much about that."),
                _("I don't know anything about that."),
                _("I don't know anything about it."),
                _("I don't know what you're talking about."),
                _("I don't know what you're saying."),
                _("I'm not very knowledgable about those things."),
                _("I'm not very knowledgable about these things."),
                _("I'm not interested in that."),
                _("I never think about that."),
                _("What's gotten into you?"),
                _("Let's just grab a drink, shall we?"),
                _("How about we just forget about all this?"),
                _("You're not thinking of firing me, are you?"),
                _("What's going on? I'm so confused."),
                _("Sometimes I just don't understand you."),
                _("You can be difficult to understand sometimes.")
            }
            responses = append_table(responses, appreciation)
            if crewmate.conversation.sentiments then
                -- a chance of changing the subject
                responses = append_table(responses, crewmate.conversation.sentiments)
            else
                -- try to be a bit smarter than usual and enlist help from a function
                responses = append_table(responses, generate_responses(spoken, crewmate))
            end
            vntk.msg(name_label, pick_one(responses))
        else
            vntk.msg(name_label, pick_one(appreciation))
        end
    end
end

-- starts a managementarial discussion
local function startManagement(edata)
    -- woah, we are a manager! lets do our manager thing
    local management = edata.manager

    -- if we can't afford our manager's services...
    if management.cost and player.credits() < management.cost then
        vntk.msg(
            fmt.f("{typetitle} {name}", edata),
            fmt.f(
                _(
                    "You don't have the {credits} you owe me for previous management and assessment services. Maybe you should work on one problem at a time."
                ),
                {credits = fmt.credits(management.cost)}
            )
        )
        return
    end

    -- we are a personnel manager, let's give a personnel assessment
    if string.find(management.type, "Personnel") then
        local key, troublemaker = crewManagerAssessment()
        if not troublemaker then
            troublemaker = {}
        end
        local message = fmt.f(pick_one(management.lines[key]), troublemaker)
        vntk.msg(fmt.f("{name}", edata), message)
    end
    -- maybe we are an unknown kind of manager, then nothing happens

    if management.cost > 0 then
        -- if we want to summon the crew with our manager it has to cost something
        -- let the player how much he paid for this service
        if
            not mem.summon_crew and
                vntk.yesno(
                    fmt.f("{typetitle}", edata),
                    fmt.f(
                        _(
                            "Would you like to summon the crew to be available for discussion at the bar at the next destination? This will cost {credits}."
                        ),
                        {credits = fmt.credits(management.cost)}
                    )
                )
         then
            mem.summon_crew = true
            player.pay(-management.cost)
            shiplog.append(
                logidstr,
                fmt.f(
                    _("You paid {credits} in crew management fees."),
                    {
                        credits = fmt.credits(management.cost)
                    }
                )
            )
        end
    end
end

-- starts a conversation with the companion
local function startConversation(companion)
    local introduction
    -- what does the companion want to talk about
    -- if satisfaction is low, be negative and brief
    if companion.satisfaction < 0 then
        introduction = pick_one(companion.conversation.unsatisfied)
    else
        -- if satisfaction is high, be positive and verbose (extra greeting, etc)
        local greeting =
            pick_one(
            {
                _("Hello, Captain."),
                _("Oh. Hi there.") -- default neutral
            }
        )
        -- use chatter variable to determine verbosity TODO
        introduction = greeting .. "\n\n" .. pick_one(companion.conversation.satisfied)
    end

    if companion.conversation.sentiment then
        introduction = introduction .. " " .. companion.conversation.sentiment
    end

    return introduction
end

-- Asks the player whether or not they want to fire the pilot
local function crewmate_barConversation(edata, npc_id)
    local managing = ""
    if edata.manager then
        managing = edata.manager.type
    end
    if not edata.firstname then
        edata.firstname = "Mysterious"
    end -- generic dreadful nick, but better than "Gendrick"

    local praise_price = math.ceil(5 * edata.xp * edata.satisfaction)
    local scold_price = math.floor(20 * edata.xp + edata.satisfaction)

    local name_label = fmt.f("{firstname} {name}", edata)
    local n, _s =
        tk.choice(
        name_label,
        startConversation(edata),
        fmt.f(_(" Discuss {managing}"), {managing = managing}),
        fmt.f(_("Give Praise ({credits})"), {credits = fmt.credits(praise_price)}),
        fmt.f(_("Reprimand ({credits})"), {credits = fmt.credits(scold_price)}),
        fmt.f(_("Fire {typetitle}"), edata),
        _("Do nothing")
    )
    if n == 1 then -- Manager stuff
        -- if we are a manager, do the manager thing, otherwise, say a random thing
        if edata.manager then
            startManagement(edata)
        else
            -- start a one on discussion with this crewmate
            startDiscussion(edata)
        end
    elseif n == 2 then -- praise
        -- some responses specific to the praise
        local responses = {
            _("Thanks!"),
            _("Yeah, okay."),
            fmt.f(_("Okay, {name}."), {name = player.name()}),
            fmt.f(_("Aright, {name}. Thanks for the feedback."), {name = player.name()}),
            _("Thank you.")
        }
        -- default responses
        responses = append_table(responses, edata.conversation.default_participation)
        -- add some interesting responses
        if edata.conversation.sentiment then
            table.insert(responses, edata.conversation.sentiment)
        end
        if edata.conversation.sentiments then
            responses = append_table(responses, edata.conversation.sentiments)
        end
        -- reply to the captain
        vntk.msg(name_label, pick_one(responses))

        -- adjust the sentiment
        edata.conversation.sentiment =
            fmt.f(
            pick_one(edata.conversation.good_talker),
            {name = player.name(), article_subject = _("the captain"), article_object = _("the captain")}
        )

        -- adjust the chatter trying to increase it
        edata.chatter = math.max(edata.chatter, math.min(0.66, edata.chatter + 0.1 + 0.1 * rnd.threesigma()))
    elseif n == 3 then -- criticize
        -- responses specific to the criticism
        local responses = {
            _("Okay captain."),
            _("Yeah, okay."),
            fmt.f(_("Okay, {name}. I'm sorry to hear that."), {name = player.name()}),
            fmt.f(_("Aright, {name}. Thanks for the feedback."), {name = player.name()}),
            _("Sorry."),
            _("I'll try and do better next time."),
            _("I'll try and learn from the others."),
            _("I'm sorry sir."),
            _("I'll take a step back."),
            _("Whatever you say sir."),
            _("Oh, man. Good to know, I guess.")
        }
        -- default responses
        responses = append_table(responses, edata.conversation.default_participation)
        -- maybe let's just talk about violence
        if edata.conversation.topics_liked.violence then
            responses = append_table(responses, edata.conversation.topics_liked.violence)
        end
        -- reply to the captain
        vntk.msg(name_label, pick_one(responses))

        -- adjust the sentiment
        edata.conversation.sentiment =
            fmt.f(
            pick_one(edata.conversation.bad_talker),
            {name = player.name(), article_subject = _("the captain"), article_object = _("the captain")}
        )

        -- adjust the chatter trying to push it down
        edata.chatter = math.min(edata.chatter, math.max(0.16, edata.chatter - 0.1 - 0.1 * rnd.threesigma()))
    elseif n == 4 and vntk.yesno("", fmt.f(_("Are you sure you want to fire {name}? This cannot be undone."), edata)) then
        -- reply to the captain or storm off, depending on whether we know violence, have friends, or neither
        if edata.conversation.topics_liked.violence then
            -- talk about violence if possible
            vntk.msg(name_label, pick_one(edata.conversation.topics_liked.violence))
        elseif edata.conversation.topics_liked.friend then
            -- remisince about a friend one last time before the captain
            vntk.msg(name_label, pick_one(edata.conversation.topics_liked.friend))
        end

        for k, v in ipairs(mem.companions) do
            if edata.name == v.name then
                if edata.hook then
                    hook.rm(edata.hook.hook)
                end
                mem.companions[k] = mem.companions[#mem.companions]
                mem.companions[#mem.companions] = nil
            end
        end
        if npc_id then
            evt.npcRm(npc_id)
            npcs[npc_id] = nil
        end

        shiplog.append(logidstr, fmt.f(_("You fired '{name}'."), edata))
    end
end

-- Approaching hired pilot at the bar
function approachCompanion(npc_id)
    local edata = npcs[npc_id]
    if edata == nil then
        evt.npcRm(npc_id)
        return
    end

    crewmate_barConversation(edata, npc_id)
end

-- Approaching a completely generic crewmate
function approachGenericCrewmate(npc_id)
    local pdata = npcs[npc_id]
    if pdata == nil then
        evt.npcRm(npc_id)
        return
    end

    if not vntk.yesno("", getOfferText(pdata)) then
        return -- Player rejected offer
    end

    if pdata.deposit and pdata.deposit > player.credits() then
        vntk.msg(_("Insufficient funds"), _("You don't have enough credits to pay for this person's deposit."))
        return
    end

    -- check if this ship has this kind of manager
    if pdata.manager then
        for _i, pers in ipairs(mem.companions) do
            if pers.manager and pers.manager.type == pdata.manager.type then
                -- TODO : generate a rejection
                vntk.msg(
                    _("No thanks"),
                    _("You look like you're pretty well staffed. I'll find another ship that needs me.")
                )
                return
            end
        end
    end

    if pdata.deposit then
        player.pay(-pdata.deposit, true)
    end

    local num_crewmates = #mem.companions

    local i = #mem.companions + 1
    if i / 2 >= player.pilot():stats()["crew"] == "Crew" then
        local params = {
            ["start"] = {
                _("Oh hey,"),
                _("Well,"),
                _("Actually, upon closer inspection")
            },
            ["reason"] = {
                _("it kind of looks like your ship has too much crew on it already."),
                _("I don't know if you have the facilities for another crew member."),
                _("I think it would be better if I joined a different ship.")
            },
            ["excuse"] = {
                _("I can tell you're only trying to be polite, but there's obviously no room for me on your ship."),
                _("I'm sure I'll find something else."),
                _("It's not you or your ship, I just can't work with so many people.")
            },
            ["bye"] = {
                _("Later."),
                _("I'll see you around."),
                _("Catch you later."),
                _("Sorry.")
            }
        }
        vntk.msg(_("No thanks"), fmt.f(_("{start} {reason} {excuse} {bye}"), params))
        return
    end
    mem.companions[i] = pdata
    evt.npcRm(npc_id)
    npcs[npc_id] = nil
    local id =
        evt.npcAdd(
        "approachCompanion",
        pdata.name,
        pdata.portrait,
        fmt.f(_("{name} is a member of your crew."), pdata),
        9
    )
    npcs[id] = pdata
    evt.save(true)

    local edata = mem.companions[i]
    shiplog.create(logidstr, _("Ship Companions"), _("Ship Companions"))
    shiplog.append(logidstr, fmt.f(_("You hired '{name}' to join your crew."), edata))
end

-- Approaching unhired companion escort at the bar
function approachEscortCompanion(npc_id)
    local pdata = npcs[npc_id]
    if pdata == nil then
        evt.npcRm(npc_id)
        return
    end

    if not vntk.yesno("", getOfferText(pdata)) then
        return -- Player rejected offer
    end

    if pdata.deposit and pdata.deposit > player.credits() then
        vntk.msg(_("Insufficient funds"), _("You don't have enough credits to pay for this person's deposit."))
        return
    end

    if pdata.deposit then
        player.pay(-pdata.deposit, true)
    end

    -- this can be generalized to an attribute like unique
    -- check if this ship has an escort
    for _i, pers in ipairs(mem.companions) do
        if pers.skill == "Escort" then
            -- TODO : generate a rejection
            vntk.msg(
                _("No thanks"),
                _(
                    "You already have an escort on your ship. I need my space. I need my privacy. I need my customers. I'll find another ship."
                )
            )
            return
        end
    end

    local i = #mem.companions + 1
    mem.companions[i] = pdata
    evt.npcRm(npc_id)
    npcs[npc_id] = nil
    local id =
        evt.npcAdd(
        "approachCompanion",
        pdata.name,
        pdata.portrait,
        fmt.f(_("{name} lives on your ship with the crew."), pdata),
        8
    )
    npcs[id] = pdata
    evt.save(true)

    local edata = mem.companions[i]
    shiplog.create(logidstr, _("Ship Companions"), _("Ship Companions"))
    shiplog.append(logidstr, fmt.f(_("You allowed '{name}' to live on your ship with your crew."), edata))
end

-- Approaching unhired demo man at the bar
function approachDemolitionMan(npc_id)
    local pdata = npcs[npc_id]
    if pdata == nil then
        evt.npcRm(npc_id)
        return
    end

    if not vntk.yesno("", getOfferText(pdata)) then
        return -- Player rejected offer
    end

    if pdata.deposit and pdata.deposit > player.credits() then
        vntk.msg(_("Insufficient funds"), _("You don't have enough credits to pay for this person's deposit."))
        return
    end

    for _i, pers in ipairs(mem.companions) do
        if pers.skill == "Explosives Expert" then
            -- TODO : generate a rejection
            vntk.msg(
                _("No thanks"),
                _(
                    "There's no room for two pyromaniacs on one ship. I'll save you the trouble and get out of your hair."
                )
            )
            evt.npcRm(npc_id)
            return
        end
    end

    if pdata.deposit then
        player.pay(-pdata.deposit, true)
    end

    local i = #mem.companions + 1
    mem.companions[i] = pdata
    evt.npcRm(npc_id)
    npcs[npc_id] = nil
    local id = evt.npcAdd("approachCompanion", pdata.name, pdata.portrait, _("This is one of your crewmates."), 8)
    npcs[id] = pdata
    evt.save(true)

    local edata = mem.companions[i]
    shiplog.create(logidstr, _("Ship Companions"), _("Ship Companions"))
    shiplog.append(logidstr, fmt.f(_("You hired '{name}' to join your crew."), edata))
end

-- the player lands on a world with a companion escort on board
function escort_landing(speaker)
    -- used like "why are we on this pick(<descriptors>) anyway"
    local bad_tags = {
        ["garbage"] = {"dump", "literal garbage dump", "floating space turd", "scrapheap", "landfill"},
        ["mining"] = {"mining world", "low-brow planet", "terrible rock", "forsaken place", "labour camp"},
        ["poor"] = {"destitute world", "forsaken ground", "filthy rock", "scrapheap", "misery farm", "labour camp"}
    }
    -- used like "<tag> <descriptor> can actually be quite lucrative"
    local neutral_tags = {
        ["agriculture"] = "planets",
        ["criminal"] = "worlds",
        ["government"] = "facilities",
        ["industrial"] = "executives and their children",
        ["prison"] = "workers",
        ["research"] = "establishments"
    }
    -- TODO: these are special
    local good_tags = {
        ["medical"] = {
            "worlds",
            "facilities",
            "planets",
            "institutions",
            "complexes"
        },
        ["military"] = {
            "worlds",
            "facilities",
            "outposts",
            "organizations",
            "complexes"
        },
        ["rural"] = {"worlds", "planets", "moons", "paradises", "gardens"},
        ["shipbuilding"] = {"facilities", "locations"},
        ["urban"] = {"cities", "megaplexes", "megacities", "suburbs", "clubs"},
        ["trade"] = {"hubs", "kernels", "stops"},
        ["rich"] = {"places", "worlds", "planets", "people", "geriatrics", "octogenarians", "centenarians"}
    }

    -- we probably didn't get our argument, so let's pick out our escort from the crew
    if not speaker then
        for _i, pers in ipairs(mem.companions) do
            if pers.skill == "Escort" then
                speaker = pers
            end
        end
    end

    if not speaker then
        print("error no speaker")
        return
    end

    -- see if we get some jobs here
    local world_score = -0.1
    local tags = spob.cur():tags()
    local relevant_message = _("I have been having trouble finding suitable customers.")
    local good_choices = {
        _("I always say that {tag} {place} are good for business."),
        _("We should come to {place} like these more often."),
        _("We should visit {place} like this one more often."),
        _("Those {tag} {place} are good for business."),
        _("I like traveling to {tag} {place}."),
        _("The {tag} {place} here were quite generous."),
        _("I had a good time here as usual."),
        _("I met one of my regulars. You'll never know the details."),
        _("Even a {made_up} would like this place.")
    }
    local neutral_choices = {
        _("I think that {tag} {place} aren't the worst for business."),
        _("The business is usually good when it comes to {tag} {place}."),
        _("We should stop at {place} like these every once in a while."),
        _("We should visit {place} like this one more often, but not too often."),
        _("Those {tag} {place} are alright for business."),
        _("I like traveling to {tag} {place}."),
        _("This was a nice break."),
        _("The {tag} {place} here were decent."),
        _("I had an unexpected good time."),
        _("I had a surprisingly good time."),
        _("I had a surprisingly relaxed stay."),
        _("I lucked into one of my regulars. You'll never guess which one."),
        fmt.f(_("I saw a {made_up} for what I think was the first time."), {made_up = getMadeUpName()}),
        fmt.f(_("Was that a {made_up}?"), {made_up = getMadeUpName()}),
        fmt.f(_("Was that a {made_up} back there?"), {made_up = getMadeUpName()}),
        fmt.f(
            _("I didn't want to ask in front of that {made_up}, but do you think it's real?"),
            {made_up = getMadeUpName()}
        )
    }
    -- check good tags
    for tag, thing_choices in pairs(good_tags) do
        if tags[tag] then
            local thing = pick_one(thing_choices)
            world_score = world_score + 3
            if rnd.rnd() > 0.33 then
                relevant_message = fmt.f(pick_one(good_choices), {tag = tag, place = thing, made_up = getMadeUpName()})
            end
        end
    end

    -- check neutral tags
    for tag, thing in pairs(neutral_tags) do
        if tags[tag] then
            world_score = world_score + 1
            if not relevant_message and rnd.rnd() > 0.67 then
                relevant_message =
                    fmt.f(pick_one(neutral_choices), {tag = tag, place = thing, made_up = getMadeUpName()})
            end
        end
    end

    -- check the dumps
    if world_score < 0 then
        for tag, choices in pairs(bad_tags) do
            if tags[tag] then
                world_score = -5
                relevant_message = fmt.f("What are we doing on this {place}?", {place = pick_one(choices)})
                -- create an unpleasant memory
                create_memory(
                    speaker,
                    "work",
                    {
                        system = system.cur(),
                        planet = spob.cur()
                    }
                )
            end
        end
    end

    local payoff = 0
    if world_score > 0 then
        for i = 0, world_score do
            local job_pay = world_score * 10e3 + world_score * 5e3 * rnd.threesigma()
            if rnd.rnd(0, 1) == 1 then -- we got the job
                payoff = payoff + job_pay
            elseif rnd.rnd() < (math.min(50, speaker.xp) * speaker.satisfaction / 1000) then
                -- we somehow got the job with a bonus
                payoff = payoff + job_pay + 100e3
            end
        end
        -- create a work memory
        create_memory(
            speaker,
            "work",
            {
                credits = fmt.credits(payoff),
                system = system.cur(),
                planet = spob.cur()
            }
        )
    end

    -- raise or lower satisfaction based on world score
    speaker.satisfaction = math.min(10, math.max(-10, speaker.satisfaction + world_score))

    if payoff > speaker.threshold then -- we are happy
        -- raise the satisfaction based on payoff
        speaker.satisfaction = math.min(10, speaker.satisfaction + math.floor(payoff / speaker.threshold))

        -- set our last message to happy regardless of true satisfaction
        -- if we are unhappy, maybe mention that this was a turnaround TODO
        speaker.conversation.sentiment = relevant_message
    else -- we are unhappy
        -- set our last message to dissatisfied regardless of true satisfaction
        speaker.conversation.sentiment = relevant_message
        if speaker.satisfaction < 0 then
            -- TODO: pick from choices
            speaker.conversation.sentiment = relevant_message .. " " .. pick_one(speaker.conversation.special["worry"])
        end
    end
end

-- the player boards a hostile ship with a demoman on board
function player_boarding_c4(target, speaker)
    if target:hostile() and target:memory().natural == true then
        hook.timer(2, "speak_notify", speaker)
        hook.timer(6, "detonate_c4", target)
        hook.timer(8, "detonate_c4", target)
        hook.timer(9, "detonate_c4", target)
        hook.timer(10, "detonate_c4", target)
        hook.timer(rnd.rnd(10, 11), "detonate_c4", target)
        hook.timer(11, "detonate_c4", target)
        hook.timer(12 + rnd.rnd(), "detonate_c4", target)
        hook.timer(12 + rnd.rnd(), "detonate_c4", target)
        hook.timer(12 + rnd.rnd(), "detonate_c4", target)
        -- we just planted a bomb, increase satisfaction
        speaker.satisfaction = math.min(10, speaker.satisfaction + 1)
        -- if we planted a bomb on something big, create a memory based on how likely we are to mention it
        if target:ship():size() >= 5 and rnd.rnd() < speaker.chatter then
            create_memory(
                speaker,
                "violence",
                {
                    target = target:name(),
                    ship = target:ship(),
                    credits = fmt.credits(target:credits()),
					cred_amt = target:credits(),
                    armour = target:health(true),
                    system = system.cur()
                }
            )
        end
    else
        -- we boarded something for friendly reasons, if we like violence, we are pissed
        for _i, person in ipairs(mem.companions) do
            if has_interest(person, "violence") then
                person.satisfaction = math.max(-10, person.satisfaction - 1)
            end
        end
    end
end

-- a demoman's bomb explodes (single payload)
function detonate_c4(target)
    if target and target:exists() then
        local sound_choices = {
            "medexp1",
            "medexp0",
            "crash1",
            "grenade",
            "explosion0",
            "explosion1",
            "explosion2",
            "tesla"
        }
        local dir_vec = vec2.new(math.floor(rnd.threesigma() * 30), math.floor(rnd.twosigma() * 20))
        target:knockback(800, dir_vec, target:pos() - dir_vec)
        target:setDir(target:dir() + rnd.threesigma() * 0.07)
        local expl_pos = vec2.add(target:pos(), rnd.threesigma() * 2, rnd.twosigma() * 2)
        -- apply the damage (the player gets the credit)
        target:damage(rnd.rnd(277, 313), 0, 100, "impact", player.pilot())
        -- visual and audio effects?
        audio.soundPlay(pick_one(sound_choices), expl_pos)
        -- we used explosives, add to cost
        local current_cost = mem.costs["equipment"]
        if current_cost == nil then
            current_cost = 0
        end
        mem.costs["equipment"] = current_cost + prices["equipment"]
        -- an explosion just happened, if we like violence, we are thrilled
        for _i, person in ipairs(mem.companions) do
            if has_interest(person, "violence") then
                person.satisfaction = math.min(10, person.satisfaction + 0.01)
            end
        end
    end
end
