--[[

    This is the first of many planned eyecandy cinematics.
    In this one, there will be a battle between the Dvaered and the FLF in the Doranthex system.

]]--

function create ()
    dakron = system.get("Dakron")
    tarsus = system.get("Tarsus")
    torg = system.get("Torg")
    tuoladis = system.get("Tuoladis")
    ogat = system.get("Ogat")
    
    pilot.clear()
    pilot.toggleSpawn(false)
    
    flfguys = {}
    dvaeredguys = {}
    flfwave = 1
    dvaeredwave = 1    
    
    evt.timerStart("FLFSpawn", 3000)
    
    evt.timerStart("DvaeredSpawn", 12000)
    
    hook.jumpout("leave")
    hook.land("leave")
end

function FLFSpawn ()
    if flfwave == 1 then
        source_system = tuoladis
    elseif flfwave == 2 then
        source_system = tuoladis
    elseif flfwave == 3 then
        source_system = dakron
    elseif flfwave == 4 then
        source_system = dakron
    else
        source_system = dakron
    end
    
    flfguys[flfwave] = {}
    flfguys[flfwave][1] = pilot.add("FLF Vendetta", nil, source_system)[1]
    flfguys[flfwave][2] = pilot.add("FLF Vendetta", nil, source_system)[1]
    flfguys[flfwave][3] = pilot.add("FLF Vendetta", nil, source_system)[1]
    flfguys[flfwave][4] = pilot.add("FLF Vendetta", nil, source_system)[1]
    flfguys[flfwave][5] = pilot.add("FLF Pacifier", nil, source_system)[1]
    flfguys[flfwave][6] = pilot.add("FLF Lancelot", nil, source_system)[1]
    flfguys[flfwave][7] = pilot.add("FLF Lancelot", nil, source_system)[1]
    
    flfwave = flfwave + 1
    if flfwave <=5 then
        evt.timerStart("FLFSpawn", 1000 + rnd.rnd(100,900) )
    end
end

function DvaeredSpawn ()
    if dvaeredwave == 1 then
        source_system = tarsus
    elseif dvaeredwave == 2 then
        source_system = tarsus
    elseif dvaeredwave == 3 then
        source_system = torg
    elseif dvaeredwave == 4 then
        source_system = torg
    else
        source_system = ogat
    end
    dvaeredguys[dvaeredwave] = {}
    dvaeredguys[dvaeredwave][1] = pilot.add("Dvaered Vendetta", nil, source_system)[1]
    dvaeredguys[dvaeredwave][2] = pilot.add("Dvaered Vendetta", nil, source_system)[1]
    dvaeredguys[dvaeredwave][3] = pilot.add("Dvaered Vendetta", nil, source_system)[1]
    dvaeredguys[dvaeredwave][4] = pilot.add("Dvaered Ancestor", nil, source_system)[1]
    dvaeredguys[dvaeredwave][5] = pilot.add("Dvaered Ancestor", nil, source_system)[1]
    dvaeredguys[dvaeredwave][6] = pilot.add("Dvaered Vigilance", nil, source_system)[1]
    dvaeredguys[dvaeredwave][7] = pilot.add("Dvaered Vigilance", nil, source_system)[1]
    dvaeredguys[dvaeredwave][8] = pilot.add("Dvaered Goddard", nil, source_system)[1]
    
    dvaeredwave = dvaeredwave + 1
    if dvaeredwave <=5 then
        evt.timerStart("DvaeredSpawn", 3000 + rnd.rnd(100,900) )
    end
end

function leave () --event ends on player leaving the system or landing
    evt.finish()
end