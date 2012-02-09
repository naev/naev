-- This is the tutorial: activated outfits.

include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title1 = "Tutorial: Activated Outfits"
    message1 = [[Welcome to the activated outfits tutorial.

This tutorial explains how activated outfits differ from regular ones, and how to use them.]]
   message2 = [[Let's begin with some theory. In general, there exist three types of outfits: passive outfits, activated outfits and weapons. Weapons are covered in another tutorial. Let's examine passive and activated outfits here.

Passive outfits modify your ship some way, and are always in effect. Engine Reroutes, Generators, Armor and Fuel Pods are examples of passive outfits.

Activated outfits have three possible states they can be in: off, on and cooldown. While off or in cooldown, the outfit does nothing for you. It only weighs down your ship. When on, its effects are applied to your ship, and it will consume whatever resources it demands from your ship.]]
   message3 = [[In this tutorial, your ship has been equipped with a single Unicorp Jammer. This is an activated outfit that can help you evade incoming missiles, but only when it's active.

Before you can use it, you must first assign the Jammer to a group. The process for this is the same as with weapons. Open the information menu using %s, and select the weapons tab. Select a group you want to use for the activated outfit, then click on the outfit in the slots diagram. The group will switch from a weapon group to an activated outfit group automatically.

Once this is done, you can use the hotkey associated with the group (%s, %s, %s, %s, %s, %s, %s, %s, %s or %s) to turn the outfit on and off.

Assign your Jammer to a group now and activate it. Keep an eye on your energy reserves when it is active.]]
   message4 = [[As you can see, the Jammer drains your energy while it is on. Since you will need your energy for other things, it's a good idea to only activate it when you need it.

It is important to know that some activated outfits can remain on as long as you have the energy, while others will only stay on for a maximum amount of time. Also, some outfits will need to go through a cooldown period after shutting off, during which time you can't use them. When an outfit is in its cooldown period, you can look at its icon to see how far along it is.]]

   message5 = [[You now know how to assign an activated outfit to a group and how to turn it on. As a final tip, remember that you can assign as many activated outfits to the same group as you like. You can combine outfit effects this way.

Congratulations! This concludes the activated outfits tutorial.]]

   aoutomsg = "Assign your Jammer to a group in the information menu (%s) and turn it on."
end

function create()
    -- Set up the player here.
    player.teleport("Cherokee")
    pilot.clear()
    pilot.toggleSpawn(false) -- To prevent NPCs from getting targeted for now.
    system.get("Mohawk"):setKnown(false)

    pp = player.pilot()
    pp:setPos(vec2.new(0, 2000))
    player.swapShip("Lancelot", "Lancelot", "Paul 2", true, true)
    pp:rmOutfit("all")
    pp:addOutfit("Unicorp Jammer", 1)
    pp:setDir(90)
    player.msgClear()

    player.pilot():setNoLand()
    player.pilot():setNoJump()

    tk.msg(title1, message1)
    tk.msg(title1, message2)
    tk.msg(title1, message3:format(tutGetKey("info"), tutGetKey("weapset1"), tutGetKey("weapset2"), tutGetKey("weapset3"), tutGetKey("weapset4"), tutGetKey("weapset5"), tutGetKey("weapset6"), tutGetKey("weapset7"), tutGetKey("weapset8"), tutGetKey("weapset9"), tutGetKey("weapset0")))
    
    omsg = player.omsgAdd(aoutomsg:format(tutGetKey("info")), 0)
    
    hook.timer(500, "checkEnergy")
end

-- Checks if the player's energy is lower than max, which indicates the outfit is active.
function checkEnergy()
   if pp:energy() < 100 then
      hook.timer(4000, "finishTutorial")
   else
      hook.timer(500, "checkEnergy")
   end
end

function finishTutorial()
   player.omsgRm(omsg)
   tk.msg(title1, message4)
   tk.msg(title1, message5)
   cleanup()
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    if not (omsg == nil) then player.omsgRm(omsg) end
    naev.keyEnableAll()
    naev.eventStart("Tutorial")
    evt.finish(true)
end
