--[[

    This is the first of many planned eyecandy cinematics.
    In this one, there will be a battle between the Dvaered and the FLF in the Doranthex system.

]]--

lang = naev.lang()
if lang == "es" then
else
    --flf_taunt = "We will claim this system once and for all!"
    --dvaered_taunt = "Not if we can help it!"
end

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
    --evt.timerStart("FLFSpawn", 3000 + rnd.rnd(100,900))
    --evt.timerStart("FLFSpawn", 4000 + rnd.rnd(100,900))
    --evt.timerStart("FLFSpawn", 5000 + rnd.rnd(100,900))
    --evt.timerStart("FLFSpawn", 6000 + rnd.rnd(100,900))
    
    evt.timerStart("DvaeredSpawn", 12000)
    --evt.timerStart("DvaeredSpawn", 15000 + rnd.rnd(100,900))
    --evt.timerStart("DvaeredSpawn", 16000 + rnd.rnd(100,900))
    --evt.timerStart("DvaeredSpawn", 17000 + rnd.rnd(100,900))
    --evt.timerStart("DvaeredSpawn", 18000 + rnd.rnd(100,900))
end

function FLFSpawn ()
    if flfwave == 1 then
        --position = vec2.new(0,1000)
        source_system = tuoladis
        --player.msg(flf_taunt)
    elseif flfwave == 2 then
        --position = vec2.new(1000,0)
        source_system = tuoladis
    elseif flfwave == 3 then
        --position = vec2.new(0,0)
        source_system = dakron
    elseif flfwave == 4 then
        --position = vec2.new(0,-1000)
        source_system = dakron
    else
        --position = vec2.new(1000,-1000)
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
    
    --[[for foo, bar in pairs(flfguys[flfwave]) do
        bar:control()
        bar:goto(position)
        --bar:setFriendly()
    end]]--
    flfwave = flfwave + 1
    if flfwave <=5 then
        evt.timerStart("FLFSpawn", 1000 + rnd.rnd(100,900) )
    end
end

function DvaeredSpawn ()
    if dvaeredwave == 1 then
        source_system = tarsus
        --player.msg(dvaered_taunt)
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
    
    --[[for foo, bar in pairs(dvaeredguys[dvaeredwave]) do
        bar:control()
        --bar:setNeutral()
        attack_fleet(bar, flfguys[dvaeredwave])
        hook.pilot(bar,"idle", "DvaeredIdle")
    end
    for foo, bar in pairs(flfguys[dvaeredwave]) do
        attack_fleet(bar, dvaeredguys[dvaeredwave])
        hook.pilot(bar, "idle", "FLFIdle")
    end]]--
    dvaeredwave = dvaeredwave + 1
    if dvaeredwave <=5 then
        evt.timerStart("DvaeredSpawn", 3000 + rnd.rnd(100,900) )
    end
end

--[[function DvaeredIdle ()
    i = 1
    while i < dvaeredwave do
        for foo, bar in pairs(dvaeredguys[i]) do
            attack_fleet(bar, flfguys[i])
        end
        i = i + 1
    end
end

function FLFIdle ()
    q = 1
    while q <= 5 do
        for foo, bar in pairs(flfguys[q]) do
            attack_fleet(bar, dvaeredguys[q])
        end
        q = q + 1
    end
end

function attack_fleet(attacker, defender)
    sortlist(defender)
    attacker:attack(defender[1])
end

function sortlist(list)
    new_list = {}
    for foo, bar in pairs(list) do
        new_list[#new_list + 1] = bar
    end
    return new_list
end]]--