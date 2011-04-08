--[[
-- Derelict Event, spawning either the FLF prelude mission string or the Dvaered anti-FLF string.
--]]

lang = naev.lang()
if lang == "es" then
    -- not translated atm
else -- default english 

-- Text
    text = {}
    title = {}
    
    broadcastmsgDV = "SOS. This is %s. Primary systems down. Requesting assistance."
    broadcastmsgFLF = "Calling all ships. This is %s. Engines down, ship damaged. Please help."
    shipnameDV = "Dvaered Patrol"
    shipnameFLF = "Frontier Patrol"
end 

function create()

    -- Create the derelicts One Dvaered, one FLF.
    pilot.toggleSpawn(false)
    pilot.clear()
    
    posDV = vec2.new(7400, 3000)
    posFLF = vec2.new(-10500, -8500)
    
    fleetDV = pilot.add("Dvaered Vendetta", "dummy", posDV)
    shipDV = fleetDV[1]
    fleetFLF = pilot.add("FLF Vendetta", "dummy", posFLF)
    shipFLF = fleetFLF[1]
    
    shipDV:disable()
    shipFLF:disable()
    
    shipDV:setHilight(true)
    shipFLF:setHilight(true)
    
    shipDV:setVisplayer()
    shipFLF:setVisplayer()
    
    shipDV:rename(shipnameDV)
    shipFLF:rename(shipnameFLF)
    
    timerDV = hook.timer(3000, "broadcastDV")
    timerFLF = hook.timer(12000, "broadcastFLF")
    
    boarded = false
    destroyed = false

    -- Set a bunch of vars, for no real reason
    var.push("flfbase_sysname", "Sigur") -- Caution: if you change this, change the location for base Sindbad in unidiff.xml as well!
    
    hook.pilot(shipDV, "board", "boardDV")
    hook.pilot(shipDV, "death", "deathDV")
    hook.pilot(shipFLF, "board", "boardFLF")
    hook.pilot(shipFLF, "death", "deathFLF")
    hook.enter("enter")
end

function broadcastDV()
    -- Ship broadcasts an SOS every 10 seconds, until boarded or destroyed.
    shipDV:broadcast(string.format(broadcastmsgDV, shipnameDV), true)
    timerDV = hook.timer(20000, "broadcastDV")
end

function broadcastFLF()
    -- Ship broadcasts an SOS every 10 seconds, until boarded or destroyed.
    shipFLF:broadcast(string.format(broadcastmsgFLF, shipnameFLF), true)
    timerFLF = hook.timer(20000, "broadcastFLF")
end

function boardFLF()
    if shipDV:exists() then
        shipDV:setHilight(false)
        shipDV:setNoboard(true)
    end
    shipFLF:setHilight(false)
    hook.rm(timerFLF)
    hook.rm(timerDV)
    player.unboard()
    naev.missionStart("Deal with the FLF agent") 
    boarded = true
end

function deathDV()
    hook.rm(timerDV)
    destroyed = true
    if not shipFLF:exists() then
        evt.finish(true)
    end
end

function boardDV()
    if shipFLF:exists() then
        shipFLF:setHilight(false)
        shipFLF:setNoboard(true)
    end
    shipDV:setHilight(false)
    hook.rm(timerDV)
    hook.rm(timerFLF)
    player.unboard()
    naev.missionStart("Take the Dvaered crew home") 
    boarded = true
end

function deathFLF()
    hook.rm(timerFLF)
    destroyed = true
    var.push("flfbase_flfshipkilled", true)
    if not shipDV:exists() then
        evt.finish(true)
    end
end

function enter()
    if boarded == true then
        evt.finish(true)
    else
        evt.finish(false)
    end
end
