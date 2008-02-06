--[[
--example of a simple delivery gotten at the bar
--]]


lang = naev.lang()
if lang == "es" then
	-- not translated atm
else -- default english
	misn_title = "No title"
	misn_reward = "No reward"
	misn_desc = "No description about %s"
	title = {}
	title[1] = "Sample Title"
	title[2] = misn_title
	title[3] = "You win"
	text = {}
	text[1] = "Sample Text"
	text[2] = "More sample text (briefing)"
	text[3] = "Congratulations on winning"
	room_title = "Not enough room"
	room_msg = "You need to make room for %d more tons of cargo if you want to be able to accept this mission."
end

-- run when the mission is created at the bar
function create()

	carg_type = "Food"
	carg_mass = rnd.int(4) -- 0 <= carg_mass <= 4

	-- Intro text
	if tk.yesno( title[1], text[1] ) then
		if player.freeCargo() < carg_mass then -- not enough room
			tk.msg( room_title, string.format(room_msg, carg_mass-player.freeCargo()) )
			misn.finish(false)
		end

		misn.accept() -- we gots mission

		dest = space.getPlanet( misn.factions() ) -- destination

		-- Mission details
		misn.setTitle( misn_title )
		misn.setReward( misn_credits )
		misn.setDesc( misn_desc )

		-- Flavour text and mini-briefing
		tk.msg( title[2], text[2] )

		-- Set up the goal
		hook.land("land")
	end
end

-- hook to check if mission is won
function land()
	if space.landName() == dest then
		if player.rmCargo(carg_id) then -- player still has the cargo
			tk.msg( title[3], text[3] )
			misn.finish(true)
		else -- player doesn't have the cargo
		end
	end
end

