--[[

Corporate War mission 4.
The player is asked to test out the prototype in the Ingot system. Once there, and after some maneouvers, the enemy company jumps in to try to take out the prototype. Player must defeat enemies.
start planet = Zhiru in the Goddard system or Krain Station in the Salvador system.
pickup planet/test sys = Ulios/Ingot
end planet = same as start.

This mission tries to claim the system "testSys", currently set to the system Ingot.


TODO:
+test
+the prototype ships are going to be changing classes, so when they do, the outfitting will need to be reworked.
++leaving it as-is for now, once the mission works i'll go back and test.
+pray

--]]


include "proximity.lua"


-- Bar information
bar_name = "%s"
bar_desc = "%s is happily sipping a drink."

-- mission information
misn_title = "Flying a New Bird"
misn_desc = "Test out the new prototype ship in a remote system."

bmsg = {} --beginning messages
emsg = {} --ending messages
fmsg = {} --failure messages
ifd = {} --in flight dialogue

bmsg[1] = [[You find %s once again in the bar. He smiles as he sees you approach. "%s! Good to see you. Good job boarding that ship and helping us get the edge over our competetion." %s grins. "We have a prototype ship made up, thanks to you. We were wondering if you'd like to take it on it's maiden voyage and run some tests for us?]]
bmsg[2] = [[%s smiles happily. "Great! You can meet up with our people on %s in the %s system. They will transfer control over to you and probably be following you around in a ship of their own, just to observe. We got a nice little payment of %d waiting for you as well." %s grins as he gets up. He says, "Thank, %s." as he turns to leave.]]
bmsg[3] = [[You land on %s, only to be greeted by a dock bustling with engineers and other people wearing the %s logo. Several engineers approach you as you leave your ship. "I assume you are %s," the lead engineer says. "I'm Giryn, lead engineer on the prototype project. We are going to let you fly the prototype ship. It is imperative that you follow my instructions and DO NOT LEAVE THE SYSTEM. We want to keep this project under wraps. I'll be flying beside you in another ship." He leads you to another part of the dock, partitioned off from the rest. Inside, you see the new prototype and your mouth starts watering. "This is the new bird," Giryn says. "Go ahead and climb in, and get yourself familiarized with the controls. You can takeoff whenever you are ready." And with that, the engineers turn and go, leaving you with the new ship.]]

ifd[1] = [[This is Gyrin. We need to check engines first. Fly to point AF-27. We've highlighted it on your map.]]
ifd[2] = [[Ok, good. Now, fly to point GE-83, then point TB-54. We've highlighted these on your map.]]
ifd[3] = [[We need to monitor offensive systems. We've highlighted a drone on your map. Please destroy it.]]
ifd[4] = [[We need to check defensive systems. Please hold your position, and allow yourself to be attacked. Do not attack back!]]
ifd[5] = [[Next, we need to check heat diffusion. Destroy the 2 groups of inactive drones at point MU-89.]]
ifd[6] = [[Please destroy both groups.]]
ifd[7] = [[Those aren't drones! Fireteams Bravo and Gamma, move in now!]]
ifd[8] = [[%s! Defensive maneouvers! Take out as many as you can, but don't risk the ship!]]
ifd[9] = [[Do not attack that ship! Please meet me on %s. The tests need to be restarted. They don't pay me enough.]]
ifd[10] = [[Alright, good job. Meet me back on the planet surface.]]

fmsg[1] = [[%s doesn't look happy as you turn him down. "Well, if you change your mind, I'll be around here somewhere." %s gets up and wanders off through the bar.]]
fmsg[2] = [[The comm on the bridge of the prototype ship suddenly displays a full hologram of %s. "I see our trust in you was misplaced. I'll assume you intend to keep that ship. You are now an enemy of %s, and we will not hesitate to shoot you on sight. Oh, and we are keeping your ship." The comm goes dark.]]
fmsg[3] = [[You land prematurely on %s, shortly followed by Giryn's ship. You barely have time to dock before he's yelling at you about "following orders" and how you are "an absolute amateur". He storms off, shouting over his shoulder that you should "go talk to %s about this" and that he "doesn't get paid enough for this frack". You try to reboard the prototype, but are locked out, so you head back to where you docked your ship earlier.]]
fmsg[4] = [[A very angry Giryn comes storming over to you. "You should not damage company property like that! How dare you. You can go back to your ship. Go talk to %s and see if he'll give you another chance." And with that, Giryn storms away.]]


emsg[1] = [[You land on %s, after guiding the new prototype ship somewhat gracefully through the atmosphere. Giryn approaches you, with smalls on his face. "Well, that was exhilirating. Thank you for your help. We've credited payment to your account, and I've received word that %s would like to speak to you back on %s, whenever you get a chance." With a wave, Giryn walks away.]]

--osd messages.
osd = {}
osd[1] = "Land on %s in the %s system."
osd[2] = "Take off in the prototype to test the new ship."
osd[3] = "Land on %s and transfer back to your ship."
--osd[2] will be updated during the mission. osdmini{} holds the updates.
osdmini = {}
osdmini[0] = "Take off in the prototype to test the new ship."
osdmini[1] = "Fly to point AF-27, highlighted on the map."
osdmini[2] = "Fly to point GE-83, then TB-54, highlighted on the map."
osdmini[3] = "Destroy the drone highlighted on the map."
osdmini[4] = "Allow the enemy craft to attack you. Do not fire back."
osdmini[5] = "Destroy groups of drones at MU-89, highlighted on the map."
osdmini[6] = "Destroy attacking drones."

function create()
   
   --This mission tries to claim "testSys", i.e. Ingot.
   startAsset,startSys = planet.cur()
   testAsset,testSys = planet.get("Ulios")

   if not misn.claim(testSys) then
      misn.finish(false)
   end

   --pull our faction variables.
   friendlyFaction = faction.get(var.peek("corpWarFaction"))
   enemyFaction = faction.get(var.peek("corpWarEnemy"))
   
   --set the handler name, and then setup the bar stuff.
   if friendlyFaction == faction.get("Goddard") then
      handlerName = "Eryk"
   else
      handlerName = "Thregyn"
   end

   --some pre-formating.
   bar_name = bar_name:format(handlerName)
   bar_desc = bar_desc:format(handlerName)

   --set up bar details.
   misn.setNPC( bar_name, "neutral/male1" )
   misn.setDesc( bar_desc )   
   misn_reward = 110000 + faction.playerStanding(friendlyFaction) * 3000 

   --format all the strings.
   bmsg[1] = bmsg[1]:format(handlerName,player.name(),handlerName)
   bmsg[2] = bmsg[2]:format(handlerName,testAsset:name(),testSys:name(),misn_reward,handlerName,player.name())
   fmsg[1] = fmsg[1]:format(handlerName,handlerName)
end


function accept ()
   if not tk.yesno(misn_title,bmsg[1]) then
      tk.msg(misn_title,fmsg[1])
      misn.finish(false)
   end
   
   --mission accepted, set up the stuffs.
   misn.accept() 
   tk.msg(misn_title,bmsg[2])
   misn.setTitle(misn_title)
   misn.setReward(misn_reward)
   misn.setDesc(misn_desc)
   missionMarker = misn.markerAdd(testSys, "high")

   missionStatus = 1
   empireStanding = faction.get("Empire"):playerStanding() --we don't want empire rep to suffer because of this campaign.
   
   --set up osd
   osdUpdate(1)

   --create our standard hooks.
   hook.jumpin("jumper")
   hook.land("lander")
end

function lander()
   
   --the player has reached the testAsset.
   --This is a big one, and may cause bugs. We will be saving the players ship to some variables,
   --then swapping it out for the prototype ship. /crossing fingers/
   if missionStatus == 1 and planet.cur() == testAsset then
      --standard mission stuff.
      bmsg[3] = bmsg[3]:format(testAsset:name(),friendlyFaction:name(),player.name())
      tk.msg(misn_title,bmsg[3])
      missionStatus = 2

      --time to swap ships. hopefully i don't eff this up.
      playerShipModel = pilot.player():ship():name()
      playerShipName = player.ship()
      playerShipOutfits = pilot.player():outfits()

      if friendlyFaction:name() == "Goddard" then
         protoShip = "Goddard MkII"
      else
         protoShip = "Kestrel MkII"
      end
      player.swapShip(protoShip,"Prototype",nil,true,true)  --swaps the players ship with the prototype.
      pilot.rmOutfit(pilot.player(),"all") --remove all the outfits
      pilot.rmOutfit(pilot.player(),"cores") --remove all the outfits
      --create some BIG outfit tables so we can add them all in using a for loop later.
      if friendlyFaction:name() == "Goddard" then
         outfits = { --goddard mkii outfit table of glory.
            outfit.get("Milspec Hermes 9802 Core System"),
            outfit.get("Turbolaser"),
            outfit.get("Turbolaser"),
            outfit.get("Railgun Turret"),
            outfit.get("Railgun Turret"),
            outfit.get("Heavy Laser"),
            outfit.get("Heavy Laser"),
            outfit.get("Heavy Laser"),
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
            outfit.get("Improved Refrigeration Cycle"),
            outfit.get("Improved Power Regulator"),
         }
      else
         outfits = { --kestrel mkii outfit table of glory.
            outfit.get("Milspec Hermes 9802 Core System"),
            outfit.get("Reactor Class III"),
            outfit.get("Turbolaser"),
            outfit.get("Turbolaser"),
            outfit.get("Turbolaser"),
            outfit.get("Railgun Turret"),
            outfit.get("Railgun Turret"),
            outfit.get("Laser Turret MK3"),
            outfit.get("Laser Turret MK3"),
            outfit.get("Large Shield Booster"),
            outfit.get("Large Shield Booster"),
            outfit.get("Emergency Shield Booster"),
            outfit.get("Milspec Scrambler"),
            outfit.get("Krain Remige Engine"),
            outfit.get("Unicorp B-16 Heavy Plating"),
            outfit.get("Battery III"),
            outfit.get("Shield Capacitor IV"),
            outfit.get("Targeting Array"),
            outfit.get("Improved Power Regulator"),
         }
      end

      --now actually add the outfits.
      for _,o in ipairs(outfits) do
         pilot.addOutfit(pilot.player(),o:name())
      end
   --hopefully that went well... moving on to setUpTest!
      player.takeoff()
      testHook = hook.timer(1000,"setUpTest")
   

   --if the player lands early during the test, then we fail the player.
   elseif missionStatus > 2 and missionStatus < 11 then
      fmsg[3] = fmsg[3]:format(testAsset:name(),handlerName)
      tk.msg(misn_title,fmsg[3])
      player.allowSave(true)
      misn.finish(false)
      removeShip()

   --if the player shoots the drone ship used to test the shields, this fails them when they land after they attack.
   elseif missionStatus == -1 then
      tk.msg(misn_title,fmsg[4])
      player.allowSave(true)
      misn.finish(false)
      removeShip()

   --yay we made it!
   elseif missionStatus == 11 then
      emsg[1] = emsg[1]:format(testAsset:name(),handlerName,startAsset:name())
      removeShip()
      player.allowSave(true)
      tk.msg(misn_title,emsg[1])
      player.pay(misn_reward)
      faction.modPlayerRaw("Empire",empireStanding - faction.get("Empire"):playerStanding())
      misn.markerRm(missionMarker)
      misn.finish(true)
   end
end

function setUpTest()
   --This is what is called after the player takes off from the planet. It'll only kick if the player takes off in the prototype ship.
   --Should that be osd'd? damn. probably.
   --only start if the player is in the mkII ship.
   missionStatus = 3
   osdUpdate(2,0)

   --testControl is Giryn. He will be guiding the player via broadcasts and generally swooping around.
   testControl = pilot.add(friendlyFaction:name() .. " Lancelot",nil,testAsset)
   testControl = testControl[1]
   testControl:control()
   testControl:follow(pilot.player())

   --set up drones, etc, used in test.
   --This drone gets attacked by the player.
   drone = pilot.add("Drone Lancelot",nil,vec2.new(-4000,3000))
   drone[1]:control()
   drone[1]:brake()
   
   --This drone is the one that does the shooting for the shield test
   if friendlyFaction:name() == "Krain" then
      drone2 = pilot.add("Krain Kestrel",nil,testAsset)
   else
      drone2 = pilot.add("Goddard Goddard",nil,testAsset)
   end
   drone2[1]:control()
   drone2[1]:goto(vec2.new(-3700,2700))
   drone2[1]:brake()
   --if the player attacks this ship, it fails the player, and the player has to restart the mission.
   hook.pilot(drone2[1],"attacked","notTheEnemy")
   --set up the two droneGroups for the "heat testing."
   --droneGroup1 is just dummy ships.
   droneGroup1Coords = vec2.new(1000,-14000)
   droneGroup1 = {}
   --droneGroup2 will be magically transformed into enemy ships after they get attacked. MWAHAHAHA.
   droneGroup2Coords = vec2.new(-1000,-16000)
   droneGroup2 = {}
   --may need to adjust numShips for balance reasons.
   numShips = 8
   --the groups will be in two circles, which is what angle and radius are used for.
   angle = math.pi * 2 / numShips
   radius = 80 + numShips * 25
   for i = 1, numShips do
      --calculate where in the circle the pilot should be.
      x = radius * math.cos(angle * i)
      y = radius * math.sin(angle * i) 

      --set up both groups concurrently.
      droneNew1 = pilot.add("Drone Lancelot",nil,droneGroup1Coords)
      droneNew1[1]:control()
      droneNew1[1]:brake()
      droneNew1[1]:setPos(droneGroup1Coords + vec2.new(x,y))
      droneNew1[1]:face(vec2.new(0,0))
      table.insert(droneGroup1, droneNew1[1])

      droneNew2 = pilot.add("Drone Lancelot",nil,droneGroup2Coords)
      droneNew2[1]:control()
      droneNew2[1]:brake()
      droneNew2[1]:setPos(droneGroup2Coords + vec2.new(x,y))
      droneNew2[1]:face(vec2.new(0,0))
      --whenever the player attacks a ship in group2,
      --all of group2 becomes hostile.
      hook.pilot(droneNew2[1],"attacked","startTheAttack")
      table.insert(droneGroup2,droneNew2[1])
   end

   --start the test for realz.
   testHook = hook.timer(250,"startTest")
end

function startTest() --test 1: fly to a point.
   if missionStatus == 3 then
      hook.rm(testHook)
      missionStatus = 4
      osdUpdate(2,1)
      testControl:broadcast(ifd[1])
      sysMrk = system.mrkAdd("AF-27",vec2.new(14500,14500))
      testHook = hook.date(time.create(0,0,50),"proximity",{location=vec2.new(14500,14500), radius=400,funcname="testTwo"})
   end
end

function testTwo() --fly through another point on your way to a third.
   if missionStatus == 4 then
      hook.rm(testHook)
      missionStatus = 5
      osdUpdate(2,2)
      testControl:broadcast(ifd[2])
      system.mrkRm(sysMrk)
      sysMrk = system.mrkAdd("GE-83",vec2.new(-10000,-12000))
      testHook = hook.date(time.create(0,0,50),"proximity",{location=vec2.new(-10000,-12000),radius=400,funcname="testThree"})
   end
end

function testThree() --arrive at the third point.
   if missionStatus == 5 then
      missionStatus = 6
      hook.rm(testHook)
      system.mrkRm(sysMrk)
      sysMrk = system.mrkAdd("TB-54",vec2.new(-8000,2000))
      testHook = hook.date(time.create(0,0,50),"proximity",{location=vec2.new(-8000,2000),radius=250,funcname="testFour"})
   end
end

function testFour() --shoot a drone.
   if missionStatus == 6 then
      missionStatus = 7
      osdUpdate(2,3)
      testControl:broadcast(ifd[3])
      hook.rm(testHook)
      system.mrkRm(sysMrk)

      --get the drone we created earlier going.
      drone[1]:setHilight()
      drone[1]:setVisplayer()
      drone[1]:setSpeedLimit(50)
      drone[1]:goto(vec2.new(0,3000))

      --hook the next phase to the player killing the drone.
      hook.pilot(drone[1],"exploded","testFive")
   end
end

function testFive() --getting shot! woo. this is split into two functions.
   --this function only makes drone2 attack the player.
   if missionStatus == 7 then
      missionStatus = 8
      osdUpdate(2,4)
      testControl:broadcast(ifd[4])
      drone2[1]:setHilight()
      drone2[1]:setVisplayer()
      drone2[1]:attack(pilot.player())
      testSix()
   end
end

function testSix() --done getting shot.
   --we poll the players health every 100 milliseconds.
   _,shield = pilot.player():health()
   if shield < 30 and missionStatus == 8 then --if the shield gets below 30%, we want to stop the drone from attacking.
      missionStatus = 9
      osdUpdate(2,5)
      testControl:broadcast(ifd[5])

      hook.rm(testHook) --stop the timer.
      drone2[1]:control() --stop the drone from attacking...
      drone2[1]:land(testAsset) --and get it out of the way.
      drone2[1]:setHilight(false)
      drone2[1]:setVisplayer(false)
      drone2[1]:setHostile(false)

      --set up the next phase. the hooks we did to droneGroup2 should handle getting to the next part of the mission.
      sysMrk = system.mrkAdd("Drone Group 1",droneGroup1Coords)
      sysMrk2 = system.mrkAdd("Drone Group 2",droneGroup2Coords)
   else
      hook.timer(100,"testSix")
   end
end

function startTheAttack() --and droneGroup2 finally got attacked.
   if missionStatus < 10 and missionStatus ~= 1 then --don't really care when the player attacks. it'll jump to EOM.
      missionStatus = 10
      osdUpdate(2,6)
      
      --get the frenzied testControl guy outta there after he calls for backup.
      ifd[8] = ifd[8]:format(player.name())
      testControl:broadcast(ifd[7])
      hookTCB = hook.timer(800,"testControlBroadcast")

      enemyGroup = {}
      system.mrkRm(sysMrk)
      system.mrkRm(sysMrk2)

      --swap out our dummy drones for enemy ships.
      for _,p in ipairs(droneGroup2) do
         if p:exists() then
            p:setFaction(enemyFaction)
            p:rename(enemyFaction:name() .. ' Lancelot')
            p:setHostile()
            p:setVisible()
            p:control(false)
            --hook each enemy pilot to an exploded function to see when they're all dead.
            hook.pilot(p,"exploded","countTheDead")
            table.insert(enemyGroup, p)
         end
      end

      --send in reinforcements. because the player will need them.
      for i = 1,12 do 
         newPt = pilot.add(friendlyFaction:name() .. " Lancelot",nil,testAsset)
         newPt[1]:setFriendly()
         newPt[1]:setVisible()
      end
   end
end

function countTheDead()
   --we wait until all the enemy ships are dead.
   numExploded = numExploded or 0
   numExploded = numExploded + 1
   if numExploded >= #enemyGroup then --yay you lived.
      missionStatus = 11
      if testControl:exists() then
         testControl:broadcast(ifd[10])
      end
      misn.osdActive(3)
      osdUpdate(3)
   end
end

--used after droneGroup2 becomes hostile.
function testControlBroadcast()
   hook.rm(hookTCB)
   testControl:broadcast(ifd[8])
   testControl:control()
   testControl:land(testAsset)
end

--used if the player attacks drone2
function notTheEnemy()
   testControl:broadcast(ifd[9])
   testControl:land(testAsset)
   misn.osdActive(3)
   missionStatus = -1
end

--update osd[2] with the osdmini{} things.
function osdUpdate(activenum,miniNum)
   osd[1] = osd[1]:format(testAsset:name(),testSys:name())
   osd[3] = osd[3]:format(testAsset:name())
   misn.osdDestroy()
   miniNum = miniNum or 0
   osd[2] = osdmini[miniNum]
   misn.osdCreate(misn_title,osd)
   misn.osdActive(activenum)
end

--remove the mkii ship. veeeeeeeeeery important.
function removeShip()
   player.swapShip(playerShipModel,playerShipName,nil,true,true) --give the player back their ship. may be buggy.
   pilot.rmOutfit(pilot.player(),"all")
   pilot.rmOutfit(pilot.player(),"cores")
   for _,o in ipairs(playerShipOutfits) do
      pilot.addOutfit(pilot.player(),o:name())
   end
end

function jumper() --basically, if the player jumps out with the mkii, they get to keep it.
   --but campaign over, that faction is now an enemy, and they technically fail the mission.
   --not too sure if i'm sold on this idea. but not sure how to handle the player jumping otherwise.
   if missionStatus == 1 and system.cur() == testSys then
      player.allowSave(false)
   end
   
   if missionStatus > 2 and missionStatus < 11 then
      fmsg[2] = fmsg[2]:format(handlerName,friendlyFaction:name())
      tk.msg(misn_title,fmsg[2])
      faction.modPlayer(friendlyFaction,-100)
      var.push("corpWarStolen",1)
      player.allowSave(true)
      misn.finish(true)
   end
end

function abort ()
   player.allowSave(true)
   misn.finish(false)
end
