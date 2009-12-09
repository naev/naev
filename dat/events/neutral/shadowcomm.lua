--[[
-- Comm Event for the Shadow missions
--]]


function create ()
    vendetta = pilot.add("Four Winds Vendetta")[1]
    
    hook.pilot(vendetta, "jump", "finish")
    hook.pilot(vendetta, "death", "finish")
    hook.land("finish")
    hook.jumpout("finish")
    
    first = true
    hailed = false

    hailie = evt.timerStart("hailme", 3000)
    hook.time("finish")
end

-- Make the ship hail the player
function hailme()
    vendetta:hailPlayer()
    hook.pilot(vendetta, "hail", "hail")
end

-- Triggered when the player hails the ship
function hail()
    tk.msg(title[1], string.format(text[1], player.name(), sysname))
    hook.jumpin("jumpin")
end

function jumpin()
    if system.cur() == destsys then
        var.push("shadowstrike_first", first)
        seiryuu = pilot.add("Seiryuu", "trader", vec2.new(0, 0), false)[1] -- TODO: Big system position.
        seiryuu:disable()
        seiryuu:setInvincible(true)
        hook.pilot(seiryuu, "board", "board")
    end
end

function board()
    evt.misnStart("Shadow Strike")
    first = false()
end

function finish()
    evt.timerStop(hailie)
    if not hailed then
        evt.finish()
    end
end