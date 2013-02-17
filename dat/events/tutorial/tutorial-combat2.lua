-- This is the tutorial: missile combat.

include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title1 = "Tutorial: Missile Combat"
    message1 = [[Welcome to the missile combat tutorial.

Guided missile weapons behave differently from other weapons. This tutorial explains the basic mechanics and teaches you how to handle such weapons.]]
    message2 = [[You are currently equipped with a Unicorp Fury Launcher. Fury missiles are low-end interceptor missiles that excel at chasing down agile targets such as light fighters.

In order to fire your missile weapon, you must first target the ship you want to shoot at, and then keep the target within an arc in front of your ship. The easiest way to do this is by holding down the auto-face button (%s). While the target is in the lockon arc, your missile weapon will gradually establish a lock. Until this process is complete, you cannot fire the weapon.

Try this now on the mobile drone in your vicinity.]]
    message3 = [[Well done. While the weapon locked on, you may have noticed an indicator on your HUD showing you the lock-on progress. This indicator exists for all weapon systems that require a lock before firing. Note that the time it takes for the lock to be established depends on several factors, including the stealth rating of the target and how good the weapon is at locking on. The lock will also degrade if your target is not within the lockon arc, even if it has been fully established. If you lose the lock, you will have to get another lock before you can fire.

You may fire your missiles now. Note that missile launchers are treated as secondary weapons by default, so you should use the secondary fire button (%s) to fire them. Of course, you can modify this using the weapon groups.

Destroy the target drone.]]
    message4 = [[Very good. Of course, this was just a target drone. A mobile target will give you a better idea of how missiles behave in real combat. Fortunately, we have an excellent pilot on hand who can help out there. He assured us that reports of his recent obliteration in another tutorial were highly exaggerated, and we trust him implicitly on that.

You have been given two fully loaded Unicorp Fury Launchers. Engage Captain T. Practice with them. Notice how the missiles will actively seek out the target. It's possible to avoid guided missiles, even the fast Fury missiles, but it's not easy to do.]]
    message5 = [[Hmm, that was over quick. Oh well. You should now have a solid grasp on how to use missile weapons. As a final tip, turreted missile launchers have a full 360 degree lockon arc, so all you have to do to acquire a lock is target an enemy.

Congratulations! This concludes the missile combat tutorial.]]

    shield30 = {
        "Bring on the pain!",
        "You're no match for the fearsome T. Practice!",
        "I've been shot by ships ten times your size!",
        "Let's get it on!",
        "I  haven't got all day.",
        "You're as threatening as an unborn child!",
        "I've snacked on foes much larger than yourself!",
        "Who's your daddy? I am!",
        "You're less intimidating than a fruit cake!",
        "My ship is the best in the galaxy!",
        "Bow down before me, and I may spare your life!",
        "Someone's about to set you up the bomb.",
        "Your crew quarters are as pungent as an over-ripe banana!",
        "I'm invincible, I cannot be vinced!",
        "You think you can take me?",
        "When I'm done, you'll look like pastrami!",
        "I've got better things to do. Hurry up and die!",
        "You call that a barrage? Pah!",
        "You mother isn't much to look at, either!",
        "You're not exactly a crack-shot, are you?",
        "I've had meals that gave me more resistance than you!",
        "You're a pathetic excuse for a pilot!",
        "Surrender or face destruction!",
        "That all you got?",
        "I am Iron Man!",
        "My shields are holding fine!",
        "You'd be dead if I'd remembered to pack my weapons!",
        "I'll end you!",
        "... Right after I finish eating this bucket of fried chicken!",
        "Em 5 Fried Chicken. Eat only the best.",
        "This is your last chance to surrender!",
        "Don't do school, stay in milk.",
        "I'm going to report you to the NPC Rights watchdog.",
        "Keep going, see what happens!",
        "You don't scare me!",
        "What do you think this is, knitting hour?",
        "I mean, good lord, man.",
        "It's been several minutes of non-stop banter!",
        "Haven't I sufficiently annoyed you yet?",
        "Go on, shoot me.",
        "You can do it! I believe in you.",
        "Shoot me!",
        "Okay, listen. I'm doing this for attention.",
        "But if you don't shoot me, I'll tell the galaxy your terrible secret.",
    }
    armour31 = {
        "Okay, that's about enough.",
        "You can stop now.",
        "I was wrong about you.",
        "Forgive and forget?",
        "Let's be pals, I'll buy you an ale!",
        "Game over, you win.",
        "I've got a wife and kids! And a cat!",
        "Surely you must have some mercy?",
        "Please stop!",
        "I'm sorry!",
        "Leave me alone!",
        "What did I ever do to you?",
        "I didn't sign up for this!",
        "Not my ship, anything but my ship!",
        "We can talk this out!",
        "I'm scared! Hold me.",
        "Make the bad ship go away, mommy!",
        "If you don't stop I'll cry!",
        "Here I go, filling my cabin up with tears.",
        "U- oh it a-pears my te-rs hav- da--age t-e co-mand cons-le.",
        "I a- T. Pr-ct---! Y-- w--l fe-r m- na-e --- Bzzzt!"
    }
end

function create()
    -- Set up the player here.
    player.teleport("Cherokee")
    pilot.clear()
    pilot.toggleSpawn(false) -- To prevent NPCs from getting targeted for now.
    system.get("Mohawk"):setKnown(false)
    system.get("Iroquois"):setKnown(false)
    system.get("Navajo"):setKnown(false)

    pp = player.pilot()
    pp:setPos(vec2.new(0, 0))
    player.swapShip("Lancelot", "Lancelot", "Paul 2", true, true)
    pp:rmOutfit("all")
    pp:addOutfit("Milspec Orion 2301 Core System", 1, true)
    pp:addOutfit("Tricon Naga Mk3 Engine", 1, true)
    pp:addOutfit("Schafer & Kane Light Combat Plating", 1, true)
    pp:addOutfit("Unicorp Fury Launcher")
    pp:setEnergy(100)
    pp:setHealth(100, 100)
    pp:setDir(90)
    player.msgClear()

    player.pilot():setNoLand()
    player.pilot():setNoJump()

    tk.msg(title1, message1)
    tk.msg(title1, message2:format(tutGetKey("face")))
    dummypractice()
end

-- Hooked function, initiates drone target practice.
function dummypractice()
    drone = pilot.add("Civilian Llama", "dummy", player.pilot():pos() + vec2.new(400, 0))[1]
    drone:rename("Target drone")
    drone:setHostile()
    drone:setNodisable(true)
    drone:setVisplayer(true)
    drone:control()
    drone:rmOutfit("all")
    drone:addOutfit("Shield Nullifier")
    hook.pilot(drone, "death", "dronedeath")
    hook.pilot(drone, "attacked", "dronedamage")
    lockhook = hook.pilot(pp, "lockon", "lockon")
end

-- Lockon hook.
function lockon()
   tk.msg(title1, message3:format(tutGetKey("secondary")))
   hook.rm(lockhook)
end

-- Drone death hook.
function dronedeath()
    hook.timer(3000, "captainpractice")
end

-- Drone attack hook. To make sure it doesn't drift off.
function dronedamage()
    drone:setVel(vec2.new())
end

function captainpractice()
    pp:rmOutfit("all")
    pp:addOutfit("Unicorp Fury Launcher", 2)
    tk.msg(title1, message4)

    TPangle = math.pi
    dist = 700

    captainTP = pilot.add("Civilian Llama", "baddie_norun", player.pilot():pos() + vec2.new(math.cos(TPangle) * dist, math.sin(TPangle) * dist))[1]
    captainTP:rename("Captain T. Practice")
    captainTP:setHostile()
    captainTP:rmOutfit("all")
    captainTP:addOutfit("Laser Cannon MK0", 1)
    captainTP:setNodisable(true)
    captainTP:setVisplayer(true)
    captainTP:setHilight(true)
    captainTP:control()
    hook.pilot(captainTP, "idle", "moveTP")
    hook.pilot(captainTP, "death", "captainTPdeath")
    taunthook = hook.timer(7000, "taunt")

    moveTP()
end

-- Make Captain T. Practice circle the player, sort of.
function moveTP()
   TPangle = (TPangle + math.pi / 3) % (2 * math.pi)
   captainTP:goto(pp:pos() + vec2.new(math.cos(TPangle) * dist, math.sin(TPangle) * dist), false, true)
end

-- Hook for Captain T. Practice's death.
function captainTPdeath()
    hook.rm(taunthook)
    hook.timer(4000, "captainTPrip")
end

-- Captain T. Practice is dead. Long live captain T. Practice.
function captainTPrip()
    tk.msg(title1, message5)
    hook.safe( "cleanup" )
end

-- Taunt function.
function taunt()
    armour, shield = captainTP:health()
    if shield >= 40 then
        captainTP:comm(shield30[rnd.rnd(1, #shield30)])
    elseif armour >= 31 then
        captainTP:comm(armour31[rnd.rnd(1, #armour31)])
    end
    taunthook = hook.timer(4000, "taunt")
end
 
-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    if not (omsg == nil) then player.omsgRm(omsg) end
    naev.keyEnableAll()
    naev.eventStart("Tutorial")
    evt.finish(true)
end
