--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Shadowcomm">
  <trigger>enter</trigger>
  <chance>3</chance>
  <cond>system.cur():presence("hostile") &lt; 300 and player.misnDone("Shadowrun") and not (player.misnDone("Shadow Vigil") or var.peek("shadowvigil_active") == true)</cond>
  <flags>
  </flags>
  <notes>
   <done_misn name="Shadowrun"/>
   <campaign>Shadow</campaign>
   <tier>3</tier>
  </notes>
 </event>
 --]]
--[[
-- Comm Event for the Shadow missions
--]]

require "proximity.lua"
require "missions/shadow/common.lua"


-- localization stuff, translators would work here
title = {}
text = {}

title[1] = _("An open invitation")
text[1] = _([["Greetings, %s," the pilot of the Vendetta says to you as soon as you answer his hail. "I have been looking for you on behalf of an acquaintance of yours. She wishes to meet with you at a place of her choosing, and a time of yours. It involves a proposition that you might find interesting - if you don't mind sticking your neck out."
    You frown at that, but you ask the pilot where this acquaintance wishes you to go anyway.
    "Fly to the %s system," he replies. "She will meet you there. There's no rush, but I suggest you go see her at the earliest opportunity."
    The screen blinks out and the Vendetta goes about its business, paying you no more attention. It seems there's someone out there who wants to see you, and there's only one way to find out what about. Perhaps you should make a note of the place you're supposed to meet her: the %s system.]])
text[2] = _([["Greetings, %s," the pilot of the Vendetta says. "Sorry to bother you; I've just noticed that it's been quite some time since we contacted you and I wanted to check in on you.
    "As was mentioned previously, the one who wishes to meet you can be found in the %s system, which is near Empire space. Again, there's no rush, but I suggest you go to see her at the earliest opportunity."
    The screen blinks out and the Vendetta goes about its business, paying you no more attention. You make a mental note again to try to remember to go to the %s system when you get the chance.]])

log_text = _([[Someone has invited you to meet with her in the Pas system, supposedly an acquaintance of yours. The pilot who told you this said that there's no rush, "but I suggest you go see her at the earliest opportunity".]])


function create ()
    sysname = "Pas"
    destsys = system.get(sysname)

    -- Create a Vendetta who hails the player after a bit
    hail_time = nil
    vendetta = pilot.add("Four Winds Vendetta", nil, true)[1]
    vendetta:control()
    vendetta:follow(player.pilot())
    hook.timer(500, "proximityScan", {focus = vendetta, funcname = "hailme"})

    -- Make sure the event can't reappear while it's active
    var.push("shadowvigil_active", true)

    -- Clean up on events that remove the Vendetta from the game
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
    hook.rm(hailhook)

    if hail_time == nil then
        hail_time = time.get()
        tk.msg(title[1], text[1]:format(player.name(), sysname, sysname))
        shadow_addLog( log_text )

        -- The event should now remain active until Pas
        -- Clear the hooks that would otherwise finish it
        hook.rm(hook1)
        hook.rm(hook2)
        hook.rm(hook3)
        hook.rm(hook4)

        player.commClose()

        vendetta:control()
        vendetta:hyperspace()

        -- Catch the player jumping into Pas
        -- The player may save between now and then, make sure our hook is saved too
        evt.save(true)
        hook.enter("jumpin")
    else
        hail_time = time.get()
        tk.msg(title[1], text[2]:format(player.name(), sysname, sysname))
        player.commClose()
        vendetta:control()
        vendetta:hyperspace()
    end
end

function jumpin()
    if system.cur() == destsys then
        seiryuu = pilot.add("Seiryuu", nil, vec2.new(0, -2000))[1]
        seiryuu:control(true)
        seiryuu:setActiveBoard(true)
        seiryuu:setInvincible(true)
        seiryuu:setHilight(true)
        seiryuu:setVisplayer(true)
        hook.pilot(seiryuu, "board", "board")
    elseif (hail_time == nil or time.get() > hail_time + time.create(0, 200, 0))
            and rnd.rnd() < 0.1 then
        vendetta = pilot.add("Four Winds Vendetta", nil, true)[1]
        vendetta:control()
        vendetta:follow(player.pilot())
        hook.timer(500, "proximityScan", {focus = vendetta, funcname = "hailme"})
    end
end

-- The player boards the Seiryuu
function board()
    player.unboard()
    seiryuu:control(false)
    seiryuu:setActiveBoard(false)
    naev.missionStart("Shadow Vigil")
    evt.finish()
end

-- Clean up
function finish()
    if hail_time == nil then
        var.pop("shadowvigil_active")
    end
    if hailhook then
        hook.rm(hailhook)
    end
    evt.finish()
end
