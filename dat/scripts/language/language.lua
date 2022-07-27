local fmt = require "format"


-- simple language stuff

local language = {}
local nouns = {}
local adjectives = {}
local descriptors = {}

-- helpers
local function _joined_table(t1, t2)
	local copy = {}
	for _i, v in ipairs(t1) do
        table.insert(copy, v)
    end
    for _i, v in ipairs(t2) do
        table.insert(copy, v)
    end

    return copy
end

-- pick a random letter out of a string
local function pick_str(str)
    local ii = rnd.rnd(1, str:len()) -- pick a random index from string length
    return string.sub(str, ii, ii) -- returns letter at ii
end

-- pick a random item from the collection
local function pick_one(target)
    local r = rnd.rnd(1, #target)
    return target[r]
end


-- Note: most of these will use a simple form like {fruit}s for the plural, add/adjust/translate accordingly
nouns.food = {}
nouns.actors = {}
-- Used like I want to eat a {fruit} or let's restock the {fruit}s
nouns.food.fruit = {
	_("banana"),
	_("apple"),
	_("pear"),
	_("grape"),
	_("orange"),
	_("cantaloupe"),
	_("melon"),
	_("lemon"),
	_("lime"),
	_("coconut"),
	_("avocado"),
	_("pineapple"),
	_("plum"),
	_("watermelon"),
	_("melon"),
	_("quince"),
	_("plantain"),
	_("fig"),
	_("peanut"),
	_("prune"),
	_("papaya"),
	_("apricot"),
	_("olive"),
	_("zucchini"),
	_("pea"),
	_("cucumber"),
	_("bell pepper"),
	_("pumpkin"),
	_("tamarind"),
	_("raisin"),
	_("durian"),
	_("date"),
	_("nectarine"),
	_("kumquat"),
	_("guava"),
	_("persimmon"),
	_("mushroom"),	-- this one is nice to have, lets add some more similar things
	_("portobello"),
	_("portabella"),
	_("cashew nut"),
}

-- same as fruit, but more general and not fruit... should fit in lore-wise somehow TODO NOTE for someone
nouns.food.general = {
	_("egg"),
	_("noodle"),
	_("luxury sub"),
	_("slice"),
	_("pie"),
	_("cake"),
	_("salad"),
	_("protein bar"),
	_("synthetic animal meat"),
	_("cheese"),
	_("cake"),
	_("synthetic pizza"),
	_("organic pastry bundle"),
	_("meat"),
	_("steak"),
}

nouns.food.drink = {}

nouns.food.drink.regular = {
	_("tea"),
	_("coffee"),
	_("water"),
	_("juice"),
	_("extract"),
	_("elixir"),
	_("concoction"),
}

nouns.food.drink.alcoholic = {
	_("Soromid special elixir"),
	_("wine"),
	_("ale"),
	_("brew"),
	_("beer"),
	_("liquor"),
	_("cognac"),
	_("liqueur"),
	_("Dvaer duelling drink"),
	_("Caladan ice crush"),
	_("Captain Lordan"),
	_("Wringer's Wrench"),
	_("Greenberg green drink"),
}

-- todo nouns.abstract?
-- for finding an activity of type
nouns.activities = {}
nouns.activities.games = {
	_("cards"),
	_("chess"),
	_("tic-tac-toe"),
	_("blackjack"),
	_("21"),
	_("poker"),
}

-- types of facilities
nouns.facilities = {}
-- facilities found on a cruiser
nouns.facilities.cruiser = {
	_("theater room"),
	_("virtual experience simulator"),
	_("mess hall"),
	_("recreation room"),
	_("gym"),
	_("spa"),
	_("sauna"),
	_("holoball"),
	_("basketball"),
	_("holoracket"),
	_("training simulator")
}

-- nouns to describe some actor

animals = {}
animals.mammals = {}

animals.mammals.domestic = {
	_("cat"),
	_("dog"),
	_("horse"),
	_("cow"),
	_("slave"), -- yeah... slaves? I figure maybe it fits with the lore...
	_("prisoner"),
	_("donkey"),
	_("equine"),
	_("rabbit"),
	_("mule"),
	_("hamster"),
	_("rat"),
}

animals.mammals.wild = {
	_("monkey"),
	_("koala"),
	_("bear"),
	_("elephant"),
	_("polar bear"),
	_("lion"),
	_("tiger"),
	_("cougar"),
	_("panther"),
	_("jaguar"),
	_("racoon"),
	_("kangaroo"),
	_("pangolin"),
	_("porcupine"),
	_("rat"),
	_("zebra"),
}

animals.aquatic = {
	_("penguin"),
	_("whale"),
	_("dolphin"),
	_("sea lion"),
	_("sea cow"),
	_("orca"),
	_("seal"),
	_("otter"),
	_("porpoise"),
	_("manatee"),
	_("walrus"),
	_("polar bear"),
	-- various kinds of fish below, I think... I'm not a marine biologist or anything so these could be incorrect
	_("tetra"),
	_("danio"),
	_("raspbora"),
	_("cichlid"),
	_("eel"),
	_("sardine"),
	_("pleco"),
	_("darter"),
}

animals.reptiles = {
	_("alligator"),
	_("crocodile"),
	_("dinosaur"),
	_("snake"),
	_("lizard"),
	_("turtle"),
	_("dragon"),
	_("iguana"),
	_("gecko"),
	_("skink"),
	_("cobra"),
	_("python"),
	_("boa constrictor"),
	_("viper"),
	_("cottonmouth"),
}

animals.birds = {
	_("duck"),
	_("chicken"),
	_("hen"),
	_("rooster"),
	_("chick"),
	_("condor"),
	_("kestrel"),
	_("caracal"),
	_("seagull"),
	_("gull"),
	_("raven"),
	_("eagle"),
	_("crow"),
	_("sparrow"),
	_("swan"),
	_("tit"),
}

animals.insects = {
	_("ant"),
	_("bee"),
	_("wasp"),
	_("cricket"),
	_("grasshopper"),
	_("beetle"),
	_("spider"),
	_("arachnid"),
	_("centipede"),
	_("millipede"),
	_("locust"),
	_("moth"),
	_("cockroach"),
	_("arthropod"),
}


nouns.actors.animals = animals

-- nouns to describe a person or person-like object
people = {}

-- when we refer to someone who is a hero in a friendly manner
-- or if we want a toy of a "hero" kind, it would be a <adjective> <hero> <toy> or something like that
people.hero = {
	_("warrior"),
	_("killer"),
	_("captain"),
	_("pilot"),
	_("negotiator"),
	_("diplomat"),
	_("researcher"),
	_("scientist"),
	_("merchant"),
	_("wayfarer"),
	_("spacefarer"),
	_("hero"),
	_("daredevil"),
}

-- same as hero, but with an implied and deliberate negative connotation
people.villain = {
	_("warrior"),
	_("killer"),
	_("slaver"),
	_("waster"),
	_("exterminator"),
	_("terrorist"),
	_("freedom fighter"),
	_("saboteur"),
	_("informant"),
	_("vagabond"),
	_("drifter"),
	_("villain"),
}

nouns.actors.people = people

-- common objects
nouns.objects = {}
-- things you might see in space
nouns.objects.space = {
	 _("drama Llama"),
	 _("Shuttle"),
	 _("Kangaroo"),
	 _("Koala"),
	 _("Derelict"),
	 _("weird sponge thing"),
	 _("luxury yacht"),
	 _("cube"),
	 _("hunk of junk"),
	 _("asteroid"),
	 _("meteor"),
	 _("comet"),
	 _("planet"),
	 _("moon"),
	 _("star"),
	 _("space truck"),
	 _("cargo container"),
	 _("battleship"),
	 _("cruiser"),
	 _("spooky frigate"),
	 _("zooming corvette"),
	 _("suicidal fighter"),
	 _("murderous drone"),
}

-- accessories are objects that a person wears or uses on itself
nouns.objects.accessories = {
	_("high-quality lip stick"),
	_("lip gloss"),
	_("eye shadow"),
	_("hairpiece"),
	_("virtual bracelet"),
	_("synthetic snakeskin applicator"),
	_("chapstick"),
	_("earring"),
	_("necklace"),
	_("ring"),
	_("nose ring"),
	_("ocular implant"),
	_("cybernetic appendage"),
}

nouns.objects.clothes = {
	_("jacket"),
	_("coat"),
	_("baseball hat"),
	_("sock"),
	_("frock"),
	_("dress"),
	_("trenchcoat"),
	_("uniform"),
	_("shirt"),
	_("neckpiece"),
	_("glove"),
	_("cap"),
	_("helmet"),
	_("apron"),
	_("chapeau"),
	_("tophat"),
	_("hat"),
	_("shoe"),
	_("boot"),
	_("stiletto"),
}

-- generic items
nouns.objects.items = {
	_("bouquet"),
	_("letter"),
	_("emblem"),
	_("piece of paper"),
	_("paper"),
	_("mechanical timekeeper"), -- because plural of watch ends in -es ...
	_("timepiece"),
	_("certificate"),
	_("bone"),
	_("skull"),
	_("book"),
	_("notebook"),
	_("diary"),
	_("plate"), -- like a dish to put food on
	_("pen"),
	_("spoon"),
	_("fork"),
	_("spork"),
	_("chopstick"),
	_("broom"),
	_("mop"),
	_("bucket"),
	_("sculpture"),
	_("figurine"),
	_("statue"),
	_("painting"),
	_("photograph"),
	_("polaroid"),
	_("orchid"),
	_("rose"),
	_("flower"),
	_("tulip"),
	_("archive"),
	_("spanner"),
	_("screwdriver"),
	_("sword"),
	_("ornament"),
	_("decoration"),
	
}

-- things that fit in your hand, pocket or belt/jacket
-- but should be some kind of work thing
nouns.objects.tools = {
	_("spanner"),
	_("screwdriver"),
	_("hammer"),
	_("pen"),
	_("pencil"),
	_("gyroscopic stabilizer"),
	_("level"),
	_("flashlight"),
	_("lamp"),
	_("beacon"),
	_("flare"),
}

-- things that can need to be replaced, repaired or inspected
nouns.objects.spaceship_parts = {
	_("coolant pump"),
	_("control unit"),
	_("insulated circuit"),
	_("transponder"),
	_("scanning system"),
	_("navigational computer"),
	_("stabilizing coil"),
	_("flux capacitor"),
	_("deflector plate"),
	_("coupling motivator"),
	_("coolant coil"),
	_("conversion module"),
	_("power modulator"),
	_("escape pod"),
	_("diagnostic computer"),
	_("pipe"),
	_("plate"), -- like an armor plate
}

-- wearables are clothes and accessories
nouns.objects.wearables = _joined_table(nouns.objects.clothes, nouns.objects.accessories)
-- objects that can be used generically but haven't been classified
nouns.objects.random = {
	_("elegant design"), -- okay, not really an object... but still
	_("abstract holosculpture"),
	_("virtual death simulator"),
	_("synthetic snakeskin applicator"),
	_("white elephant"),
	_("red herring"),
	_("classic video game"),
	_("optical combustion device"),
	_("synthetic aquarium"),
	_("animal figurine"),
	_("paper plane"),
	_("telepathically controlled camera drone"),
	_("baseball bat"),
	_("basketball"),
	_("wicker basket"),
	_("trojan"),
	_("sock puppet"),
	_("social network simulator"),
	_("vintage hand egg"),
	_("fictional literature"),
	_("device"),
	_("gadget"),
	_("hand-held"),
	_("portable"),
	_("Ultra 3000"),
	_("Neo 7000"),
	_("0K Elite Edition cup chiller"),
	_("wholesome book"),
	_("digital archive"),
	_("toy"),
	_("puppet"),
}

-- generic things to give as gifts
nouns.gifts = {
	_("symbolic gift"),
	_("bouquet of flowers"),
	_("letter of recommendation"),
	_("commemorative emblem"),
	_("book of poetic scripture"),
	_("poetry book"),
	_("nice timepiece"),
	_("new timepiece"),
	_("spa certificate"),
	_("60 second break"),
	_("handful of roses"),
	_("tulip"),
	_("hand-bound notebook"),
	_("digital diary"),
	_("decorative plate"),
	_("ceremonial pen"),
	_("ceremonial hat"),
	_("chef's hat and apron"),
	_("hunting blade"),
	_("golden ornamental spoon"),
	_("rose or two"),
	_("flower or four"),
	_("tulip or three"),
	_("historical archive"),
	_("adorned ornamental sword"),
	_("decorated vanity blade")
}

adjectives.positive = {}
adjectives.positive.nice = {
	_("adorned"),
	_("elegant"),
	_("artistic"),
	_("colorful"),
	_("classic"),
	_("wholesome"),
	_("beautiful"),
	_("good"),
	_("commemorative"),
	_("nice"),
	_("new"),
	_("hand-bound"),
	_("decorative"),
	_("decorated"),
	_("ceremonial"),
	_("golden"),
	_("ornamental"),
	_("historical"),
	_("vintage"),
	_("high-quality"),
	_("stylized"),
	_("quality"),		-- yeah I know, we have high quality, but hearing quality "sounds good", so unless we dislike low-quality, quality can be good
}

adjectives.positive.precious = {
	_("precious"),
	_("unique"),
	_("irreplacable"),
	_("valuable"),
	_("rare"),
	_("sought-after"),
	_("highly-regarded"),
	_("coveted"),
	_("legendary"),
	_("expensive"),
	_("enamored"),
	_("beloved"),
	_("admired"),
	_("beautiful"),
	_("loved"),
	_("heartwarming"),	
	_("sentimental"),
	_("poetic"),
	_("poignant"),
}

adjectives.positive.magical = {
	_("magical"),
	_("ethereal"),
	_("unreal"),
	_("surreal"),
	_("hypothetical"),
	_("theoretical"),
	_("levitating"),
	_("existential"),
	_("mystical"),
	_("mythical"),
	_("unbelievable"),
	_("spectral"),
	_("astonishing"),
	_("amazing"),
	_("phenomenal"),
	_("astounding"),	
}

-- dated, but young (but you can't use young for inanimate objects)
adjectives.positive.dated = {
	_("ripe"),
	_("fresh"),
	_("new"),
	_("hot"),	-- slang like "hip"
}

adjectives.size = {}

adjectives.size.large = {
	_("large"),
	_("gargantuan"),
	_("huge"),
	_("gigantic"),
	_("massive"),
	_("enormous"),
	_("to-scale"),
	_("full-sized"),
	_("big"),
}

adjectives.size.small = {
	_("miniature"),
	_("teacup"),
	_("model"),
	_("toy-sized"),
	_("small"),
	_("tiny"),
	_("scale"),
	_("mini"),
	_("compact"),
}

adjectives.negative = {}

-- describing items, machinery or equipment
adjectives.negative.broken = {
	_("faulty"),
	_("malfunctioning"),
	_("useless"),
	_("broken"),
	_("cracked"),
	_("smashed"),
	_("destroyed"),
	_("ruined"),
}

adjectives.negative.boring = {
	_("common"),
	_("mass-produced"),
	_("low-quality"),
}

adjectives.negative.dated = {
	_("aged"),
	_("ancient"),
	_("old"),
	_("worn"),
	_("rotten"),
	_("dated"),
	_("expired"),
	_("mature"),
	_("dusty"),
}

adjectives.negative.smelly = {
	_("pungent"),
	_("smelly"),
	_("vile"),
	_("funky"),
	_("rancid"),
	_("stinking"),
	_("foul"),
	_("fetid"),
}

adjectives.negative.nasty = {
	_("disgusting"),
	_("filthy"),
	_("nasty"),
	_("displeasing"),
	_("unpleasant"),
	_("unappealing"),
	_("rotten"),
	_("vile"),
}

adjectives.violent = {
	_("murderous"),
	_("thieving"),
	_("violent"),
	_("aggressive"),
	_("fearsome"),
	_("vehement"),
	_("brutal"),
	_("vicious"),
	_("fierce"),
	_("wild"),
	_("savage"),
	_("fearless"),
	_("dangerous"),
}

adjectives.neutral = {
	_("okay"),
	_("fine"),
	_("decent"),
	_("acceptable"),
	_("passing"),
	_("interesting"),
	_("passable"),
}

-- not concrete but abstract color... 
-- describes the look of something, so any kind of visible pattern or
-- any kind of color or color-like adjective, even if it's a homonym
-- in fact, that's better because it triggers cointeraction
-- (if I like lime fruit then I like to see a lime colored book)
adjectives.colors = {
	_("red"),
	_("green"),
	_("blue"),
	_("yellow"),
	_("purple"),
	_("magenta"),
	_("orange"),
	_("amber"),
	_("gold"),
	_("golden"),
	_("silver"),
	_("bronze"),
	_("metallic"),
	_("chrome"),
	_("crimson"),
	_("violet"),
	_("maroon"),
	_("cyan"),
	_("navy"),
	_("pink"),
	_("lime"),
	_("peach"),
	_("salmon"),
	_("brown"),
	_("black"),
	_("charcoal"),
	_("white"),
	_("ivory"),
	_("off-white"),
	_("grey"),
	_("gray"),
	_("emerald"),
	_("ruby"),
	_("mauve"),
	_("jade"),
	_("olive"),
	_("denim"),
	_("transparent"),
	_("translucent"),
	_("glowing"),
	_("luminescent"),
	_("azure"),
	_("striped"),
	_("spotted"),
	_("textured"),
	_("leather"),
	_("glazed"),
	_("stained"),
	_("fur"),
	_("hairy"),
	_("fuzzy"),
	_("mossy"),
}

-- verbs are special and come in groups of conjugations
local verbs = {}
verbs.being = {}
verbs.being.present = {}
verbs.being.past = {}

verbs.being.present.singular = _("is")
verbs.being.present.plural = _("are")
verbs.being.past.singular = _("was")
verbs.being.past.plural = _("were")

local conjunctions = {}
conjunctions.that = {
	_("that"),
}

-- parts of speech that can probably be completely ignored safely
local interjections = {}
interjections.general = {
	_("hey"),
	_("yo"),
	_("bloody hell"),
	_("umm"),
	_("um"),
	_("eh"),
	_("ehh"),
	_("erm"),
	_("err"),
	_("er"),
	_("well"),
	_("so"),
}

language.adjectives = adjectives
language.conjunctions = conjunctions
language.interjections = interjections
-- language.descriptors = descriptors  -- TODO
language.nouns = nouns
language.verbs = verbs

-- gets the plural of some noun
language.getPlural = function ( noun )
	-- TODO (defer): implement this so that it can be translatable
	-- which also means covering irregular plurals in English
	
	-- default regular: word has a plural ending
	-- this is a placeholder
	if not noun:sub(-1) == _("s") then
		return fmt.f("{noun}s", { noun = noun } )
	end

	-- default irregular: plural is like singular
	return noun
end

language.getAll = function ( collection )
	local aggregated = {}
	for category, items in pairs(collection) do
		if type(items) ~= "table" then
			-- got it
			table.insert(aggregated, items)
		else -- dig deeper
			aggregated = _joined_table(aggregated, language.getAll(items))
		end
	end
	
	return aggregated
end


-- generates a short, made-up capitalized Noun with limited imagination, usually 2-3 syllables or around 7 letters
language.getMadeUpName = function ()
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

-- a picks a random "fruit", or a made up fruit name
language.getRandomFruit = function ()
	-- must end with an s in plural in English, at least how it's being used now
	local fruits = _joined_table(nouns.food.fruit, {
		language.getMadeUpName():lower(), -- a made up food with a made up name
		-- a made up nut that the player might think is a testicle for comedic effect
		fmt.f(_("{ntype} nut"), { ntype = language.getMadeUpName() }),
		-- some fairly unappealing foods to try to get the player to discard stuff sometimes for the side effects
		_("synthetic food"),
		_("insect block"),
		_("cricket crisp"),
	})
	
	return pick_one(fruits)
end


-- returns some kind of statement describing an insulting proper noun
-- i.e. something you would say about a ship or person that's in disorder
language.getInsultingProperNoun = function ()
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
        language.getMadeUpName():lower()
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
        language.getMadeUpName():lower()
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
        language.getMadeUpName()
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
        language.getMadeUpName():lower()
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


return language