

-- Create the mission
function create()

	-- target destination
	planet = space.getPlanet( misn.factions() )

	-- mission generics
	misn_type = "Rush"
	misn.setTitle( "Rush Delivery to " .. planet )

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
	player.addCargo( carg_type, carg_mass )
	tk.msg( "Mission Accepted",
			string.format( "The workers load the %d tons of %s onto your ship.",
					carg_mass, carg_type ) )
	hook.land( "land" )
end

-- Land hook
function land()
	if space.landName() == planet then
		player.rmCargo( carg_type, carg_mass )
		player.pay( misn_reward )
		tk.msg( "Mission Accomplished",
				string.format( "The workers unload the %s at the docks.", carg_type ) )
		misn.finish()
	end
end

