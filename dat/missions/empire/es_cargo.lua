--[[

   Handles the randomly generated Empire cargo missions.

]]--
include "scripts/jumpdist.lua"
include "scripts/cargo_common.lua"

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   misn_desc = "The Empire needs to ship %d tonnes of %s to %s in the %s system by %s (%s left)."
   misn_reward = "%d credits"

   title_p1 = "ES: Cargo transport to %s in the %s system"
   title_p2 = [[ 
Cargo: %s (%d tonnes)
Jumps: %d
Travel distance: %d
Time limit: %s]]


   full = {}
   full[1] = "Ship is full"
   full[2] = "Your ship is too full. You need to make room for %d more tonnes if you want to be able to accept the mission."

   slow = {}
   slow[1] = "Too slow"
   slow[2] = [[This shipment must arrive within %s, but it will take at least %s for your ship to reach %s, and the Empire is not fond of delays.

Accept the mission anyway?]]

   msg_title = {}
   msg_title[1] = "Mission Accepted"
   msg_title[2] = "Too many missions"
   msg_title[3] = "Successful Delivery"
   msg_msg = {}
   msg_msg[1] = "The Empire workers load the %d tonnes of %s onto your ship."
   msg_msg[2] = "You have too many active missions."
   msg_msg[3] = "The Empire workers unload the %s at the docks."
   miss = {}
   miss[1]= "Cargo Missing"
   miss[2] = "You are missing the %d tonnes of %s!."
   miss[3] = "MISSION FAILED: You have failed to deliver the goods to the Empire on time!"

   osd_title = "Empire Shipping"
   osd_msg = {}
   osd_msg[1] = "Fly to %s in the %s system before %s."
   osd_msg[2] = "You have %s remaining."
   osd_msg1 = "Fly to %s in the %s system before %s."
   osd_msg2 = "You have %s remaining." -- Need to reuse.
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
    destplanet, destsys, numjumps, traveldist, cargo, tier = cargo_calculateRoute()
    if destplanet == nil then
       misn.finish(false)
    end
    if destplanet:faction() ~= faction.get( "Empire" ) then
        misn.finish(false)
    end

   -- mission generics
    stuperpx   = 0.15 - 0.015 * tier
    stuperjump = 11000 - 1000 * tier
    stupertakeoff = 15000
    timelimit  = time.get() + time.create(0, 0, traveldist * stuperpx + numjumps * stuperjump + stupertakeoff)

    
    -- Choose amount of cargo and mission reward. This depends on the mission tier.
    finished_mod = 2.0 -- Modifier that should tend towards 1.0 as naev is finished as a game
    amount     = rnd.rnd(10 + 3 * tier, 20 + 4 * tier) 
    jumpreward = 1000
    distreward = 0.15
    reward     = 1.5^tier * (numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())
    
    misn.setTitle("ES: Cargo transport (" .. amount .. " tonnes of " .. cargo .. ")")
    misn.markerAdd(destsys, "computer")
    misn.setDesc(title_p1:format(destplanet:name(), destsys:name()) .. title_p2:format(cargo, amount, numjumps, traveldist, (timelimit - time.get()):str()))
    misn.setReward(misn_reward:format(reward))

end

-- Mission is accepted
function accept()
   local playerbest = cargoGetTransit( timelimit, numjumps, traveldist )
   if timelimit < playerbest then
      if not tk.yesno( slow[1], slow[2]:format( (timelimit - time.get()):str(), (playerbest - time.get()):str(), destplanet:name()) ) then
         misn.finish()
      end
   end
   if pilot.cargoFree(player.pilot()) < amount then
      tk.msg( full[1], string.format( full[2], amount-pilot.cargoFree(player.pilot()) ))
      misn.finish()
   end

   if misn.accept() then -- able to accept the mission, hooks BREAK after accepting
      carg_id = misn.cargoAdd( cargo, amount )
      tk.msg( msg_title[1], string.format( msg_msg[1], amount, cargo ))
      osd_msg[1] = osd_msg1:format(destplanet:name(), destsys:name(), timelimit:str())
      osd_msg[2] = osd_msg2:format((timelimit - time.get()):str())
      misn.osdCreate(osd_title, osd_msg)
      hook.land( "land" ) -- only hook after accepting
      hook.date(time.create(0, 0, 100), "tick") -- 100STU per tick
   else
      tk.msg( msg_title[2], msg_msg [2] )
      misn.finish()
   end
end

-- Land hook
function land()
    if planet.cur() == destplanet then
         tk.msg( msg_title[3], string.format( msg_msg[3], cargo ))
        player.pay(reward)
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
    end
end

-- Date hook
function tick()
    if timelimit >= time.get() then
        -- Case still in time
        osd_msg[1] = osd_msg1:format(destplanet:name(), destsys:name(), timelimit:str())
        osd_msg[2] = osd_msg2:format((timelimit - time.get()):str())
        misn.osdCreate(osd_title, osd_msg)
    elseif timelimit <= time.get() then
        -- Case missed deadline
        player.msg(miss[3])
        abort()
    end
end

function abort ()
    misn.finish(false)
end
