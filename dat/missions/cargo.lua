lang = naev.lang()
if lang == "es" then
	-- not translated atm
else -- default english
	misn_desc = "%s in the %s system needs a delivery of %d tons of %s."
	misn_reward = "%d credits"
	title = {}
	title[1] = "Cargo delivery to %s"
	title[2] = "Freight delivery to %s"
	title[3] = "Transport to %s"
	title[4] = "Delivery to %s"
	full_title = "Ship is full"
	full_msg = "Your ship is too full.  You need to make room for %d more tons if you want to be able to accept the mission."
	accept_title = "Mission Accepted"
	accept_msg = "The workers load the %d tons of %s onto your ship."
	toomany_title = "Too many missions"
	toomany_msg = "You have too many active missions."
	finish_title = "Succesful Delivery"
	finish_msg = "The workers unload the %s at the docks."
	miss_title = "Cargo Missing"
	miss_msg = "You are missing the %d tons of %s!."
end

		

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
		misn.finish(false)
	end
   system = space.getSystem( planet )

	-- mission generics
	misn_type = "Cargo"
	i = rnd.int(3)
	misn.setTitle( string.format(title[i+1], planet) )

	-- more mission specifics
	carg_mass = rnd.int( 10, 30 )
	i = rnd.int(1)
	if i==0 then carg_type = "Food"
	elseif i==1 then carg_type = "Ore"
	end
	misn.setDesc( string.format( misn_desc, planet, system, carg_mass, carg_type ) )
	reward = carg_mass * (750+rnd.int(250)) + rnd.int(5000)
	misn.setReward( string.format( misn_reward, reward ) )
end

-- Mission is accepted
function accept()
	if player.freeCargo() < carg_mass then
		tk.msg( full_title, string.format( full_msg, carg_mass-player.freeCargo() ))
	elseif misn.accept() then -- able to accept the mission, hooks BREAK after accepting
		carg_id = player.addCargo( carg_type, carg_mass )
		tk.msg( accept_title, string.format( accept_msg, carg_mass, carg_type ))
		hook.land( "land" ) -- only hook after accepting
	else
		tk.msg( toomany_title, toomany_msg )
	end
end

-- Land hook
function land()
	if space.landName() == planet then
		if player.rmCargo( carg_id ) then
			player.pay( reward )
			tk.msg( finish_title, string.format( finish_msg, carg_type ))
			misn.finish(true)
		else
			tk.msg( miss_title, string.format( miss_msg, carg_mass, carg_type ))
		end
	end
end

