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
   accept_msg = {}
   accept_msg[1] = "The workers load the %d tons of %s onto your ship."
   accept_msg[2] = "The %s board your ship."
   msg_title = {}
   msg_msg = {}
   msg_title[1] = "Too many missions"
   msg_msg[1] = "You have too many active missions."
   msg_title[2] = "Successful Delivery"
   msg_msg[2] = "The workers unload the %s at the docks."
   msg_msg[5] = "The %s leave your ship."
   msg_title[3] = "Cargo Missing"
   msg_msg[3] = "You are missing the %d tons of %s!."
   msg_msg[4] = "MISSION FAILED: You have failed to delivery the goods on time!"
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
   i = rnd.int(6)
   if i < 4 then -- cargo delivery
      misn_type = "Cargo"
      misn_faction = rnd.int(2)
      i = rnd.int(3)
      misn.setTitle( string.format(title[i+1], pnt:name()) )
      misn.setMarker(sys,"cargo") -- mark the system
   elseif i < 6 then -- rush delivery
      misn_type = "Rush"
      misn_faction = rnd.int(5)
      misn.setTitle( string.format(title[11], pnt:name()) )
      misn.setMarker(sys,"rush") -- mark the system
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
      misn.setTitle( string.format(title[i+21], carg_type, pnt:name()) )
      misn.setMarker(sys,"cargo") -- mark the system
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
   if player.cargoFree() < carg_mass then
      tk.msg( full[1], string.format( full[2], carg_mass-player.cargoFree() ))
      misn.finish()

   elseif misn.accept() then -- able to accept the mission, hooks BREAK after accepting
      carg_id = misn.cargoAdd( carg_type, carg_mass )

      if misn_type == "People" then
         tk.msg( accept_title, string.format( accept_msg[2], carg_type ))
      else
         tk.msg( accept_title, string.format( accept_msg[1], carg_mass, carg_type ))
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
            tk.msg( msg_title[2], string.format( msg_msg[5], carg_type ))
         else
            tk.msg( msg_title[2], string.format( msg_msg[2], carg_type ))
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

