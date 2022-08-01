--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Auxiliary Ship Handler">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]

--[[
   Auxiliary Ship Event Handler

   Allows the player to fly an auxiliary ship from the bay
--]]

-- luacheck: globals auxiliary_ship_mission auxiliary_ship_return check_aux_bay spawn_ghost (Hook functions passed by name)


local fmt = require "format"
local der = require "common.derelict"
local vntk = require "vntk"

local joyride

function create()
	hook.takeoff("check_aux_bay")
end

local function has_bay()
	local po = player.pilot():outfits()
	for _i, oo in ipairs(po) do
		if string.find(oo:nameRaw(), "Auxiliary Ship Bay") then
			return true
		end
	end
	return false
end

local shuttle_outfits
local function add_outfit( oname )
	if player.numOutfit( oname ) > 0 then
		shuttle_outfits[#shuttle_outfits + 1] = oname
		return true
	end
	return false
end

local accessories
local function get_accessories()
	if not accessories then
		accessories = {}
		local po = player.pilot():outfits()
		for _i, oo in ipairs(po) do
			if oo:type() == "Accessory" then
				table.insert(accessories, oo:nameRaw())
			end
		end
	end
	return accessories
end

local function configure_outfits()
	if not shuttle_outfits then
		shuttle_outfits = {}

		-- add a plasma drill if we own one
		if not add_outfit("S&K Plasma Drill") then
			-- if we don't have a plasma drill, maybe we are pirates
			-- so fit a transponder if we need one
			local sys = system.cur()
			if sys:presence("friendly") < sys:presence("hostile") then
				add_outfit("Fake Transponder")
			else
				-- couldn't find anything useful, let's try to put a blink drive in the medium slot
				add_outfit("Blink Drive")
			end
		end
		-- if we have unlocked the pulse scanner, we definitely want one!
		add_outfit("Pulse Scanner")

		-- if we didn't already fill our slots, try to increase stealth
		add_outfit("Veil of Penelope")
		add_outfit("Nexus Stealth Coating")
		-- structure slots, no use for anything else since we don't allow landing or jumping
		add_outfit("Small Cargo Pod")
		add_outfit("Small Cargo Pod")

		-- accessory slot
		accessories = get_accessories()
		local accessory = accessories[rnd.rnd(1, #accessories)]
		if accessory then
			add_outfit(accessory)
		end

		-- tiny drone slot
		add_outfit("Za'lek Scanning Drone Interface")
	end
end

function check_aux_bay()
	if has_bay() then
		mem.last_ship = player.ship()
		showButton()
		configure_outfits()
	else
		hideButton()
		-- allows the player to reload outfits by removing and adding the bay back to the ship
		shuttle_outfits = nil
		accessories     = nil
	end
end

-- if the player swapped out of his own ship in space, or if
-- the player despawned his own ship while landing, we need to respawn it
local function spawn_ghost()
	if
		joyride
	then
		if joyride.hook then
			hook.rm(joyride.hook)
			joyride.hook = nil
		end
		local pp = player.pilot()
		local fakefac = faction.dynAdd(pp:faction():name(), joyride.mothership, joyride.mothership, { ai = "escort_guardian", clear_enemies = true})

		joyride.pilot = pilot.add(joyride.ship, fakefac, joyride.pos, joyride.mothership, { naked = true })
		-- match speed and velocity
		joyride.pilot:setDir(joyride.dir)
		joyride.pilot:setVel(joyride.vel)
		-- and outfits
		for _j, o in ipairs(joyride.outfits) do
			joyride.pilot:outfitAdd(o)
		end
		-- put the cargo back
		for k, v in pairs(joyride.cargo) do
			-- the player took the mission cargo
			if not v.m then
				joyride.pilot:cargoAdd( v.name, v.q )
			end
		end
		joyride.pilot:setVisplayer(true)
		joyride.pilot:setNoClear(true)
		joyride.pilot:setNoLand(true)
		joyride.pilot:setNoJump(true)
		joyride.pilot:setActiveBoard(true)
		joyride.pilot:setHilight(true)
		joyride.pilot:setFriendly(true)
		joyride.pilot:setInvincPlayer(true)
		hook.pilot(joyride.pilot, "board", "auxiliary_ship_return")
	end
end

function auxiliary_ship_mission()
	local template = pilot.add("Cargo Shuttle", "Trader", player.pilot():pos())
	if shuttle_outfits then
		template:outfitRm("all")
--		template:outfitRm("cores")
		template:outfitRm( "Unicorp PT-16 Core System")
		template:outfitAdd("Unicorp PT-68 Core System")
		for _j, o in ipairs(shuttle_outfits) do
			template:outfitAdd(o, 1 , true, false)
		end
	end

	local pp = player.pilot()

	joyride = {}
	joyride.mothership = player.ship()
	joyride.ship = pp:ship()
	joyride.pos = pp:pos()
	joyride.dir = pp:dir()
	joyride.vel = pp:vel()
	joyride.outfits = pp:outfits()
	joyride.cargo = pp:cargoList()

	local cl = pp:cargoList()
	for k, v in pairs( cl ) do
		if not v.m then
			-- goes into the placeholder ship
			pp:cargoRm( v.name, v.q )
		end
	end

	local desired_fuel = template:stats().fuel_consumption
	local reserved_fuel
	if pp:stats().fuel > desired_fuel + pp:stats().fuel_consumption then
		reserved_fuel = desired_fuel
		-- unfuel the ship before we hand it over
		pp:setFuel(pp:stats().fuel - reserved_fuel)
	end

	-- create and swap to the shuttle here
	pp:hookClear() -- clear player hooks to prevent errors
	local acquired = fmt.f(_("The shuttle bay of your {mothership}."), { mothership = player:ship() } )

	local shuttle_name = fmt.f( _("{name}'s Shuttle"), {name = player:ship() } )
	local newship = player.addShip("Cargo Shuttle", shuttle_name, acquired, true)
	player.swapShip( newship , false, false)

	-- fix the velocity vector and direction
	player.pilot():setVel(joyride.vel)
	player.pilot():setDir(joyride.dir)

	-- perform refit
	pp = player.pilot()
	pp:setFuel(0)	-- don't start with free fuel
	pp:outfitRm( "all" )
	pp:outfitRm( "cores" )

	for _j, o in ipairs( template:outfits() ) do
		pp = player.pilot() -- not sure why I'm doing this, but swapship.swap#116 does this
		pp:outfitAdd(o, 1 , true)
	end
	player.allowSave(false)
	der.sfx.unboard:play()
	template:rm()

	-- create the player's ship in space
	spawn_ghost()
	joyride.pilot:changeAI( "escort_guardian" )

	-- unregister the info button, need to hail the mothership now
	hideButton()
	player.allowLand ( false, _("The shuttle isn't equipped with landing gear.") )
	player.pilot():setNoJump(true)

	-- risky
	player.pilot():hookClear() -- clear player hooks to prevent errors

	player.pilot():setHealth(100, 75, 25)
	player.pilot():setEnergy(35)

	if reserved_fuel then
		pp:setFuel(reserved_fuel)
	else
		pp:setFuel(0)	-- don't start with free fuel
	end

	return true
end

function auxiliary_ship_return()
	if joyride.pilot and joyride.pilot:exists() then
		-- make sure we are in the shuttle (we reserve this variable when we are out of the mothership)
		if player.pilot():ship() ~= ship.get("Cargo Shuttle") then
			vntk.msg( _("Docking Error"), _("The ship you are in doesn't appear to have the necessary adjustments to fit inside the docking bay. Whatever you've done with the shuttle, you'd better bring it back if you want to get back on your ship."))
			-- player doesn't get to return
			player.commClose()
			return false -- pun not intended
		end

		-- we are redocking, save the current outfit layout
		shuttle_outfits = {}
		for j, o in ipairs(player.pilot():outfits()) do
			shuttle_outfits[#shuttle_outfits + 1] = o:nameRaw()
		end
		local carried_fuel = player.pilot():stats().fuel
		-- the player goes back into the captain's seat
		-- bringing any cargo along
		player.swapShip(joyride.mothership, false, true)
		-- copy the vector
		player.pilot():setDir(joyride.pilot:dir())
		player.pilot():setVel(joyride.pilot:vel())
		player.pilot():setFuel(player.pilot():stats().fuel + carried_fuel)

		-- put the cargo back
		local cl = joyride.pilot:cargoList()
		-- goes back into the player
		for k,v in pairs( cl ) do
			joyride.pilot:cargoRm( v.name, v.q )
			player.pilot():cargoAdd( v.name, v.q )
		end
		joyride.pilot:rm()
		player.allowSave(true)
		der.sfx.board:play()
		player.allowLand ( true )
		showButton()
	end
	joyride = nil
end

local infobtn

local function hideButton()
   if infobtn then
      player.infoButtonUnregister( infobtn )
      infobtn = nil
   end
end

local function showButton()
	hideButton()
	infobtn = player.infoButtonRegister( _("Launch Auxiliary Ship"), auxiliary_ship_mission, 3, "S" )
end