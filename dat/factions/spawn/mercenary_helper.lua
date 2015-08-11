--This script chooses the mercenaries that spawn
include("dat/factions/spawn/common.lua")

function spawnLtMerc(faction)
    local pilots = {}
    local r = rnd.rnd()

    if r < 0.1 then
        scom.addPilot( pilots, {"Lancelot","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Phalanx","Bountyhunter","mercenary","Civilian", faction}, 45 );
    elseif r < 0.3 then
        scom.addPilot( pilots, {"Hyena","Bountyhunter","mercenary","Civilian", faction}, 15 );
        scom.addPilot( pilots, {"Hyena","Bountyhunter","mercenary","Civilian", faction}, 15 );
        scom.addPilot( pilots, {"Shark","Bountyhunter","mercenary","Civilian", faction}, 20 );
    elseif r < 0.5 then
        scom.addPilot( pilots, {"Vendetta","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Shark","Bountyhunter","mercenary","Civilian", faction}, 20 );
    elseif r < 0.8 then
        scom.addPilot( pilots, {"Vendetta","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Lancelot","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Ancestor","Bountyhunter","mercenary","Civilian", faction}, 20 );
    else
        scom.addPilot( pilots, {"Vendetta","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Ancestor","Bountyhunter","mercenary","Civilian", faction}, 20 );
        scom.addPilot( pilots, {"Admonisher","Bountyhunter","mercenary","Civilian", faction}, 45 );
    end

    return pilots
end

function spawnMdMerc(faction)
    local pilots = {}
    local r = rnd.rnd()

    if r < 0.5 then
        scom.addPilot( pilots, {"Vendetta","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Ancestor","Bountyhunter","mercenary","Civilian", faction}, 20 );
        scom.addPilot( pilots, {"Vigilance","Bountyhunter","mercenary","Civilian", faction}, 70 );
    elseif r < 0.8 then
        scom.addPilot( pilots, {"Vendetta","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Lancelot","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Ancestor","Bountyhunter","mercenary","Civilian", faction}, 20 );
        scom.addPilot( pilots, {"Pacifier","Bountyhunter","mercenary","Civilian", faction}, 70 );
    else
        scom.addPilot( pilots, {"Vendetta","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Vigilance","Bountyhunter","mercenary","Civilian", faction}, 70 );
        scom.addPilot( pilots, {"Phalanx","Bountyhunter","mercenary","Civilian", faction}, 45 );
    end

    return pilots
end

function spawnBgMerc(faction)
    local pilots = {}

    -- Generate the capship
    r = rnd.rnd()
    if r < 0.5 then
        scom.addPilot( pilots, {"Kestrel","Bountyhunter","mercenary","Civilian", faction}, 90 );
    elseif r < 0.8 then
        scom.addPilot( pilots, {"Hawking","Bountyhunter","mercenary","Civilian", faction}, 105 );
    else
        scom.addPilot( pilots, {"Goddard","Bountyhunter","mercenary","Civilian", faction}, 120 );
    end

    -- Generate the escorts
    r = rnd.rnd()
    if r < 0.1 then
        scom.addPilot( pilots, {"Lancelot","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Phalanx","Bountyhunter","mercenary","Civilian", faction}, 45 );
    elseif r < 0.3 then
        scom.addPilot( pilots, {"Hyena","Bountyhunter","mercenary","Civilian", faction}, 15 );
        scom.addPilot( pilots, {"Hyena","Bountyhunter","mercenary","Civilian", faction}, 15 );
        scom.addPilot( pilots, {"Shark","Bountyhunter","mercenary","Civilian", faction}, 20 );
    elseif r < 0.5 then
        scom.addPilot( pilots, {"Vendetta","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Shark","Bountyhunter","mercenary","Civilian", faction}, 20 );
    elseif r < 0.8 then
        scom.addPilot( pilots, {"Vendetta","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Lancelot","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Ancestor","Bountyhunter","mercenary","Civilian", faction}, 20 );
    else
        scom.addPilot( pilots, {"Vendetta","Bountyhunter","mercenary","Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Ancestor","Bountyhunter","mercenary","Civilian", faction}, 20 );
        scom.addPilot( pilots, {"Admonisher","Bountyhunter","mercenary","Civilian", faction}, 45 );
    end

    return pilots
end
