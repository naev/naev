--[[

   This is one of NAEV's tutorial modules.

   This module is for the landing window and associated tabs.

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
   text[1] = [[Would you like to learn about the landing tabs?]]
   text[2] = [[This is the landing window. Here, you can see an image of where you are, along with a description. Depending on the location, you'll have various services available, including the spaceport bar, mission computer, outfit retailer, shipyard, equipment screen and commodity exchange. If you wish to learn about them, you can visit them by clicking on their respective tabs on the  bottom of the landing window.]]
   title[2] = "NAEV Tutorial - Spaceport Bar"
   text[3] = [[The spaceport bar is a hub of activity in NAEV. The vast majority of storyline missions begin at the bar, so it pays to stop by.

In future versions, the bar will also feature dynamic news reflecting activities within the universe.]]
   title[3] = "NAEV Tutorial - Mission Computer"
   text[4] = [[The mission computer contains NAEV's automatically-generated missions. While not nearly as engaging as storyline missions, they are a great way to make money as a new player.

It's advisable to do rush delivery missions in smaller ships, as larger ships hyperspace too slowly to make the deliveries on time.]]
   title[4] = "NAEV Tutorial - Outfitter"
   text[5] = [[The outfitter is where you buy and sell outfits for your ship. Each location has its own technology level, so the outfit selection is best in major systems.

There are also a handful of outfits with special technology levels, which can only be found at certain spaceports.]]

   title[5] = "NAEV Tutorial - Shipyard"
   text[6] = [[The shipyard is where ships are bought. Much like outfits, each vessel has its own technology level, and therefore larger locations will usually have a better selection than border colonies.

There are a number of special ships, such as the Goddard and the Skull & Bones pirate vessels which can only be found on their faction homeworlds.]]
   title[6] = "NAEV Tutorial - Equipment"
   text[7] = [[The equipment screen is certainly the most complex among the landing tabs. The upper left pane displays your current ship, and if the current spaceport has a shipyard, you can also view, re-equip, sell, and even transport your other ships to the current spaceport, for a price.

The lower left pane displays outfits that you own, provided the spaceport has an outfitter. Outfits show up here when bought, and the number in the inventory decreases as you equip them to ships.]]
   text[8] = [[In the middle are the slots. They represent the number of outfits you can equip on a ship.

High slots are for weapons, medium slots are for 'active' outfits, generally those that substantially upgrade a statistic or use a large amount of resources. Low slots contain simplistic 'passive' outfits, which usually lack the drawbacks of medium-slot outfits.

For example, a laser cannon occupies a high slot, a reactor occupies a medium slot because it both supplies power and uses substantial CPU, and Plasteel plating is a low slot outfit, as it upgrades armour with no drawbacks besides a mass increase.]]
   text[9] = [[CPU power is the primary limiter of what can be equipped on a ship in NAEV. The Vendetta, a low-tech vessel, has an abundance of weapon slots, yet proportionally low CPU power. This restricts what can be equipped on one, as to use all weapon slots, each weapon must use a low amount of CPU. On the other hand, a high-tech cruiser such as the Kestrel may equip nearly any weapon loadout it wants, given its large amount of CPU power.

The rotating image of your ship in the upper right illustrates how quickly your ship will turn in space, given the current equipment.]]
   title[7] = "NAEV Tutorial - Commodity Exchange"
   text[10] = [[The commodity exchange is fairly simple in purpose and execution. Each spaceport buys and sells certain commodities, and a profit can be made in the traditional manner, by buying commodities where they're cheap, and selling where they're in demand at a higher price.
   
Unfortunately the economy is currently not terribly dynamic, so there's much more profit to be made taking regular trade missions.]]
   text[11] = [[Well, this was the last stop. When you're done here, take off and the tutorial will continue.]]
   text[12] = [[Easy there, space cowboy. You're getting a bit ahead, might want to land and finish learning about the landing tabs.]]
   
   text[13] = [[If I've told you once, I've told you a thousand times! Get back on the planet and learn, learn, learn!]]
   
   -- Mission details
   misn_title = "NAEV Tutorial - Landing Tabs"
   misn_reward = "Knowledge of the landing window."
   misn_desc = "Overview of the landing tabs."

   -- OSD stuff
   osd_title = {}
   osd_msg   = {}
   osd_title[1] = "Tutorial - Landing"
   osd_msg[1]   = {
      "Explore the landing window."
   }
end

      
function create()
   if tk.yesno(title[1], text[1]) then
      misn.accept()
      tutLand()
      
      -- Set basic mission information.
      misn.setTitle(misn_title)
      misn.setReward(misn_reward)
      misn.setDesc(misn_desc)

      -- Create OSD
      misn.osdCreate(osd_title[1], osd_msg[1])

      -- Set Hooks
      hook.land("tutBar", "bar")
      hook.land("tutMission", "mission")
      hook.land("tutOutfits", "outfits")
      hook.land("tutShipyard", "shipyard")
      hook.land("tutEquipment", "equipment")
      hook.land("tutCommodity", "commodity")
      hook.takeoff("tutEarly")
   else
      var.push("tutorial_aborted", true)
      misn.finish(false)
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
   elseif earlyTakeoff == 1 then
      tk.msg (title[1], text[13])
      earlyTakeoff = 2
   else
      return
   end
end

function tutEnd()
	tk.msg(title[1], text[11])
   var.push("tutorial_done", 2)
	misn.finish(true)
end

function abort()
   var.push("Tutorial Aborted", "It seems you're ahead of the curve, so I'll cut you loose.")
   var.push("tutorial_aborted", true)
   misn.finish(false)
end
