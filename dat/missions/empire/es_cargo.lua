--[[

   Handles the randomly generated Empire cargo missions.

]]--

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   misn_desc = "The Empire needs to ship %d tons of %s to %s in the %s system by %s (%s left)."
   misn_reward = "%d credits"
   title = {}
   title[1] = "ES: Ship to %s"
   title[2] = "ES: Delivery to %s"
   full = {}
   full[1] = "Ship is full"
   full[2] = "Your ship is too full. You need to make room for %d more tons if you want to be able to accept the mission."
   msg_title = {}
   msg_title[1] = "Mission Accepted"
   msg_title[2] = "Too many missions"
   msg_title[3] = "Successful Delivery"
   msg_msg = {}
   msg_msg[1] = "The Empire workers load the %d tons of %s onto your ship."
   msg_msg[2] = "You have too many active missions."
   msg_msg[3] = "The Empire workers unload the %s at the docks."
   miss = {}
   miss[1]= "Cargo Missing"
   miss[2] = "You are missing the %d tons of %s!."
   miss[3] = "MISSION FAILED: You have failed to deliver the goods to the Empire on time!"
end

--[[
--    Empire shipping missions are always timed, but quite lax on the schedules
--    pays a bit more then the rush missions
--]]

-- Create the mission
function create()

   -- target destination
   local i = 0
   local landed, landed_sys = planet.get()
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
   misn.setMarker(sys,"rush") -- set system marker
   misn_dist = sys:jumpDist()

   -- mission generics
   misn_type = "Cargo"
   i = rnd.rnd(1)
   misn.setTitle( string.format(title[i+1], pnt:name()) )

   -- more mission specifics
   carg_mass = rnd.rnd( 10, 30 )
   i = rnd.rnd(12)
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

   misn_time = time.get() + time.units(5) +
         rnd.rnd(time.units(6), time.units(8)) * misn_dist
   misn.setDesc( string.format( misn_desc, carg_mass, carg_type,
         pnt:name(), sys:name(),
         time.str(misn_time), time.str(misn_time-time.get())) )
   reward = misn_dist * carg_mass * (500+rnd.rnd(250)) +
         carg_mass * (250+rnd.rnd(150)) +
         rnd.rnd(2500)
   misn.setReward( string.format( misn_reward, reward ) )
end

-- Mission is accepted
function accept()
   if player.cargoFree() < carg_mass then
      tk.msg( full[1], string.format( full[2], carg_mass-player.cargoFree() ))
      misn.finish()
   elseif misn.accept() then -- able to accept the mission, hooks BREAK after accepting
      carg_id = misn.cargoAdd( carg_type, carg_mass )
      tk.msg( msg_title[1], string.format( msg_msg[1], carg_mass, carg_type ))
      hook.land( "land" ) -- only hook after accepting
      hook.enter( "timeup" )
   else
      tk.msg( msg_title[2], msg_msg [2] )
      misn.finish()
   end
end

-- Land hook
function land()

   local landed = pnt.get()
   if landed == pnt then
      if misn.cargoRm( carg_id ) then
         player.pay( reward )
         tk.msg( msg_title[3], string.format( msg_msg[3], carg_type ))

         -- increase empire shipping mission counter
         n = var.peek("es_misn")
         if n ~= nil then
            var.push("es_misn", n+1)
         else
            var.push("es_misn", 1)
         end

         -- increase faction
         if player.getFaction("Empire") < 50 then
            player.modFaction("Empire", rnd.rnd(5))
         end

         misn.finish(true)
      else
         tk.msg( miss[1], string.format( miss[2], carg_mass, carg_type ))
      end
   end
end

-- Time hook
function timeup()
   if time.get() > misn_time then
      hook.timer(2000, "failed")
   else
      misn.setDesc( string.format( misn_desc, carg_mass, carg_type,
            pnt:name(), sys:name(),
            time.str(misn_time), time.str(misn_time-time.get())) )
   end
end

function failed ()
   player.msg( miss[3] )
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
