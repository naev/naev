--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Shadowcomm">
  <trigger>enter</trigger>
  <chance>3</chance>
  <cond>system.cur():presence("hostile") &lt; 300 and player.misnDone("Shadowrun") and not (player.misnDone("Shadow Vigil") or player.misnActive("Shadow Vigil")) and not (system.cur() == system.get("Pas"))</cond>
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

require "proximity"
require "missions/shadow/common"


-- localization stuff, translators would work here
title = {}
text = {}

title[1] = _("An open invitation")
text[1] = _([["Greetings, %s," the pilot of the Vendetta says to you as soon as you answer his hail. "I have been looking for you on behalf of an acquaintance of yours. She wishes to meet with you at a place of her choosing, and a time of yours. It involves a proposition that you might find interesting - if you don't mind sticking your neck out."
    You frown at that, but you ask the pilot where this acquaintance wishes you to go anyway.
    "Fly to the %s system," he replies. "She will meet you there. There's no rush, but I suggest you go see her at the earliest opportunity."
    The screen blinks out and the Vendetta goes about its business, paying you no more attention. It seems there's someone out there who wants to see you, and there's only one way to find out what about. Perhaps you should make a note of the place you're supposed to meet her: the %s system.]])

log_text = _([[Someone has invited you to meet with her in the Pas system, supposedly an acquaintance of yours. The pilot who told you this said that there's no rush, "but I suggest you go see her at the earliest opportunity".]])

yesnotxt = _("Do you intend to respond to the invitation?")


function create ()
     -- Claim: duplicates the claims in the mission.
    misssys = {system.get("Qex"), system.get("Shakar"), system.get("Borla"), system.get("Doranthex")}
    if not evt.claim(misssys) then
        abort()
    end

    sysname = "Pas"

    -- Create a Vendetta who hails the player after a bit
    hail_time = nil
    vendetta = pilot.add( "Vendetta", "Four Winds", true, _("Four Winds Vendetta"), {ai="trader"} )
    vendetta:control()
    vendetta:follow(player.pilot())
    hook.timer(500, "proximityScan", {focus = vendetta, funcname = "hailme"})

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

    tk.msg(title[1], text[1]:format(player.name(), sysname, sysname))
    player.commClose()
    vendetta:control()
    vendetta:hyperspace()

    if tk.yesno( "", yesnotxt ) then
        shadow_addLog( log_text )
        naev.missionStart("Shadow Vigil")
        evt.finish()
    end
end

-- Clean up
function finish()
    if hailhook then
        hook.rm(hailhook)
    end
    evt.finish()
end
