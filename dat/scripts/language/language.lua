-- simple language stuff

local language = {}
local nouns = {}

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

-- Used like I want to eat a {fruit} or let's restock the {fruit}s
nouns.fruit = {
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
	_("vintage lip gloss"),
	_("eye shadow"),
	_("hairpiece"),
	_("virtual bracelet"),
	_("synthetic snakeskin applicator"),
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
	_("glove")
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