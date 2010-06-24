--[[

   This is one of NAEV's tutorial modules.
   This module is responsible for bringing 0.3.9 (and earlier) player up to speed with current developments.   
   As gameplay stabilizes, it will eventually be phased out.

]]--

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

-- Initialize variables
   stagesDone = 0
   earlyTakeoff = 0

   title = {}
   text = {}
   title[1] = "NAEV Tutorial - Landing Tabs"
   text[1] = [[It appears you've played before. There are many new changes in 0.4.0 and subsequent releases, including a complete revamp of the outfitting system, an on-screen display, map improvements, and many more.
   
Would you like to play a short tutorial familiarizing you with these new features?]]
   text[2] = [[Perhaps most noticeable is the change from buttons to tabs. Functionality has stayed roughly the same, but tabs provide a cleaner, compact interface.
   
There have been changes to the majority of tabs, so view each one for an explanation of the changes.]]
   title[2] = "NAEV Tutorial - Spaceport Bar"
   text[3] = [[The bar interface has been completely redone. Missions no longer pop up at you, instead NPCs will appear in the list on the left, whom you can approach.]]
   title[3] = "NAEV Tutorial - Mission Computer"
   text[4] = [[The mission computer has changed little. Given that there are over ten new systems, it's now quite likely to find long-distance, high-reward shipping missions.]]
   title[4] = "NAEV Tutorial - Outfitter"
   text[5] = [[The outfitter hasn't changed much, graphically. Given the equipment system revamp, the number in the top left of each outfit image is the number in your unequipped inventory pool. Outfit statistics are now displayed for you, to make decisions easier. Without an  outfitter, you can't change your ship's equipment.]]
   title[5] = "NAEV Tutorial - Shipyard"
   text[6] = [[The shipyard has been somewhat streamlined. Statistics are now displayed on the right instead of within the info subwindow, and the shipyard is now purely for buying ships, rather than selling as well.]]
   title[6] = "NAEV Tutorial - Equipment"
   text[7] = [[The most complex of the tabs, the equipment screen is entirely new for 0.4.0. It's the player's hub, allowing requipping, ship swapping, and a view of ship statistics as modified by outfits.
   
The outfits in the bottom left are your inventory pool. Selecting one and right clicking on a slot equips it there.]]
   text[8] = [[In the middle are the slots. They represent the number of outfits you can equip on a ship.

High slots are for weapons, medium slots are for 'active' outfits, generally those that substantially upgrade a statistic or use a large amount of resources. Low slots contain simplistic 'passive' outfits, which usually lack the drawbacks of medium-slot outfits.

For example, a laser cannon occupies a high slot, a reactor occupies a medium slot because it both supplies power and uses substantial CPU, and Plasteel plating is a low slot outfit, as it upgrades armour with no drawbacks besides a mass increase.]]
   text[9] = [[CPU power is the new primary limiter of what can be equipped on a ship. The Vendetta, a low-tech vessel, has an abundance of weapon slots, yet proportionally low CPU power. This restricts what can be equipped on one, as to use all weapon slots, each weapon must use a low amount of CPU. On the other hand, a high-tech cruiser such as the Kestrel may equip nearly any weapon loadout it wants, given its large amount of CPU power.

The rotating image of your ship in the upper right illustrates how quickly your ship will turn in space, given the current equipment.]]
   title[7] = "NAEV Tutorial - Commodity Exchange"
   text[10] = [[The commodity exchange hasn't changed.
   
Keep in mind that as the economy isn't very dynamic, there's far more profit to be made doing cargo missions.]]
   text[11] = [[Well, this was the last stop. You should now be familiar with all the new landing window changes.
   
Other changes include the addition of a new faction, several  new ships, additional missions and major changes to the map.

On the map, known systems now have a central circle depicting its friendliness (or hostility) to you. The outer circle represents security, which is essentially the number of friendly ships versus the number of enemies. The outer glow represents the faction that owns the system.]]
   text[12] = [[Easy there, space cowboy. Might want to land and finish viewing the new tabs.]]
   text[13] = [[Head to Zhiru in the Goddard system to proceed with this tutorial.]]
   -- Mission details
   misn_title = "NAEV Tutorial - New Features"
   misn_reward = "Familiarization with new features."
   misn_desc = "Overview of the new features"
   -- Aborted mission
   msg_abortTitle = "Tutorial Aborted"
   msg_abort = [[Very well. If you wish to familiarize yourself more completely, replay the tutorial.]]
   -- Rejected mission
   msg_rejectTitle = "NAEV Tutorial - Landing Tabs"
   msg_reject = [[Are you sure? This is the only opportunity to brush up on the new features without replaying with tutorial.
   
Press yes to abort the tutorial, or no to continue it.]]

   viewedTabs = {}
   viewedTabs[1] = "\nSpaceport Bar"
   viewedTabs[2] = "\nMission Computer"
   viewedTabs[3] = "\nOutfitter"
   viewedTabs[4] = "\nShipyard"
   viewedTabs[5] = "\nEquipment Screen"
   viewedTabs[6] = "\nCommodity Exchange"

   -- OSD stuff
   osd_title = {}
   osd_msg   = {}
   osd_title[1] = "Tutorial - New Features"
   osd_msg[1] = "Head to Zhiru in the Goddard system.\nVisit the: %s %s %s %s %s %s"
   
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
   var.push("version", 042)
   -- Create OSD
   osdCreation()
   sysname = "Goddard"
   sys = system.get(sysname)
   misn.markerAdd( sys, "low" )
   
   tk.msg(title[1], text[13])
   
   -- tutLand()
   
   -- Set basic mission information.
   misn.setTitle(misn_title)
   misn.setReward(misn_reward)
   misn.setDesc(misn_desc)

hook.land("tutLand0")

end

function osdCreation()
   misn.osdCreate(osd_title[1], { string.format(osd_msg[1],viewedTabs[1], viewedTabs[2], viewedTabs[3], viewedTabs[4], viewedTabs[5], viewedTabs[6]) } )
end

function tutLand0()
   if planet.get() == planet.get( "Zhiru" ) then
      hook.land("tutLand")
      hook.land("tutBar", "bar")
      hook.land("tutMission", "mission")
      hook.land("tutOutfits", "outfits")
      hook.land("tutShipyard", "shipyard")
      hook.land("tutEquipment", "equipment")
      hook.land("tutCommodity", "commodity")
      hook.takeoff("tutEarly")
   else
      return
   end
end

function tutLand()
   if viewedLand == 1 then
      return
   else
      viewedLand = 1
      tk.msg (title[1], text[2])
   end
end

function tutBar()
   if viewedBar == 1 then
      return
   else
      viewedBar = 1
      viewedTabs[1] = ""
      osdCreation()
      stagesDone = stagesDone + 1
      tk.msg (title[2], text[3])
      if stagesDone == 6 then
         tutEnd()
      end
   end
end

function tutMission()
   if viewedMission == 1 then
      return
   else
      viewedMission = 1
      viewedTabs[2] = ""
      osdCreation()
      stagesDone = stagesDone + 1
      tk.msg (title[3], text[4])
      if stagesDone == 6 then
         tutEnd()
      end
   end
end

function tutOutfits()
   if viewedOutfits == 1 then
      return
   else
      viewedOutfits = 1
      viewedTabs[3] = ""
      osdCreation()
      stagesDone = stagesDone + 1
      tk.msg (title[4], text[5])
      if stagesDone == 6 then
         tutEnd()
      end
   end
end

function tutShipyard()
   if viewedShipyard == 1 then
      return
   else
      viewedShipyard = 1
      viewedTabs[4] = ""
      osdCreation()
      stagesDone = stagesDone + 1
      tk.msg (title[5], text[6])
      if stagesDone == 6 then
         tutEnd()
      end
   end
end

function tutEquipment()
   if viewedEquipment == 1 then
      return
   else
      viewedEquipment = 1
      viewedTabs[5] = ""
      osdCreation()
      stagesDone = stagesDone + 1
      tk.msg (title[6], text[7])
      tk.msg (title[6], text[8])
      tk.msg (title[6], text[9])
      if stagesDone == 6 then
         tutEnd()
      end
   end
end

function tutCommodity()
   if viewedCommodity == 1 then
      return
   else
      viewedCommodity = 1
      viewedTabs[6] = ""
      osdCreation()
      stagesDone = stagesDone + 1
      tk.msg (title[7], text[10])
      if stagesDone == 6 then
         tutEnd()
      end
   end
end

function tutEarly()
   if earlyTakeoff == 0 then
      earlyTakeoff = 1
      tk.msg (title[1], text[12])
   else
      return
   end
end

function tutEnd()
	tk.msg(title[1], text[11])
	misn.finish(true)
end

function abort()
   tk.msg(msg_abortTitle, msg_abort)
   var.push("tutorial_aborted", true)
   misn.finish(false)
   var.push("version", 042)
end

function reject()
   if tk.yesno(msg_rejectTitle, msg_reject) then
      abort()
      var.push("version", 042)
   else
      start()
   end
end
