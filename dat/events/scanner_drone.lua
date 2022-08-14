--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Scanning Drone Handler">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
--[[
   Scanner Drone Event Handler

   Handler for the tiny drone bay that lets you scan enemy ships for their outfits
   To be deprecated and replaced in outfit/scanner_drone_bay.lua instead or with something even better!
--]]

-- luacheck: globals scan_drone_enter scan_drone_update find_drone (Hook functions passed by name)

function create()
	hook.enter("scan_drone_enter")
end

local DRONE_SEARCH_INTERVAL = 6
function scan_drone_enter()
	hook.timer(DRONE_SEARCH_INTERVAL, "find_drone")
end

local function enable( t )
	mem.target = t
	return true
end

local function disable( p )
	mem.target = nil
	if mem.scanner_drone and mem.scanner_drone:exists() then
		mem.scanner_drone:follow( p )
	end
	return true
end

local function dock_drone()
--	if mem.scanner_drone and mem.scanner_drone:exists() then
--		mem.scanner_drone:rm()
--	end
	mem.scanner_drone = nil
	mem.target = nil
	hook.timer(DRONE_SEARCH_INTERVAL, "find_drone")
	if mem.scanning_hook then
		hook.rm(mem.scanning_hook)
		mem.scanning_hook = nil
	end
end

-- gets a random outfit from the mem.target
local function get_target_outfit()
	if mem.target and #mem.target:outfitsList() > 0 then
		local to = mem.target:outfitsList()
		return to[rnd.rnd(1, #to)]
	end
	return { name = function () return _("No signatures disrupted") end }
end

local SCAN_TIME = 8
local function do_scan( dt )
	if not mem.timer then
		mem.timer = SCAN_TIME
		return
	end
	mem.timer = mem.timer - dt
	if mem.timer <= 0 then
		mem.timer = nil
		local found_outfit = get_target_outfit()
		mem.scanner_drone:broadcast(found_outfit:name())
	end
end

local SCAN_RANGE_2 = 750 * 750
function scan_drone_update( dt, _rdt, _args )
	if mem.scanner_drone and mem.scanner_drone:exists() then
		if
			mem.target and mem.target:exists()
			and vec2.dist2(mem.scanner_drone:pos(), mem.target:pos()) < SCAN_RANGE_2
		then
			do_scan(dt)
		end
	elseif mem.scanner_drone then
		-- the drone died
		dock_drone()
	end
end

local function use_drone( p )
	if mem.scanner_drone and mem.scanner_drone:exists() then
		local t = p:target()
		if not t then return false end
		local _detected, scanned = p:inrange(t)
		if not scanned then
		 return false
		end

		if not mem.target then
			return enable(t)
		elseif mem.target ~= t then
			return disable(p)
		end

	elseif mem.scanner_drone and not mem.scanner_drone:exists() then
		-- it died
		return disable(p)
	end

	return true
end

function find_drone()
	if mem.scanning_hook then
		hook.rm(mem.scanning_hook)
		mem.scanning_hook = nil
	end
	for _i, follower in ipairs(player.pilot():followers()) do
		if follower:name() == _("Za'lek Scanner Drone") then
			mem.scanner_drone = follower
			mem.scanning_hook = hook.update("scan_drone_update")

			return use_drone(player.pilot())
		end
	end
	hook.timer(DRONE_SEARCH_INTERVAL, "find_drone")
	return false
end