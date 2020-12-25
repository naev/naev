--This script chooses the mercenaries that spawn
require("factions/spawn/common")

pbm = 0.05   --5% mercenaries

function spawnLtMerc(faction)
    local pilots = {}
    local r = rnd.rnd()

    if r < 0.1 then
        scom.addPilot( pilots, {"Lancelot", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Phalanx", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 45 );
    elseif r < 0.3 then
        scom.addPilot( pilots, {"Hyena", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 15 );
        scom.addPilot( pilots, {"Hyena", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 15 );
        scom.addPilot( pilots, {"Shark", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 20 );
    elseif r < 0.5 then
        scom.addPilot( pilots, {"Vendetta", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Shark", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 20 );
    elseif r < 0.8 then
        scom.addPilot( pilots, {"Vendetta", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Lancelot", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Ancestor", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 20 );
    else
        scom.addPilot( pilots, {"Vendetta", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Ancestor", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 20 );
        scom.addPilot( pilots, {"Admonisher", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 45 );
    end

    return pilots
end

function spawnMdMerc(faction)
    local pilots = {}
    local r = rnd.rnd()

    if r < 0.5 then
        scom.addPilot( pilots, {"Vendetta", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Ancestor", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 20 );
        scom.addPilot( pilots, {"Vigilance", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 70 );
    elseif r < 0.8 then
        scom.addPilot( pilots, {"Vendetta", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Lancelot", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Ancestor", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 20 );
        scom.addPilot( pilots, {"Pacifier", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 70 );
    else
        scom.addPilot( pilots, {"Vendetta", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Vigilance", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 70 );
        scom.addPilot( pilots, {"Phalanx", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 45 );
    end

    return pilots
end

function spawnBgMerc(faction)
    local pilots = {}

    -- Generate the capship
    r = rnd.rnd()
    if r < 0.5 then
        scom.addPilot( pilots, {"Kestrel", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 90 );
    elseif r < 0.8 then
        scom.addPilot( pilots, {"Hawking", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 105 );
    else
        scom.addPilot( pilots, {"Goddard", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 120 );
    end

    -- Generate the escorts
    r = rnd.rnd()
    if r < 0.1 then
        scom.addPilot( pilots, {"Lancelot", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Phalanx", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 45 );
    elseif r < 0.3 then
        scom.addPilot( pilots, {"Hyena", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 15 );
        scom.addPilot( pilots, {"Hyena", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 15 );
        scom.addPilot( pilots, {"Shark", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 20 );
    elseif r < 0.5 then
        scom.addPilot( pilots, {"Vendetta", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Shark", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 20 );
    elseif r < 0.8 then
        scom.addPilot( pilots, {"Vendetta", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Lancelot", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Ancestor", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 20 );
    else
        scom.addPilot( pilots, {"Vendetta", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 25 );
        scom.addPilot( pilots, {"Ancestor", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 20 );
        scom.addPilot( pilots, {"Admonisher", _("Bounty Hunter"), "mercenary", "Civilian", faction}, 45 );
    end

    return pilots
end
