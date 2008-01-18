


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
	misn.setReward( string.format( "%d credits",
				carg_mass * 1000 + rnd.int( 0, 5000 ) ) )

end
