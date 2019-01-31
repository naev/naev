--[[
	Event for checking various system parameters.
	Activated from event.xml when needed for debug purposes.
	Prints out various system parameters.
--]]

-- =======================================================================================
--
-- Script-global initializations
-- 
-- =======================================================================================
--

-- Debug facility
include "debug/debug.lua"

-- Event prefix for variables
evtPrefix = "evt_Test_Landed_"

--
-- =======================================================================================


-- =======================================================================================
--
-- Declare script-global variables
-- to avoid declaration errors at procedure level
-- 
-- =======================================================================================
--

--
-- =======================================================================================


-- =======================================================================================
--
-- Setup strings
-- 
-- =======================================================================================
--

if lang == "es" then
	 -- not translated atm
else -- default english 
end

--
-- =======================================================================================


-- =======================================================================================
--
-- Event standard functions, called by the engine
--	create()	From event.xml
-- 
-- =======================================================================================
--

-- Create the event
function create ()
	local strPrefix = "TEST Landed Event : function create()"
	local boolDebug = false

	-- DEBUG
	dbg.stdOutput( strPrefix, -1, "entering", boolDebug )

	-- Get current system
	local currentSystem = system.cur()

	-- Get and print factions list
	local presencesList = currentSystem:presences()
	local factionsList = {}
	dbg.stdOutput( strPrefix, 1, "", boolDebug )
	dbg.stdOutput( strPrefix, 1, string.format("Factions in system %s :",currentSystem:name()), boolDebug )
	for faction,presence in pairs(presencesList) do
		dbg.stdOutput( strPrefix, 2, string.format("%s (%u)",faction,presence), boolDebug )
		table.insert( factionsList, faction )
	end
	dbg.stdOutput( strPrefix, 1, string.format("End of factions list"), boolDebug )

	-- Test trade and outfit hooks
	dbg.stdOutput( strPrefix, 1, "", boolDebug )
	dbg.stdOutput( strPrefix, 1, "Setting hooks", boolDebug )
	hook.takeoff("hook_takeoff")
	hook.comm_buy("hook_comm_buy")
	hook.comm_sell("hook_comm_sell")
	hook.outfit_buy("hook_outfit_buy")
	hook.outfit_sell("hook_outfit_sell")

	dbg.stdOutput( strPrefix, 0, "exiting", boolDebug )
end

--
-- =======================================================================================


-- =======================================================================================
--
-- Event standard functions, called by the engine
--	hook_takeoff()
-- 
-- =======================================================================================
--

function hook_takeoff ()
	local strPrefix = "TEST Landed Event : function hook_takeoff()"
	local boolDebug = true

	-- DEBUG
	dbg.stdOutput( strPrefix, -1, "entering", boolDebug )

	dbg.stdOutput( strPrefix, 0, "exiting", boolDebug )
end

--
-- =======================================================================================


-- =======================================================================================
--
-- Event standard functions, called by the engine
--	hook_comm_buy()	From event.xml
-- 
-- =======================================================================================
--

function hook_comm_buy (strCommodity, numQuantity)
	local strPrefix = "TEST Landed Event : function hook_comm_buy()"
	local boolDebug = true

	-- DEBUG
	dbg.stdOutput( strPrefix, -1, "entering", boolDebug )

	dbg.stdOutput( strPrefix, 1, string.format( "You bought %d tons of %s", numQuantity, strCommodity ), boolDebug )

	dbg.stdOutput( strPrefix, 0, "exiting", boolDebug )
end

--
-- =======================================================================================


-- =======================================================================================
--
-- Event standard functions, called by the engine
--	hook_comm_sell()	From event.xml
-- 
-- =======================================================================================
--

function hook_comm_sell (strCommodity, numQuantity)
	local strPrefix = "TEST Landed Event : function hook_comm_sell()"
	local boolDebug = true

	-- DEBUG
	dbg.stdOutput( strPrefix, -1, "entering", boolDebug )

	dbg.stdOutput( strPrefix, 1, string.format( "You sold %d tons of %s", numQuantity, strCommodity ), boolDebug )

	dbg.stdOutput( strPrefix, 0, "exiting", boolDebug )
end

--
-- =======================================================================================


-- =======================================================================================
--
-- Event standard functions, called by the engine
--	hook_outfit_buy()	From event.xml
-- 
-- =======================================================================================
--

function hook_outfit_buy (strCommodity, numQuantity)
	local strPrefix = "TEST Landed Event : function hook_comm_buy()"
	local boolDebug = true

	-- DEBUG
	dbg.stdOutput( strPrefix, -1, "entering", boolDebug )

	dbg.stdOutput( strPrefix, 1, string.format( "You bought %d units of %s", numQuantity, strCommodity ), boolDebug )

	dbg.stdOutput( strPrefix, 0, "exiting", boolDebug )
end

--
-- =======================================================================================


-- =======================================================================================
--
-- Event standard functions, called by the engine
--	hook_comm_sell()	From event.xml
-- 
-- =======================================================================================
--

function hook_outfit_sell (strCommodity, numQuantity)
	local strPrefix = "TEST Landed Event : function hook_comm_sell()"
	local boolDebug = true

	-- DEBUG
	dbg.stdOutput( strPrefix, -1, "entering", boolDebug )

	dbg.stdOutput( strPrefix, 1, string.format( "You sold %d units of %s", numQuantity, strCommodity ), boolDebug )

	dbg.stdOutput( strPrefix, 0, "exiting", boolDebug )
end

--
-- =======================================================================================
