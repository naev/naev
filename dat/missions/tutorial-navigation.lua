--[[

   This is one of NAEV's tutorial modules.

   This module introduces the map, as well as inter-system travel.

]]--

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

   title = {}
   text = {}
   title[1] = "NAEV Tutorial - Navigation"
   text[1] = [[Would you like to learn about the map and navigation?]]
   text[2] = [[In this part of the tutorial we'll deal with long-distance navigation. All ships in NAEV are equipped with a hyperspace drive and universe map.
   
Important keys to remember:
 %s opens the system map.
 %s cycles through hyperspace targets.
 %s attempts to enter hyperspace or aborts an attempt.
 %s activates autopilot.]]
   title[2] = "System Map"
   text[3] = [[We'll first talk about the map. When you open your map you'll notice it's barren. As you explore or buy star maps, the visible area will expand.
   
Each circle represents a system, and the interconnecting lines represent hyperspace routes. You can click on a system to select it as a hyperspace target. If it's far away, the autonav system will make a route to it which your autopilot can use. The colour of each jump indicates whether or not you have enough fuel to make it.]]
   title[3] = "Nav System"
   text[4] = [[Once you have a target, you'll see it in your navigation system  on the HUD. If you are far enough to jump it will be green. It's impossible to jump near large gravity wells, such as planets and stations. If you're too close to a gravity well, the hyperspace indicator will be grey. Once you're far enough from any gravity wells, you can jump to another system.]]
   text[5] = [[Now we'll try to jump. Here's an overview of how it works:
 Select a target with the map, or cycle through targets with %s.
 Move away from gravity wells until navigation turns green, or use autopilot with %s.
 Use %s to initialize the jump.
   
Try doing this now. Since you haven't explored any systems, only one other system is visible on the map.]]
   title[4] = "Tutorial Finished"
   text[6] = [[This concludes the tutorial. You should now know how to fly, target, fight, land, and hyperspace between systems.

You can start earning credits by accepting cargo missions, available at mission computers, which will also help you explore the universe.

Enjoy the game!]]

   -- Mission details
   misn_title = "NAEV Tutorial - Navigation"
   misn_reward = "Navigation knowledge."
   misn_desc = "Overview of the map and hyperspace navigation."
   -- Aborted mission
   msg_abortTitle = "Tutorial Aborted"
   msg_abort = [[Seems you're ready to go on your own. Enjoy the game!]]
   -- Rejected mission
   msg_rejectTitle = "NAEV Tutorial - Navigation"
   msg_reject = [[Are you sure? If you reject this portion of the tutorial, you will be unable to complete the other portions.
   
Press yes to abort the tutorial, or no to continue it.]]
   -- OSD stuff
   osd_title = {}
   osd_msg   = {}
   osd_title[1] = "Tutorial - Navigation"
   osd_msg[1]   = {
      "Open the map with " .. naev.getKey("starmap") .. " and select a target.",
      "Move away from the planet.",
      "Use " .. naev.getKey("jump") .. " to initialize the jump."
   }
end

function create()
   if forced == 1 then
      start()
   else
      if tk.yesno(title[1], text[1]) then
         start()
      else
         reject()
      end
   end
end

function start()
   misn.accept()

   -- Set basic mission information.
   misn.setTitle(misn_title)
   misn.setReward(misn_reward)
   misn.setDesc(misn_desc)

   -- Create OSD
   misn.osdCreate(osd_title[1], osd_msg[1])

   -- Set Hooks
   tutTakeoff()
   hook.enter("tutEnter")
end

function tutTakeoff()
   misn_sys = system.cur()
   tk.msg(title[1], string.format(text[2], naev.getKey("starmap"), naev.getKey("thyperspace"), naev.getKey("jump"), naev.getKey("autonav")))
   tk.msg(title[2], text[3])
   tk.msg(title[3], text[4])
   tk.msg(title[3], string.format(text[5], naev.getKey("thyperspace"), naev.getKey("autonav"), naev.getKey("jump")))
end


function tutEnter()
   enter_sys = system.cur()
   if enter_sys ~= misn_sys then
      hook.timer(5000, "tutEnd")
   end
end
   
function tutEnd()
	tk.msg(title[4], text[6])
	misn.finish(true)
end

function abort()
   tk.msg(msg_abortTitle, msg_abort)
   var.push("tutorial_aborted", true)
   misn.finish(false)
end

function reject()
   if tk.yesno(msg_rejectTitle, msg_reject) then
      abort()
   else
      start()
   end
end
