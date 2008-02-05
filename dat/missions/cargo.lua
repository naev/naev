

-- Create the mission
function create()

	-- target destination
	local i = 0
	repeat
		planet = space.getPlanet( misn.factions() )
		i = i + 1
	until planet ~= space.landName() or i > 10
	-- infinite loop protection
	if i > 10 then
		misn.finish()
	end

	-- mission generics
	misn_type = "Cargo"
	i = rnd.int(0,3)
	if i == 0 then
		misn.setTitle( "Cargo delivery to " .. planet )
	elseif i == 1 then
		misn.setTitle( "Freight delivery to " .. planet )
	elseif i == 2 then
		misn.setTitle( "Transport to " .. planet )
	elseif i == 3 then
		misn.setTitle( "Delivery to " .. planet )
	end

	-- more mission specifics
	carg_mass = rnd.int( 10, 30 )
	carg_type = "Food"
	misn.setDesc( string.format(
				"%s needs a rush delivery of %d tons of %s by %s.",
				planet, carg_mass, carg_type, "SOMEDAY" ) )
	misn_reward = carg_mass * 1000 + rnd.int( 0, 5000 )
	misn.setReward( string.format( "%d credits", misn_reward ) )
end

-- Mission is accepted
function accept()
	if player.freeCargo() < carg_mass then
		tk.msg( "Ship is full",
			string.format("Your ship is too full.  You need to make room for %d more tons if you want to be able to accept the mission.",
				carg_mass-player.freeCargo()) )
	elseif misn.accept() then -- able to accept the mission, hooks BREAK after accepting
		carg_id = player.addCargo( carg_type, carg_mass )
		tk.msg( "Mission Accepted",
				string.format( "The workers load the %d tons of %s onto your ship.",
						carg_mass, carg_type ) )
		hook.land( "land" ) -- only hook after accepting
	else
		tk.msg( "Too many missions", "You have too many active missions." )
	end
end

-- Land hook
function land()
	if space.landName() == planet then
		if player.rmCargo( carg_id ) then
			player.pay( misn_reward )
			tk.msg( "Mission Accomplished",
					string.format( "The workers unload the %s at the docks.", carg_type ) )
			misn.finish()
		else
			tk.msg( "Where is the cargo?",
					string.format( "You are missing the %d tons of %s!.",
							carg_mass, carg_type ) )
		end
	end
end

