lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   misn_desc = {}
   misn_desc[1] = "%s in the %s system needs a delivery of %d tons of %s."
   misn_desc[2] = "%s in the %s system needs a rush delivery of %d tons of %s before %s (%s left)."
   misn_reward = "%d credits"
   title = {}
   title[1] = "Cargo delivery to %s"
   title[2] = "Freight delivery to %s"
   title[3] = "Transport to %s"
   title[4] = "Delivery to %s"
   title[5] = "Rush Delivery to %s"
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
   misn_time_msg = "MISSION FAILED: You have failed to delivery the goods on time!"
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
   misn_dist = space.jumpDist( system )

   -- mission generics
   i = rnd.int(4)
   if i < 3 then -- cargo delivery
      misn_type = "Cargo"
      i = rnd.int(3)
      misn.setTitle( string.format(title[i+1], planet) )
   else -- rush delivery
      misn_type = "Rush"
      misn.setTitle( string.format(title[5], planet) )
   end

   -- more mission specifics
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

   if misn_type == "Cargo" then
      misn.setDesc( string.format( misn_desc[1], planet, system, carg_mass, carg_type ) )
      reward = misn_dist * carg_mass * (250+rnd.int(150)) +
            carg_mass * (150+rnd.int(75)) +
            rnd.int(1500)
   elseif misn_type == "Rush" then
      misn_time = time.get() + time.units(2) +
            rnd.int(time.units(2), time.units(4)) * misn_dist
      misn.setDesc( string.format( misn_desc[2], planet, system,
            carg_mass, carg_type,
            time.str(misn_time), time.str(misn_time-time.get()) ) )
      reward = misn_dist * carg_mass * (450+rnd.int(250)) +
            carg_mass * (250+rnd.int(125)) +
            rnd.int(2500)
   end
   misn.setReward( string.format( misn_reward, reward ) )
end

-- Mission is accepted
function accept()
   if player.freeCargo() < carg_mass then
      tk.msg( full_title, string.format( full_msg, carg_mass-player.freeCargo() ))

   elseif misn.accept() then -- able to accept the mission, hooks BREAK after accepting
      carg_id = player.addCargo( carg_type, carg_mass )
      tk.msg( accept_title, string.format( accept_msg, carg_mass, carg_type ))

      -- set the hooks
      hook.land( "land" ) -- only hook after accepting
      if misn_type == "Rush" then -- rush needs additional time hook
         hook.time( "timeup" )
      end
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

-- Time hook
function timeup()
   if time.get() > misn_time then
      player.msg( misn_time_msg )
      misn.finish(false)
   end
   misn.setDesc( string.format( misn_desc[2], planet, system,
         carg_mass, carg_type,
         time.str(misn_time), time.str(misn_time-time.get()) ) )
end

