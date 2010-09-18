--[[

   Handles the randomly created cargo delivery missions.

]]--
lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   misn_desc = {}
   misn_desc[1] = "%s in the %s system needs a delivery of %d tons of %s."
   misn_desc[11] = "%s in the %s system needs a rush delivery of %d tons of %s before %s (%s left)."
   misn_desc[21] = "A group of %s needs to travel to %s in the %s system."
   misn_reward = "%d credits"
   title = {}
   title[1] = "Cargo delivery to %s"
   title[2] = "Freight delivery to %s"
   title[3] = "Transport to %s"
   title[4] = "Delivery to %s"
   title[11] = "Rush Delivery to %s"
   title[21] = "Transport %s to %s"
   title[22] = "Ferry %s to %s"
   full = {}
   full[1] = "Ship is full"
   full[2] = "Your ship is too full. You need to make room for %d more tons if you want to be able to accept the mission."
   accept_title = "Mission Accepted"
	--accept_msg = {}
 	--accept_msg[1] = "The workers load the %d tons of %s onto your ship."
 	--accept_msg[2] = "The %s board your ship."
 	--replaced with randomized dialogue below
   msg_title = {}
   msg_msg = {}
   msg_title[1] = "Too many missions"
   msg_msg[1] = "You have too many active missions."
   msg_title[2] = "Successful Delivery"
 	--msg_msg[2] = "The workers unload the %s at the docks."
 	--msg_msg[5] = "The %s leave your ship."
 	--replaced with randomized dialoge below
   msg_title[3] = "Cargo Missing"
   msg_msg[3] = "You are missing the %d tons of %s!."
   msg_msg[4] = "MISSION FAILED: You have failed to delivery the goods on time!"
	--make sure these are in matched pairs. Script will fall back to array with fewer strings if not, and will print a warning in stdout.txt
  	--For cargo missions
   accept_msg_list = {}
   accept_msg_list[1] = "The workers quickly load the %d tons of %s onto your ship."
   accept_msg_list[2] = "The workers slowly load the %d tons of %s onto your ship."
   msg_msg_list = {}
   msg_msg_list[1] = "The workers quickly unload the %s at the docks."
   msg_msg_list[2] = "The people slowly unload the %s at the docks."
   
	--[[For passenger missions
   accept_msg_pass_list = {}
   accept_msg_pass_list[1] = "A group of %s noisily boards your ship"
   accept_msg_pass_list[2] = "The %s march aboard your ship."
   msg_msg_pass_list = {}
   msg_msg_pass_list[1] = "Chattering up a storm, the %s amble out of the ship, leaving you in peace at last."
   msg_msg_pass_list[2] = "Thanking you for the ride, the %s leave your ship." ]]-- --Replaced with better randomization
   
   accept_msg_pass = ""
   msg_msg_pass = ""
   
	--Randomize passenger mission dialogue

	--Each pass_accept_p1 is paired with its corresponding pass_land_p1, and each pass_accept_p2 is paired with its corresponding pass_land_p2
	--The number of strings must be the same in each pair member, but the number of strings in p1 can be different than in p2. 
	--If this is violated, it will fall back to the size of the smaller area and print an error in stdout.txt
	--opening dialogue for accept
	pass_accept_p1 = {}
	pass_accept_p1[1] = "A talkative crowd of %s"
	pass_accept_p1[2] = "A somewhat apprehensive group of %s"
	pass_accept_p1[3] = "A group of %s"
	pass_accept_p1[4] = "A confident party of %s"

	-- ..."the %s"... (in-between text)

	--closing dialogue for accept
	pass_accept_p2 = {}
	pass_accept_p2[1] = "slowly makes its way to their seats aboard"
	pass_accept_p2[2] = "files into"
	pass_accept_p2[3] = "boards"
	pass_accept_p2[4] = "greets you before taking their seats in"

	--opening dialogue for landing
	pass_land_p1 = {}
	pass_land_p1[1] = "Chatting noisily"
	pass_land_p1[2] = "Relieved to have arrived"
	pass_land_p1[3] = "Somewhat spacesick"
	pass_land_p1[4] = "In high spirits"

	-- ..."the %s"... (in-between text)

	--closing dialog for landing
	pass_land_p2 = {}
	pass_land_p2[1] = "stroll down the boarding ramp of"
	pass_land_p2[2] = "march out of"
	pass_land_p2[3] = "disembark from"
	pass_land_p2[4] = "thank you for the ride before departing"

end

--Dialog picker for cargo missions
i_arg = 0;
function cargo_dialog()
	if #accept_msg_list ~= #msg_msg_list then
		-- print error to stdout.txt
		print("Number of cargo dialogue strings is not the same for both pairs", #accept_msg_list, "strings in accept_msg_list and", #msg_msg_list, "strings in msg_msg_list. Using the lower of the two sizes.")
		--use smaller array size
		if #accept_msg_list > #msg_msg_list then
			dialog_pick = rnd.int(1, #msg_msg_list)
		else
			dialog_pick = rnd.int(1, #accept_msg_list)
		end
	else
		dialog_pick = rnd.int(1, #accept_msg_list)  -- for cargo missions
	end
end
	--[[
	else
		if #accept_msg_pass_list ~= #msg_msg_pass_list then
			-- print error to stdout
			print("Number of passenger dialogue strings is not the same for both pairs", #accept_msg_pass_list, "strings in accept_msg_pass_list and", #msg_msg_pass_list, "strings in msg_msg_pass_list. Using the lower of the two sizes.")
			--use smaller array size
			if #accept_msg_pass_list > #msg_msg_pass_list then
				dialog_pick = rnd.int(1, #msg_msg_pass_list)
			else
				dialog_pick = rnd.int(1, #accept_msg_pass_list)
			end
		else
			dialog_pick = rnd.int(1, #accept_msg_pass_list)  -- for passenger missions
		end
	end ]]--  Replace with randomizer

--Dialog generator/picker for passenger missions
function passenger_dialog()
	if #pass_accept_p1 ~= #pass_land_p1 then
		--print error to stdout.txt
		print("Number of p1 passenger dialogue strings is not the same for both pairs: " .. #pass_accept_p1 .. " strings in pass_accept_p1 and " .. #pass_land_p1 ..  " strings in pass_land_p1. Using the lower of the two sizes.")
		--use smaller array size
		if #pass_accept_p1 > #pass_land_p1 then
			j = rnd.int(1, #pass_land_p1)	
		else
			j = rnd.int(1, #pass_accept_p1)
		end
	else
		j = rnd.int(1, #pass_accept_p1)
	end
	if #pass_accept_p2 ~= #pass_land_p2 then
		--print error to stdout.txt
		print("Number of p2 passenger dialogue strings is not the same for both pairs: " .. #pass_accept_p2 .. " strings in pass_accept_p2 and " .. #pass_land_p2 ..  " strings in pass_land_p2. Using the lower of the two sizes.")
		--use smaller array size
		if #pass_accept_p2 > #pass_land_p2 then
			k = rnd.int(1, #pass_land_p2)	
		else
			k = rnd.int(1, #pass_accept_p2)
		end
	else
		k = rnd.int(1, #pass_accept_p2)  -- Pick a pair of dialogue for p1 and p2
	end
	msg_accept_pass = pass_accept_p1[j] .. " " .. pass_accept_p2[k] .. " your vessel."
	msg_msg_pass = pass_land_p1[j] .. ", the %s " .. pass_land_p2[k] .. " your ship."
end


-- Create the mission
function create()
   landed, landed_sys = planet.get() -- Get landed planet

   -- Only 50% chance of appearing on Dvaered systems
   dv = faction.get("Dvaered")
   if landed:faction() == dv and rnd.int(1) == 0 then
      misn.finish(false)
   end

   -- target destination
   i = 0
   local s
   repeat
      pnt,sys = planet.get( misn.factions() )
      s = pnt:services()
      i = i + 1
   until (s["land"] and s["inhabited"] and landed_sys:jumpDist(sys) > 0) or i > 10
   -- infinite loop protection
   if i > 10 then
      misn.finish(false)
   end
   misn_dist = sys:jumpDist()

   -- mission generics
   --check cargo mission strings to see if there are the same number for accept and for completion)
   i = rnd.int(6)
   if i < 4 then -- cargo delivery
      misn_type = "Cargo"
      cargo_dialog()
      misn_faction = rnd.int(2)
      i = rnd.int(3)
      misn.setTitle( string.format(title[i+1], pnt:name()) )
      misn.markerAdd( sys, "computer" )
   elseif i < 6 then -- rush delivery
      misn_type = "Rush"
      cargo_dialog()
      misn_faction = rnd.int(5)
      misn.setTitle( string.format(title[11], pnt:name()) )
      misn.markerAdd( sys, "computer" )
   else -- people delivery :)
      misn_type = "People"
      misn_faction = rnd.int(1)
      passenger_dialog()
      carg_mass = 0
      i = rnd.int(5)
      if i < 2 then
         carg_type = "Colonists"
      elseif i < 4 then
         carg_type = "Tourists"
      else
         carg_type = "Pilgrims"
      end
      i = rnd.int(1)
      misn.setTitle( string.format(title[i+21], carg_type, pnt:name()) )
      misn.markerAdd( sys, "computer" )
   end

   -- more mission specifics
   if misn_type == "Cargo" or misn_type == "Rush" then
      carg_mass = rnd.int( 10, 30 )
      i = rnd.int(12) -- set the goods
      if i < 5 then
         carg_type = "Food"
      elseif i < 8 then
         carg_type = "Ore"
      elseif i < 10 then
         carg_type = "Industrial Goods"
      elseif i < 12 then
         carg_type = "Luxury Goods"
      else
         carg_type = "Medicine"
      end
   end

   -- Set reward and description
   if misn_type == "Cargo" then
      misn.setDesc( string.format( misn_desc[1], pnt:name(), sys:name(), carg_mass, carg_type ) )
      reward = misn_dist * carg_mass * (250+rnd.int(150)) +
            carg_mass * (150+rnd.int(75)) +
            rnd.int(1500)
   elseif misn_type == "Rush" then
      misn_time = time.get() + time.units(3) +
            rnd.int(time.units(3), time.units(5)) * misn_dist
      misn.setDesc( string.format( misn_desc[11], pnt:name(), sys:name(),
            carg_mass, carg_type,
            time.str(misn_time), time.str(misn_time-time.get()) ) )
      reward = misn_dist * carg_mass * (450+rnd.int(250)) +
            carg_mass * (250+rnd.int(125)) +
            rnd.int(2500)
   elseif misn_type == "People" then
      misn.setDesc( string.format( misn_desc[21], carg_type, pnt:name(), sys:name() ))
      reward = misn_dist * (1000+rnd.int(500)) + rnd.int(2000)
   end
   misn.setReward( string.format( misn_reward, reward ) )
end

-- Mission is accepted
function accept()
   if pilot.cargoFree(player.pilot()) < carg_mass then
      tk.msg( full[1], string.format( full[2], carg_mass-pilot.cargoFree(player.pilot()) ))
      misn.finish()

   elseif misn.accept() then -- able to accept the mission, hooks BREAK after accepting
      carg_id = misn.cargoAdd( carg_type, carg_mass )
      if misn_type == "People" then
         --tk.msg( accept_title, string.format( accept_msg_pass_list[dialog_pick], carg_type ))  -- Replace with even more randomized dialogue
         tk.msg( accept_title, string.format( msg_accept_pass, carg_type ))
      else
         tk.msg( accept_title, string.format( accept_msg_list[dialog_pick], carg_mass, carg_type ))
      end

      -- set the hooks
      hook.land( "land" ) -- only hook after accepting
      if misn_type == "Rush" then -- rush needs additional time hook
         hook.enter( "timeup" )
      end
   else
      tk.msg( msg_title[1], msg_msg[1] )
      misn.finish()
   end
end

-- Land hook
function land()
   landed = planet.get()
   if landed == pnt then
      if misn.cargoRm( carg_id ) then
         player.pay( reward )
         if misn_type == "People" then
           -- tk.msg( msg_title[2], string.format( msg_msg_pass_list[dialog_pick], carg_type ))   -- Replace with even more randomized dialogue
           tk.msg( msg_title[2], string.format( msg_msg_pass, carg_type ))
         else
            tk.msg( msg_title[2], string.format( msg_msg_list[dialog_pick], carg_type ))
         end

         -- modify the faction standing
         if player.getFaction("Trader") < 70 then
            player.modFactionRaw("Trader",misn_faction);
         end
         if player.getFaction("Independent") < 30 then
            player.modFactionRaw("Independent", misn_faction/2)
         end
         if player.getFaction("Empire") < 10 then
            player.modFaction("Empire", misn_faction/3)
         end

         misn.finish(true)
      else
         tk.msg( msg_title[3], string.format( msg_msg[3], carg_mass, carg_type ))
      end
   end
end

-- Time hook
function timeup ()
   if time.get() > misn_time then
      hook.timer(2000, "failed")
   else
      misn.setDesc( string.format( misn_desc[11], pnt:name(), sys:name(),
            carg_mass, carg_type,
            time.str(misn_time), time.str(misn_time-time.get()) ) )
   end
end


function failed ()
   player.msg( msg_msg[4] )
   if misn_type ~= "People" then
      misn.cargoJet( carg_id )
   end
   misn.finish(false)
end

function abort ()
   if misn_type ~= "People" then
      misn.cargoJet( carg_id )
   end
end

