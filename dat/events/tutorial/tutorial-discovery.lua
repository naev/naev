-- This is the tutorial: exploration and discovery

include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
title1 = _("Tutorial: Exploration and Discovery")
message1 = _([[Welcome to the exploration and discovery tutorial.

This tutorial will tell you a little about the exploration aspect of the game.]])
message2 = _([[In this tutorial, you have been placed in a high-interference system. You may notice that visibility is fairly low here. Some systems, such as this one, have a certain degree of sensor interference. Sensor interference does what you would expect it to do - it lowers the range of your sensors.

Open your system map (%s key).]])
message3 = _([[You're looking at the system map for this system. But it appears empty! This is because at the moment, your sensors haven't picked up anything yet. The interference is too strong to see anything from here.

A marker has been placed on your map. Navigate toward it. Remember, you can right-click on a location to get there faster.]])
message4 = _([[Something just happened. You discovered a planet in this system, called Tein. Planets are the biggest objects you'll encounter in any system, so you will usually discover them before anything else.

Keep going.]])
message5 = _([[You've discovered something else, Tein's moon Rein. Moons are a lot smaller than planets and are therefore harder to detect.

You now know where Tein and Rein are located. Their positions have been added to your ship's computer, so you won't have to search for them again. Even if you fly away from them again, you will still be able to see them on the map.

There is still one thing left in this system for you to find: the jump point leading to the next system. Jump points are generally much more difficult to find than planets or moons, especialy in a high interference system such as this one. Look at your map, the marker is now toward the right hand side. Fly there.]])
message6 = _([[Well done, you have found the jump point. As you can see, you had to get pretty close to it before it appeared! Just like with planets though, jump points will always be visible to you once you know where they are.]])
message7 = _([[Jump points are vitally important to pilots who want to travel through the galaxy, yet they can be hard to find. Fortunately, there are a few things that make this easier:

- Keep an eye on other ships in the system. Especially trader ships have a tendency to travel to other systems. If you see one heading to a part of space that seems empty to you, follow them! They will likely lead you to a jump point.

- When you land on a planet, don't forget to chat up the people in the bar. They can be quite friendly, and some of them may even help you out by telling you the location of a jump point in the system.

- Keep a look out for Star Maps. These maps are sold on planets, and each one reveals a specific part of the galaxy. Most maps reveal several systems at once, and will usually reveal some or all of their jump points as well.

- Not all jump points are hard to find! Some of them have buoys placed next to them that transmit their coordinates. You will be able to see these jump points from any distance away. Jumps like these are usually arranged in a long chain of jumps, and these chains are called Highways. Highways are found in large faction territories.]])

message8 = _([[You now know about discovering planets and jump points, and you also know that some systems can make that process more difficult. As a final tip, rumor has it that some jump points can not be discovered at all unless you have special sensors on board and know exactly where to look. It even seems some jump points can not be seen or used at all, and only serve as exit points...

Congratulations! This concludes the exploration and discovery tutorial.]])

navomsg = _("Navigate to the marker on the map")

function create()
   -- Set up the player here.
   player.teleport("Sioux")
   player.msgClear()
   
   pilot.clear()
   pilot.toggleSpawn(false) -- To prevent NPCs from getting targeted for now.
   player.pilot():setPos(vec2.new(-15000, -5000))
   
   player.pilot():setNoLand()
   player.pilot():setNoJump()

   tein = planet.get("Tein")
   rein = planet.get("Rein")
   jmp = jump.get(system.cur(), "Iroquois")
   
   tk.msg(title1, message1)
   tk.msg(title1, message2:format(tutGetKey("overlay")))

   inhook = hook.input("input")
end

function input(inputname, inputpress)
   if inputname == "overlay" then
      tk.msg(title1, message3)
      marker = system.mrkAdd( _("Fly here"), tein:pos())
      omsg = player.omsgAdd(navomsg, 0)
      hook.discover("discover")
      hook.rm(inhook)
   end
end

function discover(disctype, discovered)
   -- TODO: test for the actual discovered object.
   if disctype == "asset" and discovered == tein then
      tk.msg(title1, message4)
   elseif disctype == "asset" and discovered == rein then
      tk.msg(title1, message5)
      system.mrkRm(marker)
      marker = system.mrkAdd( _("Fly here"), jmp:pos())
   elseif disctype == "jump" then
      tk.msg(title1, message6)
      tk.msg(title1, message7)
      tk.msg(title1, message8)
      system.mrkRm(marker)
      cleanup()
   end
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    if not (omsg == nil) then player.omsgRm(omsg) end
    naev.keyEnableAll()
    naev.eventStart("Tutorial")
    evt.finish(true)
end
