--[[

   Handles the randomly generated Empire cargo missions.

]]--
include "scripts/jumpdist.lua"
include "scripts/cargo_common.lua"

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   misn_desc = "The Empire needs to ship %d tons of %s to %s in the %s system by %s (%s left)."
   misn_reward = "%d credits"

   title_p1 = "ES: Cargo transport to %s in the %s system"
   title_p2 = [[ 
Cargo: %s (%d tons)
Jumps: %d
Travel distance: %d
Time limit: %s]]


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
   slow = {}
   slow[1] = "Ship Too Slow"
   slow[2] = [[Your ship will take at least %s to reach %s, %s past the deadline.

A faster ship is needed. Do you want to accept the mission anyway?]]
end

--[[
--    Empire shipping missions are always timed, but quite lax on the schedules
--    pays a bit more then the rush missions
--]]

-- Create the mission
function create()
   -- Note: this mission does not make any system claims.

    origin_p, origin_s = planet.cur()
    local routesys = origin_s
    local routepos = origin_p:pos()

    -- target destination
    -- Todo: Ensure selected planet is aligned to player faction    
    destplanet, destsys, numjumps, traveldist, cargo, tier = cargo_calculateRoute()
    if destplanet == nil then
       misn.finish(false)
    end

   -- mission generics
    stuperpx   = 0.15 - 0.015 * tier
    stuperjump = 11000 - 1000 * tier
    stupertakeoff = 15000
    timelimit  = time.get() + time.create(0, 0, traveldist * stuperpx + numjumps * stuperjump + stupertakeoff)
    timelimit2 = time.get() + time.create(0, 0, (traveldist * stuperpx + numjumps * stuperjump + stupertakeoff) * 1.2)

    
    -- Choose amount of cargo and mission reward. This depends on the mission tier.
    finished_mod = 2.0 -- Modifier that should tend towards 1.0 as naev is finished as a game
    amount     = rnd.rnd(10 + 3 * tier, 20 + 4 * tier) 
    jumpreward = 1000
    distreward = 0.15
    reward     = 1.5^tier * (numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())
    
    misn.setTitle("ES: Cargo transport (" .. amount .. " tons of " .. cargo .. ")")
    misn.markerAdd(destsys, "computer")
    misn.setDesc(title_p1:format(destplanet:name(), destsys:name()) .. title_p2:format(cargo, amount, numjumps, traveldist, (timelimit - time.get()):str()))
    misn.setReward(misn_reward:format(reward))

end

-- Mission is accepted
function accept()
   delay = time.get() + time.units(2) + (player.pilot():stats().jump_delay * 1000 * misn_dist)
   if pilot.cargoFree(player.pilot()) < carg_mass then
      tk.msg( full[1], string.format( full[2], carg_mass-pilot.cargoFree(player.pilot()) ))
      misn.finish()
   elseif delay > misn_time then
      if not tk.yesno( slow[1], string.format( slow[2], time.str(delay - time.get()),
            pnt:name(), time.str(delay - misn_time) )) then
         misn.finish()
      end
   end

   if misn.accept() then -- able to accept the mission, hooks BREAK after accepting
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
   if misn_type ~= "People" and carg_id ~= nil then
      misn.cargoJet( carg_id )
   end
end
