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
   full_title = "Ship is full"
   full_msg = "Your ship is too full.  You need to make room for %d more tons if you want to be able to accept the mission."
   accept_title = "Mission Accepted"
   accept_msg = {}
   accept_msg[1] = "The workers load the %d tons of %s onto your ship."
   accept_msg[2] = "The %s board your ship."
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

   local landed = space.getPlanet() -- Get landed planet

   -- target destination
   local i = 0
   repeat
      planet,system = space.getPlanet( misn.factions() )
      i = i + 1
   until planet ~= landed or i > 10
   -- infinite loop protection
   if i > 10 then
      misn.finish(false)
   end
   misn.setMarker(system) -- mark the system
   misn_dist = system:jumpDist()

   -- mission generics
   i = rnd.int(6)
   if i < 4 then -- cargo delivery
      misn_type = "Cargo"
      misn_faction = rnd.int(2)
      i = rnd.int(3)
      misn.setTitle( string.format(title[i+1], planet:name()) )
   elseif i < 6 then -- rush delivery
      misn_type = "Rush"
      misn_faction = rnd.int(5)
      misn.setTitle( string.format(title[11], planet:name()) )
   else -- people delivery :)
      misn_type = "People"
      misn_faction = rnd.int(1)
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
      misn.setTitle( string.format(title[i+21], carg_type, planet:name()) )
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

   if misn_type == "Cargo" then
      misn.setDesc( string.format( misn_desc[1], planet:name(), system:name(), carg_mass, carg_type ) )
      reward = misn_dist * carg_mass * (250+rnd.int(150)) +
            carg_mass * (150+rnd.int(75)) +
            rnd.int(1500)
   elseif misn_type == "Rush" then
      misn_time = time.get() + time.units(2) +
            rnd.int(time.units(2), time.units(4)) * misn_dist
      misn.setDesc( string.format( misn_desc[11], planet:name(), system:name(),
            carg_mass, carg_type,
            time.str(misn_time), time.str(misn_time-time.get()) ) )
      reward = misn_dist * carg_mass * (450+rnd.int(250)) +
            carg_mass * (250+rnd.int(125)) +
            rnd.int(2500)
   else -- "People"
      misn.setDesc( string.format( misn_desc[21], carg_type, planet:name(), system:name() ))
      reward = misn_dist * (1000+rnd.int(500)) + rnd.int(2000)
   end
   misn.setReward( string.format( misn_reward, reward ) )
end

-- Mission is accepted
function accept()
   if player.freeCargo() < carg_mass then
      tk.msg( full_title, string.format( full_msg, carg_mass-player.freeCargo() ))
      misn.finish()

   elseif misn.accept() then -- able to accept the mission, hooks BREAK after accepting
      carg_id = player.addCargo( carg_type, carg_mass )

      if misn_type == "People" then
         tk.msg( accept_title, string.format( accept_msg[2], carg_type ))
      else
         tk.msg( accept_title, string.format( accept_msg[1], carg_mass, carg_type ))
      end

      -- set the hooks
      hook.land( "land" ) -- only hook after accepting
      if misn_type == "Rush" then -- rush needs additional time hook
         hook.time( "timeup" )
      end
   else
      tk.msg( toomany_title, toomany_msg )
      misn.finish()
   end
end

-- Land hook
function land()
   local landed = space.getPlanet()
   if landed == planet then
      if player.rmCargo( carg_id ) then
         player.pay( reward )
         tk.msg( finish_title, string.format( finish_msg, carg_type ))

         -- modify the faction standing
         if player.getFaction("Merchant") < 70 then
            player.modFactionRaw("Merchant",misn_faction);
         end
         if player.getFaction("Independent") < 30 then
            player.modFactionRaw("Independent", misn_faction/2)
         end
         if player.getFaction("Empire") < 10 then
            player.modFaction("Empire", misn_faction/3)
         end

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
   misn.setDesc( string.format( misn_desc[21], planet:name(), system:name(),
         carg_mass, carg_type,
         time.str(misn_time), time.str(misn_time-time.get()) ) )
end

