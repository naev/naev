--[[

    This is the first of many planned eyecandy cinematics.
    In this one, there will be a battle between the Dvaered and the FLF in the Doranthex system.

]]--

function create ()
    pilot.clear()
    pilot.toggleSpawn(false)
    
    flfguys = {}
    dvaeredguys = {}
    flfwave = 1
    dvaeredwave = 1    
    
    hook.timer(3000, "FLFSpawn")
    
    hook.timer(12000, "DvaeredSpawn")
    
    hook.jumpout("leave")
    hook.land("leave")
end

function FLFSpawn ()
    source_system = system.get("Zacron")

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
        hook.timer(1000, "FLFSpawn")
    end
end

function DvaeredSpawn ()
    source_system = system.get("Doranthex")

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
        hook.timer(3000, "DvaeredSpawn")
    end
end

function leave () --event ends on player leaving the system or landing
    evt.finish()
end
