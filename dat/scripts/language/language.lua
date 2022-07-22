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


-- Note: most of these will use a simple form like {fruit}s for the plural, add/adjust/translate accordingly
nouns.food = {}
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
}

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
nouns.actors = {}
nouns.actors.animals = {}
nouns.actors.animals.mammals.domestic = {
	_("cat"),
	_("dog"),
	_("horse"),
	_("cow"),
	_("anthropod"), -- yeah... slaves?
	_("donkey"),
	_("equine"),
	_("rabbit"),
	_("mule"),
	_("hamster"),
	_("rat"),
}

nouns.actors.animals.mammals.wild = {
	_("monkey"),
	_("koala"),
	_("bear"),
	_("lion"),
	_("tiger"),
	_("cougar"),
	_("pathner"),
	_("jaguar"),
	_("racoon"),
	_("kangaroo"),
	_("pangolin"),
	_("rat"),
}

nouns.actors.animals.aquatic = {
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

nouns.actors.animals.reptiles = {
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

nouns.actors.animals.birds = {
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
	_("tit"),
}

nouns.actors.animals.insects = {
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
}

-- nouns to describe a person or person-like object
nouns.actors.people = {}

-- when we refer to someone who is a hero in a friendly manner
-- or if we want a toy of a "hero" kind, it would be a <adjective> <hero> <toy> or something like that
nouns.actors.people.hero = {
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
nouns.actors.people.villain = {
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
}

-- something to wear TODO MOVE ADJECTIVES!
nouns.objects.clothes = {
	_("leather jacket"),
	_("vintage coat"),
	_("baseball hat"),
	_("sock"),
	_("frock"),
	_("trenchcoat"),
	_("uniform"),
	_("shirt"),
	_("neckpiece"),
	_("glove"),
}

-- generic items
nouns.objects.items = {
	_("bouquet of flowers"),
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
	_("helmet"),
	_("cap"),
	_("chapeau"),
	_("tophat"),
	_("hat"),
	_("apron"),
	_("spoon"),
	_("fork"),
	_("spork"),
	_("chopstick"),
	_("broom"),
	_("mop"),
	_("bucket"),
	_("sculpture"),
	_("figurine"),
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
	_("conversiot module"),
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
	_("destoryed"),
	_("ruined"),
}

adjectives.negative.boring = {
	_("common"),
	_("mass-produced"),
	_("low-quality"),
}

adjectives.negative.dated = {
	_("aged"),
	_("old"),
	_("worn"),
	_("rotten"),
	_("dated"),
	_("expired"),
	_("mature"),
}


adjectives.negative.nasty = {
	_("disgusting"),
	_("filthy"),
	_("nasty"),
	_("displeasing"),
	_("unpleasant"),
	_("unappealing"),
	_("rotten"),
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
}

language.adjectives = adjectives
-- language.descriptors = descriptors  -- TODO
language.nouns = nouns

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

return language