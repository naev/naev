-- This is the tutorial: basic combat.

include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
title1 = _("Tutorial: Basic Combat")
message1 = _([[Welcome to the basic combat tutorial.

Combat is an important aspect of Naev, and you will have to fight off enemies sooner or later, no matter what career you decide to pursue. In this tutorial, you will learn the basic principles of combat.]])
message2 = _([[For this tutorial, you will be flying a Lancelot fighter. It comes equipped with two advanced laser cannons. You can fire your weapons by pressing %s. Try this now.]])
message3 = _([[You may have noticed that as you fired your weapons, you depleted your energy gauge. Most energy weapons use energy while firing, and when you run out of energy you can no longer use them. Energy recharges automatically over time.

Now we will examine another type of weapon that uses ammunition instead of energy. Ammunition does not automatically recharge, you will have to buy it on planets or stations.

You have been equipped with a Mace rocket launcher, which is treated as a secondary weapon by default. Fire it now using %s and watch its ammunition deplete.]])
message4 = _([[Let's take a closer look at the difference between primary and secondary weapons.

Open the info menu by pressing %s.]])
message5 = _([[The weapons tab on the info menu allows you to manage your weapons. You have ten weapon groups available to you. Each weapon may be assigned to any or all of these weapon groups, as either a primary or a secondary weapon. As you have just seen, primary weapons are fired with %s while secondary weapons are fired with %s. During flight, you may switch between weapon groups at any time by using the appropriate buttons (%s, %s, %s, %s, %s, %s, %s, %s, %s and %s).

Additionally, you may set weapon groups to fire when activated. If you do this, then you may fire the weapons in that weapon group simply by holding down the key for that weapon group. Your current weapon group will remain selected in this case.

Configure your weapons as you like now, or simply leave them as they are. Then close the info menu.]])
message6 = _([[A target practice drone has been placed in space close to you. This drone won't move or fight back. Your task is to fire your weapons at it until you destroy it.

Before you attack the drone, you should target it. To do so, you can use %s, which will target the nearest hostile enemy. You can also click on it with the mouse. It's a good idea to always use targeting in combat, because some weapons only work when you have a target, and you can tell your ship to face a targeted enemy by pressing %s.

Target the drone, then shoot at it until it is destroyed.]])
message8 = _([[Well done, you have just destroyed your enemy. You now know the basics of ship to ship combat. As the final part of this tutorial, you're going to fight against a live opponent. We've hired the best fighter pilot in the sector to test your mettle, he will jump into the system any moment now. Good luck, you're going to need it!]])
message9 = _([[Oh. Well, good job, you've defeated your opponent. Don't worry about him though, he made it out before his ship blew. He mentioned he was good at that. Maybe that should have given us a clue...]])
message11 = _([[You now know the basic principles of combat. As a final tip, you can target specific enemies at long range by clicking on them on the overlay map.

Congratulations! This concludes the basic combat tutorial.]])

wepomsg = _([[Use %s to test your weapons (%ds remaining)]])
infoomsg = _([[Use %s to to open the info menu]])

shield30 = {
   _("Bring on the pain!"),
   _("You're no match for the fearsome T. Practice!"),
   _("I've been shot by ships ten times your size!"),
   _("Let's get it on!"),
   _("I  haven't got all day."),
   _("You're as threatening as an unborn child!"),
   _("I've snacked on foes much larger than yourself!"),
   _("Who's your daddy? I am!"),
   _("You're less intimidating than a fruit cake!"),
   _("My ship is the best in the galaxy!"),
   _("Bow down before me, and I may spare your life!"),
   _("Someone's about to set you up the bomb."),
   _("Your crew quarters are as pungent as an over-ripe banana!"),
   _("I'm invincible, I cannot be vinced!"),
   _("You think you can take me?"),
   _("When I'm done, you'll look like pastrami!"),
   _("I've got better things to do. Hurry up and die!"),
   _("You call that a barrage? Pah!"),
   _("You mother isn't much to look at, either!"),
   _("You're not exactly a crack-shot, are you?"),
   _("I've had meals that gave me more resistance than you!"),
   _("You're a pathetic excuse for a pilot!"),
   _("Surrender or face destruction!"),
   _("That all you got?"),
   _("I am Iron Man!"),
   _("My shields are holding fine!"),
   _("You'd be dead if I'd remembered to pack my weapons!"),
   _("I'll end you!"),
   _("This is your last chance to surrender!"),
   _("I'm going to report you to the NPC Rights watchdog."),
   _("Keep going, see what happens!"),
   _("You don't scare me!"),
   _("What do you think this is, knitting hour?"),
   _("Go on, shoot me."),
   _("You can do it! I believe in you."),
   _("Shoot me!"),
   _("Okay, listen. I'm doing this for attention."),
   _("If you don't shoot me, I'll tell the galaxy your terrible secret!"),
}
armour31 = {
   _("Okay, that's about enough."),
   _("You can stop now."),
   _("I was wrong about you."),
   _("Forgive and forget?"),
   _("Let's be pals, I'll buy you an ale!"),
   _("Game over, you win."),
   _("I've got a wife and kids! And a cat!"),
   _("Surely you must have some mercy?"),
   _("Please stop!"),
   _("I'm sorry!"),
   _("Leave me alone!"),
   _("What did I ever do to you?"),
   _("I didn't sign up for this!"),
   _("Not my ship, anything but my ship!"),
   _("We can talk this out!"),
   _("I'm scared! Hold me."),
   _("Make the bad ship go away, mommy!"),
   _("If you don't stop I'll cry!"),
}

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
    pp:addOutfit("Nexus Dart 300 Engine", 1, true)
    pp:addOutfit("S&K Light Combat Plating", 1, true)
    pp:setEnergy(100)
    pp:setHealth(100, 100)
    pp:addOutfit("Laser Cannon MK2", 2)
    pp:setDir(90)
    player.msgClear()

    pp:setNoLand()
    pp:setNoJump()

    tk.msg(title1, message1)
    tk.msg(title1, message2:format(tutGetKey("primary")))

    waitenergy = true
    flytime = 10 -- seconds of fly time

    omsg = player.omsgAdd(wepomsg:format(tutGetKey("primary"), flytime), 0)
    hook.timer(1000, "flyUpdate")
    playerHeatLoss()
end

-- Make the player fire their weapons.
function flyUpdate()
    flytime = flytime - 1
    
    if waitenergy then 
        if flytime == 0 then
            waitenergy = false
            waitammo = true

            player.omsgRm(omsg)
            tk.msg(title1, message3:format(tutGetKey("secondary")))

            pp:rmOutfit("all")
            pp:addOutfit("Unicorp Mace Launcher", 1)

            flytime = 10
            omsg = player.omsgAdd(wepomsg:format(tutGetKey("secondary"), flytime), 0)
            hook.timer(1000, "flyUpdate")
        else
            player.omsgChange(omsg, wepomsg:format(tutGetKey("primary"), flytime), 0)
            hook.timer(1000, "flyUpdate")
        end
    elseif waitammo then
        if flytime == 0 then
            player.omsgRm(omsg)
            waitammo = false
            waitinfo = true
            
            tk.msg(title1, message4:format(tutGetKey("info")))
            omsg = player.omsgAdd(infoomsg:format(tutGetKey("info")), 0)

            hook.input("input")
        else
            player.omsgChange(omsg, wepomsg:format(tutGetKey("secondary"), flytime), 0)
            hook.timer(1000, "flyUpdate")
        end
    end
end

-- Input hook.
function input(inputname, inputpress)
    if inputname == "info" and inputpress and waitinfo then
        waitinfo = false
        pp:rmOutfit("all")
        pp:addOutfit("Unicorp Mace Launcher", 2)
        pp:addOutfit("Laser Cannon MK2", 2)
        
        player.omsgRm(omsg)
        tk.msg(title1, message5:format(tutGetKey("primary"), tutGetKey("secondary"), tutGetKey("weapset1"), tutGetKey("weapset2"), tutGetKey("weapset3"), tutGetKey("weapset4"), tutGetKey("weapset5"), tutGetKey("weapset6"), tutGetKey("weapset7"), tutGetKey("weapset8"), tutGetKey("weapset9"), tutGetKey("weapset0")))
        
        hook.timer(1, "dummypractice") -- Ugly way to detect the closure of the info menu.
    end
end

-- Hooked function, initiates drone target practice.
function dummypractice()
    drone = pilot.add("FLF Lancelot", "dummy", pp:pos() + vec2.new(200, 0))[1]
    drone:rename("Target drone")
    drone:setHostile()
    drone:setNodisable(true)
    drone:setVisplayer(true)
    hook.pilot(drone, "death", "dronedeath")
    hook.pilot(drone, "attacked", "dronedamage")
    tk.msg(title1, message6:format(tutGetKey("target_hostile"), tutGetKey("face")))
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
    tk.msg(title1, message8)

    pp:rmOutfit("all")
    pp:addOutfit("Unicorp Mace Launcher", 2)
    pp:addOutfit("Laser Cannon MK2", 2)

    captainTP = pilot.add("Civilian Llama", "baddie_norun")[1]
    captainTP:rename( _("Captain T. Practice") )
    captainTP:setHostile()
    captainTP:rmOutfit("all")
    captainTP:addOutfit("Laser Cannon MK0", 1)
    captainTP:setNodisable(true)
    captainTP:setVisplayer(true)
    captainTP:setHilight(true)
    hook.pilot(captainTP, "death", "captainTPdeath")
    taunthook = hook.timer(7000, "taunt")
end

-- Hook for Captain T. Practice's death.
function captainTPdeath()
    hook.rm(taunthook)
    hook.timer(4000, "captainTPrip")
end

-- Captain T. Practice is dead. Long live captain T. Practice.
function captainTPrip()
    tk.msg(title1, message9)
    tk.msg(title1, message11)
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
 
-- Special timer for the player to stop him from heating up.
function playerHeatLoss()
    pp:setTemp(0)
    hook.timer(500, "playerHeatLoss")
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    if not (omsg == nil) then player.omsgRm(omsg) end
    naev.keyEnableAll()
    naev.eventStart("Tutorial")
    evt.finish(true)
end
