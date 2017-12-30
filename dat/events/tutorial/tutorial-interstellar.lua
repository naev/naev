-- This is the tutorial: interstallar flight.

include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
title1 = _("Tutorial: Interstellar Flight")
message1 = _([[Welcome to the interstellar flight tutorial.

In this tutorial you will learn how to travel from one star system to another, and how to use the galaxy map.]])
message2 = _([[In this tutorial we will leave the Mohawk system and travel the stars.

Each star system is connected to one or more other star systems by means of "jump points". These jump points are the ONLY places where you may enter hyperspace, and once you do you will appear near the jump point leading the other way in the destination system. Jump points are shown on your overlay map as triangles with the names of the destination systems next to them.

Let's select a jump point now. You can press %s to do this.]])
message3 = _([[Good. If you look at the bar at the bottom of the screen, you'll see that it has changed to show that you have selected a hyperspace target (next to "Nav"). Currently, the target is unknown, because you don't know anything about it yet. To actually use the jump point you've just selected you need to fly close to it and engage your hyperdrive. This is done with %s, but we're not going to do that right now, as there's an easier way.

For now, press %s to deselect your target. If you also have planets or ships targeted, you can use %s multiple times to select your most recent targets, in order.]])
message4 = _([[Another method of selecting a hyperspace target is by opening the galaxy map and clicking on the system you want to go to. The galaxy map is accessed by pressing %s, do so now.]])
message5 = _([[This is the galaxy map. It shows you a lot of information about the systems you've visited, and lets you plot jump routes to other star systems. Clicking a system will select it as your hyperspace target.

You'll see a button in the lower right that says "Autonav". Clicking this button will automatically make your ship fly to your selected destination. You can also engage the autonav at any time during flight by pressing %s. The autonav will continue to jump until it either reaches its destination, until you run out of fuel or until you come under enemy attack, whichever comes first.

Note that the autonav can also be used to fly to a planet or station. Simply target one and engage the autonav - this has the same effect as right-clicking on it on the map overlay.

Select a hyperspace target and jump to another system.]])
message6 = _([[Well done! You've successfully performed a hyperspace jump to another system. From here you can jump to yet other systems to continue your exploration.

But watch out! That jump just now has drained your fuel reserves, as the bottom bar will tell you. If you run out of fuel you can't jump anymore. You can get more fuel by refueling at a planet, by buying fuel off other ships, or by boarding disabled ships and stealing it fom them. Keep an eye on your fuel when traveling!

For the remainder of this tutorial, you will have unlimited fuel. Continue to make hyperjumps until you reach system Navajo.]])
message7 = _([[Excellent. You have learned how to make hyperspace jumps and explore the galaxy. You may continue jumping around if you wish. Once you're ready to move on, land on either Rin in the Navajo system, or Paul 2 in the Mohawk system. As a final tip, you can hold down %s while clicking on the galaxy map to specify a manual path for the autonav.

Congratulations! This concludes the interstellar flight tutorial.]])

thyperomsg = _("Press %s to target a jump point")
clearomsg = _("Press %s to deselect your target")
starmapomsg = _("Press %s to open the galaxy map")
hyperomsg = _("Select a hyperspace target with %s or by using the galaxy map, then press %s to engage the autonav (or jump manually with %s)")
hyperomsg2 = _("Continue exploring until you reach the Navajo system")
hyperomsg3 = _("Land (with %s) on Rin in the Navajo system or Paul 2 in the Mohawk system to complete the tutorial")

function create()
    -- Set up the player here.
    player.teleport("Mohawk")
    player.msgClear()

    player.pilot():setPos(planet.get("Paul 2"):pos() + vec2.new(0, 250))

    tk.msg(title1, message1)
    tkMsg(title1, message2:format(tutGetKey"thyperspace"))
    omsg = player.omsgAdd(thyperomsg:format(tutGetKey"thyperspace"), 0)

    keystage = "thyperspace"
    hinput = hook.input("input")
    hook.jumpin("jumpin")

end

-- Input hook.
function input(inputname, inputpress)
   nav, hyp = player.pilot():nav()
   if inputname == "thyperspace" and keystage == "thyperspace" then
      keystage = "deselect"
      player.omsgRm(omsg)
      tkMsg(title1, message3:format(tutGetKey("jump"), tutGetKey("target_clear"), tutGetKey("target_clear")))
      omsg = player.omsgAdd(clearomsg:format(tutGetKey("target_clear")), 0)
   elseif hyp == nil and keystage == "deselect" then
      keystage = "map"
      player.omsgRm(omsg)
      tkMsg(title1, message4:format(tutGetKey("starmap")))
      omsg = player.omsgAdd(starmapomsg:format(tutGetKey("starmap")), 0)
   elseif inputname == "starmap" and keystage == "map" then
      keystage = nil
      hook.rm(hinput)
      tk.msg(title1, message5:format(tutGetKey("autonav")))
      player.omsgChange(omsg, hyperomsg:format(tutGetKey("thyperspace"), tutGetKey("autonav"), tutGetKey("jump")), 0)
      firstjump = true
   end
end

-- Jumpin hook.
function jumpin()
    if not firstjump then player.refuel() end
    if system.cur() == system.get("Navajo") then
        hook.land( "land_clean" )
        hook.timer(2000, "jumpmsg", message7:format( _("\abshift\a0") ))
        player.omsgChange(omsg, hyperomsg3:format(tutGetKey("land")), 0)
    elseif firstjump then
        hook.timer(2000, "jumpmsg", message6)
        firstjump = false
        player.omsgChange(omsg, hyperomsg2, 0)
    end
end

-- Delay this tk.msg by a bit to make the jump less jarring.
function jumpmsg(message)
    tk.msg(title1, message)
end

-- Safe land function, takes off and cleans
function land_clean ()
   player.takeoff()
   hook.safe("cleanup")
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    if not (omsg == nil) then player.omsgRm(omsg) end
    naev.keyEnableAll()
    naev.eventStart("Tutorial")
    evt.finish(true)
end
