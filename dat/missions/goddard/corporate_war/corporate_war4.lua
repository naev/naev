--[[

Corporate War mission 4.
The player is asked to test out the prototype in the Ingot system. Once there, and after some maneouvers, the enemy company jumps in to try to take out the prototype. Player must defeat enemies.
start planet = Zhiru in the Goddard system or Krain Station in the Salvador system.
pickup planet/test sys = Ulios/Ingot
end planet = same as start.

This mission tries to claim the system "testSys", currently set to the system Ingot.




psuedo giving a ship/taking a ship from the player. do when landed.

playerShipModel = pilot.player(pilot.ship()) --gets model of current ship to swap back to later.
playerShipName = player.ship() --gets the ship name.
playerShipOutfits = pilot.outfits(pilot.player()) --gets a table with all outfits
player.swapShip(friendlyFaction .. " Prototype","Prototype",nil,true,false)  --swaps the players ship with the prototype.
pilot.rmOutfit(pilot.player(),"all")
pilot.rmOutfit(pilot.player(),"cores") --remove all outfits
pilot.addOutfit(pilot.player(),outfitX) --add the outfits we want.
-------
player.swapShip(playerShipModel,playerShipName,nil, true, true) --swaps the prototype with the old players ship, removing the prototype
pilot.rmOutfit(pilot.player(),"all")
pilot.rmOutfit(pilot.player(),"all")
for _,o in ipairs(playerShipOutfits) do
   pilot.addOutfit(pilot.player(),o)
end

--FOR CPW3 ------ player.unboard()

--]]


include "proximity.lua"


-- Bar information
bar_name = "%s"
bar_desc = "%s is happily sipping a drink."

misn_title = "Flying a New Bird"
misn_desc = "Test out the new prototype ship in a remote system."

bmsg = {} --beginning messages
emsg = {} --ending messages
fmsg = {} --failure messages
ifd = {} --in flight dialogue

bmsg[1] = [[You find %s once again in the bar. He smiles as he sees you approach. "%s! Good to see you. Good job boarding that ship and helping us get the edge over our competetion." %s grins. "We have a prototype ship made up, thanks to you. We were wondering if you'd like to take it on it's maiden voyage and run some tests for us?]]
bmsg[2] = [[%s smiles happily. "Great! You can meet up with our people on %s in the %s system. They will transfer control over to you and probably be following you around in a ship of their own, just to observe. We got a nice little payment of %d waiting for you as well." %s grins as he gets up. He says, "Thank, %s." as he turns to leave.]]
bmsg[3] = [[You land on %s, only to be greeted by a dock bustling with engineers and other people wearing the %s logo. Several engineers approach you as you leave your ship. "I assume you are %s," the lead engineer says. "I'm Giryn, lead engineer on the prototype project. We are going to let you fly the prototype ship. It is imperative that you follow my instructions and DO NOT LEAVE THE SYSTEM. We want to keep this project under wraps. I'll be flying beside you in another ship." He leads you to another part of the dock, partitioned off from the rest. Inside, you see the new prototype and your mouth starts watering. "This is the new bird," Giryn says. "Go ahead and climb in, and get yourself familiarized with the controls. You can takeoff whenever you are ready." And with that, the engineers turn and go, leaving you with the new ship.]]

ifd[1] = [[Alright, just follow our commands and we should be done quick.]]
ifd[2] = [[First thing, We need to check engines. Fly to point AF-27. We've highlighted it on your map.]]
ifd[3] = [[Ok, good. Now, fly to point GE-83, then point TB-54. We've highlighted these on your map.]]
ifd[4] = [[We need to monitor turret tracking. We've highlighted a drone on your map. Please attack it.]]
ifd[5] = [[We need to check hull integrity. Please fly to point QH-09, and allow yourself to be attacked.]]
ifd[6] = [[Next, we need to check heat diffusion. There are 2 groups of inactive drones at points MU-89.]]
ifd[7] = [[Please destroy both groups.]]
ifd[8] = [[Those aren't drones! Fireteams Bravo and Gamma, move in now!]]
ifd[9] = [[%s! Defensive maneouvers! Take out as many as you can, but don't risk the ship!]]

fmsg[1] = [[%s doesn't look happy as you turn him down. "Well, if you change your mind, I'll be around here somewhere." %s gets up and wanders off through the bar.]]--player refuses mission.
fmsg[2] = [[The comm on the bridge of the prototype ship suddenly displays a full hologram of %s. "I see our trust in you was misplaced. I'll assume you intend to keep that ship. You are now an enemy of %s, and we will not hesitate to shoot you on sight. Oh, and we are keeping your ship." The comm goes dark.]]
fmsg[3] = [[You land prematurely on %s, shortly followed by Giryn's ship. You barely have time to dock before he's yelling at you about "following orders" and how you are "an absolute amateur". He storms off, shouting over his shoulder that you should "go talk to %s about this" and that he "doesn't get paid enough for this frack". You try to reboard the prototype, but are locked out, so you head back to where you docked your ship earlier.]]

emsg[1] = [[You land on %s, after guiding the new prototype ship somewhat gracefully through the atmosphere. Giryn approaches you, with smalls on his face. "Well, that was exhilirating. Thank you for your help. We've credited payment to your account, and I've received word that %s would like to speak to you back on %s, whenever you get a chance." With a wave, Giryn walks away.]]

osd = {}
osd[1] = "Land on %s in the %s system."
osd[2] = "Follow instructions given to test the new ship."
osd[3] = "Land on %s and transfer back to your ship."
osd2temp = ""

function create ()
   
   --This mission tries to claim "combatSys", i.e. Mason.
   friendlyFaction = faction.get(var.peek("corpWarFaction"))
   enemyFaction = faction.get(var.peek("corpWarEnemy"))

   startAsset,startSys = planet.cur()
   testAsset,testSys = planet.get("Ulios")

   if friendlyFaction == faction.get("Goddard") then
      handlerName = "Eryk"
   else
      handlerName = "Thregyn"
   end

   if not misn.clam(targetSys) then
      misn.finish(false)
   end

   bar_name = bar_name:format(handlerName)
   bar_desc = bar_desc:format(handlerName)

   bmsg[1] = bmsg[1]:format(handlerName,player.name(),handlerName)
   bmsg[2] = bmsg[2]:format(handlerName,testAsset:name(),testSys:name(),misn_reward,handlerName,player.name())
   bmsg[3] = bmsg[3]:format(testAsset:name(),friendlyFaction,player.name())
   fmsg[1] = fmsg[1]:format(handlerName,handlerName)
   fmsg[2] = fmsg[2]:format(handlerName,friendlyFaction)
   fmsg[3] = fmsg[3]:format(testAsset:name(),handlerName)
   emsg[1] = emsg[1]:format(testAsset:name(),handlerName,startAsset:name())
   osd[1] = osd[1]:format(testAsset:name(),testSys:name())
   osd[3] = osd[3]:format(testAsset:name())

   misn.setNPC( bar_name, "neutral/male1" )
   misn.setDesc( bar_desc )   
   misn_reward = 110000 + faction.playerStanding(friendlyFaction) * 3000 
end


function accept ()
   if not tk.yesno(misn_title,bmsg[1]) then
      tk.msg(misn_title,fmsg[1])
      misn.finish(false)
   end
   
   misn.accept() 
   tk.msg(misn_title,bmsg[2])
   misn.setTitle(misn_title)
   misn.setReward(misn_reward)
   misn.setDesc(misn_desc)
   missionMarker = misn.markerAdd(testSys)

   missionStatus = 1

   misn.osdCreate(misn_title,osd)
   misn.osdActive(missionStatus)
   
   hook.jumpin("jumper")
   hook.land("lander")
   hook.takeoff("startTest")
end

function lander()
   if missionStatus == 1 and planet.cur() == testAsset then
      tk.msg(misn_title,bmsg[3])
      missionStatus = 2
      --time to swap ships. hopefully i don't eff this up.
      playerShipModel = pilot.player():ship()
      playerShipName = player.ship()
      playerShipOutfits = pilot.player():outfits()

      player.swapShip(friendlyFaction .. " Prototype","Prototype",nil,true,false)  --swaps the players ship with the prototype.
      pilot.rmOutfit(pilot.player(),"all")
      pilot.rmOutfit(pilot.player(),"cores") --remove all outfits
      --need to figure out outfits.
      if friendlyFaction == "Goddard" then
         outfits = { --goddard mkii
            outfit.get("Turbolaser"),
            outfit.get("Turbolaser"),
            outfit.get("Railgun Turret"),
            outfit.get("Railgun Turret"),
            outfit.get("Heavy Laser"),
            outfit.get("Heavy Laser"),
            outfit.get("Heavy Laser"),
            outfit.get("Milspec Hermes 9802 Core System"),
            outfit.get("Milspec Scrambler"),
            outfit.get("Large Shield Booster"),
            outfit.get("Reactor Class III"),
            outfit.get("Reactor Class III"),
            outfit.get("Emergency Shield Booster"),
            outfit.get("Tricon Typhoon II Engine"),
            outfit.get("S&K Superheavy Combat Plating"),
            outfit.get("Battery III"),
            outfit.get("Battery III"),
            outfit.get("Shield Capacitor IV"),
            outfit.get("Shield Capacitor IV"),
            outfit.get("Targeting Array"),
            outfit.get("Improved Regrigeration Cycle"),
            outfit.get("Improved Power Regulator"),
         }
      else
         outfits = { --kestrel mkii
            outfit.get("Turbolaser")
            outfit.get("Turbolaser")
            outfit.get("Turbolaser")
            outfit.get("Railgun Turret")
            outfit.get("Railgun Turret")
            outfit.get("Laser Turret MK3")
            outfit.get("Laser Turret MK3")
            outfit.get("Milspec Hermes 9802 Core System")
            outfit.get("Reactor Class III")
            outfit.get("Large Shield Booster")
            outfit.get("Large Shield Booster")
            outfit.get("Emergency Shield Booster")
            outfit.get("Milspec Scrambler")
            outfit.get("Krain Remige Engine")
            outfit.get("Unicorp B-16 Heavy Plating")
            outfit.get("Battery III")
            outfit.get("Shield Capacitor IV")
            outfit.get("Targeting Array")
            outfit.get("Improved Power Regulator")
         }
      end
      for _,o in ipairs(outfits) do
         pilot.addOutfit(pilot.player(),o)
      end  
   end
end

function startTest()
   if player.ship() == "Prototype" then
      missionStatus = 3
      testControl = pilot.add("Gawain")
      testControl = testControl[1]
      testControl:control()
      testControl:follow(pilot.player())

      --set up drones, etc, used in test.
      drone = pilot.add("Drone Lancelot","dummy",vec2.new(-4000,3000))
      
      droneGroup1Coords = vec2.new(1000,-14000)
      droneGroup1 = {}
      droneGroup2Coords = vec2.new(-1000,-16000)
      droneGroup2 = {}
      numShips = 8
      angle = math.pi * 2 / numShips
      radius = 80 + numShips * 25
      for i = 1, 8 do
         
         x = radius * math.cos(angle * i)
         y = radius * math.sin(angle * i) 

         droneNew1 = pilot.add("Drone Lancelot","dummy",droneGroup1Coords)
         table.insert(droneGroup1, droneNew1[1])
         
         droneNew1[1]:control()
         droneNew1[1]:brake()
         droneNew1[1]:setPos(droneGroup1Coords + vec2.new(x,y))

         droneNew2 = pilot.add("Drone Lancelot","dummy",droneGroup2Coords)
         table.insert(droneGroup2,droneNew2[1])

         droneNew2[1]:control()
         droneNew2[1]:brake()
         droneNew2[1]:setPos(droneGroup2Coords + vec2.new(x,y))

      end

      --start the hooks.
      hook.date(time.create(0,0,100),"startTest")

--x=32250 y=18500
--x=-37000 y=-18500

--x=17000 y=18000
--x=-18200,y=-12000
--[[
ifd[1] = [[This is Gyrin. We need to check engines first. Fly to point AF-27. We've highlighted it on your map.]]
ifd[2] = [[Ok, good. Now, fly to point GE-83, then point TB-54. We've highlighted these on your map.]]
ifd[3] = [[We need to monitor offensive systems. We've highlighted a drone on your map. Please destroy it.]]
ifd[4] = [[We need to check defensive systems. Please fly to point QH-09, and allow yourself to be attacked.]]
ifd[5] = [[Next, we need to check heat diffusion. There are 2 groups of inactive drones at point MU-89.]]
ifd[6] = [[Please destroy both groups.]]
ifd[7] = [[Those aren't drones! Fireteams Bravo and Gamma, move in now!]]
ifd[8] = [[%s! Defensive maneouvers! Take out as many as you can, but don't risk the ship!]]
--]]
   end
end

function startTest() --fly to a point.
   testControl:broadcast(ifd[1])
   testHook = hook.date(time.create(0,0,50),"proximity",{location=vec2.new(14500,14500), radius=250,funcname="testTwo"})
end

function testTwo() --fly through another point on your way to a third.
   hook.rm(testHook)
   testControl:broadcast(ifd[2])
   testHook = hook.date(time.create(0,0,50),"proximity",{location=vec2.new(-10000,-12000),radius=250,funcname="testThree"})
end

function testThree() --arrive at the third point.
   hook.rm(testHook)
   testHook = hook.date(time.create(0,0,50),"proximity",{location=vec2.new(-8000,2000),radius=250,funcname="testFour"})
end

function testFour() --shoot a drone.
   --get the drone going
   drone[1]:control()
   drone[1]:hilight()
   drone[1]:setSpeedLimit(50)
   drone[1]:goto(0,3000)
   --hook the next phase to the player killing the drone.

end

function jumper()

end

function abort ()
   misn.finish(false)
end
