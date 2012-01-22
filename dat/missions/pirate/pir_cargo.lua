--[[

   Handles the randomly generated Pirate cargo missions.

]]--

include "scripts/numstring.lua"
include "scripts/jumpdist.lua"

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   misn_desc = "%d tons of %s needs to be shiped to %s in the %s system by %s (%s left)."
   misn_reward = "%s credits"
   title = {}
   title[1] = "Pir: Ship to %s"
   title[2] = "Pir: Delivery to %s"
   full = {}
   full[1] = "Ship is full"
   full[2] = "Your ship is too full. You need to make room for %d more tons if you want to be able to accept the mission."
   msg_title = {}
   msg_title[1] = "Mission Accepted"
   msg_title[2] = "Too many missions"
   msg_title[3] = "Successful Delivery"
   msg_msg = {}
   msg_msg[1] = "%d tons of %s are loaded onto your ship."
   msg_msg[2] = "You have too many active missions."
   msg_msg[3] = "The %s is unloaded at the docks."
   msg_msg[4] = "The %s is passed through the airlock."
   miss = {}
   miss[1]= "Cargo Missing"
   miss[2] = "You are missing the %d tons of %s!."
   miss[3] = "MISSION FAILED: You have failed to deliver the goods to on time!"
end

--[[
--    Pirate shipping missions are often timed, but mostly lax on the schedules
--    doesn't pay as well as most missions
--]]

-- Create the mission
function create()
   -- Note: this mission does not make any system claims. 

   -- target destination
   local planets = {} 
   getsysatdistance( system.cur(), 1, 6,
       function(s)
           for i, v in ipairs(s:planets()) do
               if v:faction() == faction.get("Pirate") and v:canLand() then
                   planets[#planets + 1] = {v, s}
               end
           end 
           return false
       end ) 
   if #planets == 0 then abort() end -- Sanity in case no suitable planets are in range. 
   local index = rnd.rnd(1, #planets)
   dest = planets[index][1]
   sys = planets[index][2] 

   misn.markerAdd( sys, "computer" )
   misn_dist = sys:jumpDist()

   -- mission generics
   misn_type = "Cargo"
   i = rnd.rnd(1)
   misn.setTitle( string.format(title[i+1], pnt:name()) )

   -- more mission specifics
   carg_mass = rnd.rnd( 10, 30 )
   i = rnd.rnd(12)
   if i < 5 then
      carg_type = "Unmarked Boxes"
   elseif i < 8 then
      carg_type = "Weapons"
   elseif i < 10 then
      carg_type = "Drugs"
   elseif i < 12 then
      carg_type = "Exotic Animals"
   else
      carg_type = "Radioactive Materials"
   end

   misn_time = time.get() + time.units(5) +
         rnd.rnd(time.units(6), time.units(11)) * misn_dist
   misn.setDesc( string.format( misn_desc, carg_mass, carg_type,
         pnt:name(), sys:name(),
         time.str(misn_time), time.str(misn_time-time.get())) )
   reward = misn_dist * carg_mass * (500+rnd.rnd(250)) +
         carg_mass * (250+rnd.rnd(150)) +
         rnd.rnd(100) * i
   misn.setReward( string.format( misn_reward, numstring(reward) ) )
end

-- Mission is accepted
function accept()
   if pilot.cargoFree(player.pilot()) < carg_mass then
      tk.msg( full[1], string.format( full[2], carg_mass-pilot.cargoFree(player.pilot()) ))
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

--I don't know if there is (or should be) a counter for this. I'm only including it because it was in the empire shipping missions.
--         -- increase Pirate shipping mission counter
--         n = var.peek("pir_misn")
--         if n ~= nil then
--            var.push("pir_misn", n+1)
--         else
--            var.push("pir_misn", 1)
--         end

         -- increase faction
         if faction.playerStandingRaw("Pirate") < 50 then
            faction.modPlayerSingle("Pirate", rnd.rnd(5))
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
   misn.finish(false)
end
