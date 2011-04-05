--[[
-- Comm Event for the Shadow missions
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title = {}
    text = {}

    title[1] = "An open invitation"
    text[1] = [[    "Greetings, %s," the pilot of the Vendetta says to you as soon as you answer his hail. "I have been looking for you on behalf of an acquaintance of yours. She wishes to meet with you at a place of her choosing, and a time of yours. It involves a proposition that you might find interesting - if you don't mind sticking your neck out."
    You frown at that, but you ask the pilot where this acquaintance wishes you to go anyway.
    "Fly to the %s system," he replies. "She will meet you there. There's no rush - but I suggest you go see her at the earliest opportunity."
    The screen blinks out, and the Vendetta goes about its business, paying you no more attention. It seems there's someone out there who wants to see you, and there's only one way to find out what about.]]

end

function create ()
    sysname = "Pas"
    destsys = system.get(sysname)
    
    hailed = false
    
    vendetta = pilot.add("Four Winds Vendetta", nil, true)[1]
    
    hailie = hook.timer(3000, "hailme")

    var.push("shadowvigil_active", true) -- Make sure the event can't reappear while it's active

    hook1 = hook.pilot(vendetta, "jump", "finish")
    hook2 = hook.pilot(vendetta, "death", "finish")
    hook3 = hook.land("finish")
    hook4 = hook.jumpout("finish")
end

-- Make the ship hail the player
function hailme()
    vendetta:hailPlayer()
    hailhook = hook.pilot(vendetta, "hail", "hail")
end

-- Triggered when the player hails the ship
function hail(p)
    hailed = true
    tk.msg(title[1], string.format(text[1], player.name(), sysname))
    var.push("shadowvigil_active", true)
    hook.rm(hook1)
    hook.rm(hook2)
    hook.rm(hook3)
    hook.rm(hook4)
    hook.rm(hailhook)
    hook.jumpin("jumpin")
end

function jumpin()
    if system.cur() == destsys then
        seiryuu = pilot.add("Seiryuu", nil, vec2.new(0, -2000))[1]
        seiryuu:control(true)
        seiryuu:setActiveBoard(true)
        seiryuu:setInvincible(true)
        hook.pilot(seiryuu, "board", "board")
    end
end

function board()
    player.unboard()
    seiryuu:control(false)
    seiryuu:setActiveBoard(false)
    naev.missionStart("Shadow Vigil")
    finish()
end

function finish()
    if not hailed then
        var.pop("shadowvigil_active")
    end
    hook.rm(hailie)
    evt.finish()
end
