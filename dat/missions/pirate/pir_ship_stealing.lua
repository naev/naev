--[[

-- The player pays a fellow pirate to know where to steal a random ship.

	The player pays to get the position of a ship on a random planet of a random
	faction. When he gets there, the planet is guarded (which means he may have
	to fight his way through, which is the most probable option).

	When he lands on the target planet, he gets a nice message explaining what
	happens, he gets a new ship, is able to refit it, etc.

	Then, when the player wants to leave the planet, and that will eventually
	happen (at least, I hope…) he’ll be pursued by a few fighters.
--]]

include "jumpdist.lua"

local informer
local refusal
local approval
local success

local title = "Stealing a %s"
local reward = "A brand new %s"
local description = "Land on %s, in the %s system, and escape with your new %s."

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
	informer = {
		description = "A pirate informer is looking at you. Maybe he has some useful information to sell?";
		title = "Ship to steal";
		message = [[   "Hi, pilot. I have the location of a %s to be used by the %s. Maybe it interests you, who knows?"
   "However, I'm going to sell that information only. It'd cost you %s, but the ship is probably worth much more, if you can get it."
   
   Do you really want to pay to know where that ship is?]];
	}

	approval = {
		title = "Of course";
		message = [[   You pay the informer, who tells you the ship in currently on %s, in the %s system. He also gives you its security codes and warns you about patrols.
		
   Interestingly, he also tells you you shouldn’t use a ship you are attached to on this mission, as you may not be able to get out of the planet with both ships.]];
	}

	refusal = {
		title = "Not interested";
		message = "   You politely refuse, much to the surprise of this fellow who offered you an offer he obviously thought to be interesting.";
	}

	success = {
		title = "Ship successfully stolen!";
		message = [[   It took you several hours to escape patrols, and a few more ours to get in the ship to steal and manage to access it, but you finally take control of it with the access codes you were given. Hopefully, you will be able to sell this %s, or maybe even to use it.
		
   Enemy ships will probably be after you as soon as you'll leave the atmosphere, so you should be ready.]];
	}
end

local base_price = 100000

local guards = {
	Empire = {
		-- Not too big, we may have to fight our way through.
		ship = "Empire Admonisher";
		AI = "empire_idle";
	};
}

local ships = {
	Empire = {
		fighter   = { "Empire Shark", "Empire Lancelot" };
		corvette  = { "Empire Admonisher" };
		destroyer = { "Empire Pacifier" };
		cruiser   = { "Empire Hawking" };
		carrier   = { "Empire Peacemaker" };
	}
}

-- FIXME: Should be automated, using ships[]
local classes = {
	Empire = { "fighter", "corvette", "destroyer", "cruiser", "carrier" };
}

local function price(class)
	local modifier = 1
	if class == "fighter" then
		modifier = 0.5
	elseif class == "bomber" then
		modifier = 0.75
	elseif class == "destroyer" then
		modifier = 1.5
	elseif class == "cruiser" then
		modifier = 2
	elseif class == "carrier" then
		modifier = 3
	end

	return modifier * base_price
end

local function random_faction()
	-- FIXME: Soromid, Dvaered, Goddard, Sirius or major faction.
	-- FIXME: For fame, at least once, a fellow Pirate clan.
	-- FIXME: For beginning players, Frontier, Independent or minor faction.
	-- FIXME: Well, of course, check there’s a planet of that faction in range
	--        (minor exception for other pirate clans)
	return "Empire"
end

local function random_class(faction)
	local m = #classes[faction]

	if m == 0 then
		return
	end

	local r = rnd.rnd(1, m)

	return classes[faction][r]
end

local function random_ship(faction, class)
	local m = #ships[faction][class]

	if m == 0 then
		return
	end

	local r = rnd.rnd(1, m)

	return ships[faction][class][r]
end

local function random_planet(f)
	local planets = {}
	local f = faction.get(f)
	local maximum_distance = 6
	local minimum_distance = 1

	getsysatdistance(
		system.cur(),
		minimum_distance, maximum_distance,

		function(s)
			for i, v in ipairs(s:planets()) do
				if v:faction() == f and v:services().shipyard then
					planets[#planets + 1] = v
				end
			end 
			return false
		end
	)

	if #planets > 0 then
		return planets[rnd.rnd(1,#planets)]
	else
		return
	end
end

function create ()
	ship = { __save = true }
	ship.faction = random_faction()
	ship.class   = random_class(ship.faction)

	if not ship.class then
		-- If we’re here, it means we couldn’t get a ship of the right faction
		-- and of the right class.
		print("WARNING: no ship.class :(")
		misn.finish()
	end

	-- We’re assuming ships[faction][class] is not empty, here…
	ship.exact_class = random_ship(ship.faction, ship.class)
	ship.price   = price(ship.class)
	ship.planet  = random_planet(ship.faction)

	if not ship.planet then
		-- If we’re here, it means we couldn’t get a planet close enough.
		print("WARNING: no ship.planet :(")
		misn.finish()
	end

	ship.system = ship.planet:system()

	if not misn.claim { ship.system } then
		-- FIXME: Shouldn’t this mean we end the mission or something?
	end

	-- FIXME: Portrait
	misn.setNPC( "A Pirate informer", "none" )
	misn.setDesc( informer.description )
end

function accept()
	if
		tk.yesno(
			informer.title,
			string.format(
				informer.message,
				ship.class,
				ship.faction,
				tostring(ship.price)
			)
		)
	then
		tk.msg(
			approval.title,
			string.format(
				approval.message,
				ship.planet:name(),
				ship.system:name()
			)
		)

		misn.accept()

		-- Mission title, reward, description
		misn.setTitle(
			string.format(
				title,
				ship.class
			)
		)
		misn.setReward(
			string.format(
				reward,
				ship.class
			)
		)
		misn.setDesc(
			string.format(
				description,
				ship.planet:name(),
				ship.system:name(),
				ship.class
			)
		)

		-- FIXME: Mission markers
		-- FIXME: OSD

		hook.land("land")
		hook.enter("enter")
	else
		tk.msg(refusal.title, refusal.message)
		misn.finish(false)
	end
end

function land()
	local landed = planet.cur()
	if landed == ship.planet then
		-- Oh yeah, we stole the ship. \o/
		tk.msg(
			success.title,
			string.format(
				success.message,
				ship.exact_class
			)
		)

		-- The old ship the player used will still be on the planet. I’m not 
		-- too sure what to do about it, but, well…
		player.swapShip(ship.exact_class)

		-- FIXME: That ship needs equipment. What do I do about it?

		-- Two cannons should be enough for most not-so-big ships.
		player.pilot():addOutfit"Laser Cannon MK3"
		player.pilot():addOutfit"Laser Cannon MK3"

		player.takeoff()

		-- FIXME: We should add a few pursuers, there. But, without solving a
		--        fow other FIXMEs, this would just make what we currently have
		--        of the mission unplayable…

		-- FIXME: A penalty to faction standing?

		-- This is a success. The player stole his new ship, and everyone is
		-- happy with it. Getting out of the system alive is the player’s 
		-- responsibility, now.
		misn.finish(true)
	end
end

function enter()
	-- A few faction ships guard the target planet.
	if system.cur() == ship.system then
		-- We want the player to be able to land on the destination planet…
		ship.planet:landOverride(true)

		-- I’m not really sure what this is for, but in was in shadowrun.lua,
		-- so I just kept it…
		pilot.clear()
		pilot.toggleSpawn(false)

		-- FIXME: Make the number of guarding ships (and therefor their 
		--        position) vary with the player’s rating, his pirate fame,
		--        etc. The class of the ship on the ground should also
		--        influence the number of guardians, or maybe the class of
		--        ships guarding?
		local planetpos = ship.planet:pos()
		local positions = {
			planetpos + vec2.new(200,0),
			planetpos + vec2.new(130,130),
			planetpos + vec2.new(0,200),
			planetpos + vec2.new(-130,130),
			planetpos + vec2.new(-200,0),
			planetpos + vec2.new(-130,-130),
			planetpos + vec2.new(0,-200),
			planetpos + vec2.new(130,-130),
		}

		for i = 1,#positions do
			local position = positions[i]
			pilot.add(guards[ship.faction].ship, guards[ship.faction].AI, position)
		end
	end
end

function abort()
	misn.finish(false)
end

