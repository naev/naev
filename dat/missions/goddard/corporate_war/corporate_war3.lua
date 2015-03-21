--[[

Corporate War mission 3.
The player learns of the Goddard/Kestrel MkIIs. He also learns that someone running shipments for friendly-corp defected to the other one. The player is tasked with disabling the enemy-corps prototype ship, boarding it, allowing some engineers to steal the technology back, and returning with it.
start planet = Zhiru in the Goddard system or Krain Station in the Salvador system.
combat system = somewhere out of the way.
end planet = same as start.

This mission tries to claim the system "combatSys", currently set to the system Mason.

--]]

-- Bar information
bar_name = "%s"
bar_desc = "%s sits here darkly."

misn_title = "Corporate Duties"
misn_desc = "Board a prototype ship to steal technology from it."

bmsg = {} --beginning messages
emsg = {} --ending messages
fmsg = {} --failure messages
boardmsg = {}


bmsg[1] = [[%s greets you as you approach. "Good to see you, %s. If you haven't yet figured it out, %s wanted that delivery because they are building a new ship. A bigger, better, faster ship. Good thing for us is so are we." %s grimaces slightly as he furtively peers around. "All of this is top-secret, of course. Now that you know, we need your help. %s needs your help. What do you say? We can reward you well."]] --handlerName, player.name(), enemyfaction, handler name, friendly faction
bmsg[2] = [[%s appears relieved as you answer in the affirmative. "Great. Now listen carefully. We just received word that %s is going to be testing their new prototype. Their ship appears to be further along in development than ours, which isn't good. We need you to take some marines and some engineers, and board their prototype. They will steal some technology, and you need to bring it and the personnel back here. When you get back, we will pay you %d credits." He claps you on the back. "This is big, %s. Thanks for your help." You get up to get prepping.]] --handlername, enemyfaction, misn reward, player name.

fmsg[1] = [[%s frowns at you. "It looks like your ship doesn't have enough cargo room. Can you get a ship with at least %d tons worth of space free? Come back when you have." With that, you get up and walk away.]] --player does not have enough cargo.
fmsg[2] = [[%s looks disappointed. "Well, if you change your mind, come on back and let me know." And with that, %s pointedly turns to his drink.]] --player said no in the bar.
fmsg[3] = [[Your comm blares to life, and you hear %s come on over it. "%s, after you jumped %s was able to get the prototype to safety. Looks like we are out of luck today. Come talk to me when you are ready to try again." Your comm then falls abruptly silent.]] --player left combatsys before the ship was boarded.
fmsg[4] = [[Your comm squaks to life, and you hear %s raging on the other end. "The prototype ship is dead! How could this have happened? Well, whatever. They probably have another one somewhere; we just need to wait for it to surface." The comm system abruptly cuts off.]] --protoShip is dead.
fmsg[5] = [[Your comm snaps on and you hear the voice of %s begin speaking. "%s, while you were landing, %s had enough time to get their prototype ship to safety. It looks like we won't be able to complete the mission today. Meet me back on %s when you're ready to have another go." The comm dies, leaving the cabin eerily quiet.]] --if the player lands before the ship has been boarded.

emsg[1] = [[You land on %s, grateful to have made it back safely with the equipment and the personnel. The engineers and the marines quickly took off with the cargo, with you exiting your ship shortly after them. You see %s walking across the hanger, flagging you down. "Great job! When you are ready for some more, it looks like we may have another job for you. Come meet me in the bar in a few." And with that, he turns and leaves the way he came.]] 

boardmsg[1] = [[You successfully latch on to the disabled prototype ship, and the engineers and marines shuffle over. You hear small arms fire echoing, before the marines and engineers come running back to your ship with several crates in tow. One of the marines yells at you to punch it and get out; you are more than happy to oblige.]]

osd = {}
osd[1] = "Fly to the %s system."
osd[2] = "Disable and board the prototype ship."
osd[3] = "Return with the personnel and cargo to %s in the %s system."

function create ()
   
   --This mission tries to claim "combatSys", i.e. Mason.

   friendlyFaction = faction.get(var.peek("corpWarFaction"))
   enemyFaction = faction.get(var.peek("corpWarEnemy"))

   startAsset,startSys = planet.cur()
   combatSys = system.get("Mason")

   cargoSize = 15

   if friendlyFaction == faction.get("Goddard") then
      handlerName = "Eryk"
   else
      handlerName = "Thregyn"
   end

   if not misn.claim(combatSys) then
      misn.finish(false)
   end

   bar_name = bar_name:format(handlerName)
   bar_desc = bar_desc:format(handlerName)
   misn_reward = 90000 + faction.playerStanding(friendlyFaction) * 3000 

   bmsg[1] = bmsg[1]:format(handlerName, player.name(), enemyFaction:name(), handlerName, friendlyFaction:name())
   bmsg[2] = bmsg[2]:format(handlerName, enemyFaction:name(), misn_reward, player.name())

   osd[1] = osd[1]:format(combatSys:name())
   osd[3] = osd[3]:format(startAsset:name(), startSys:name())

   misn.setNPC( bar_name, "neutral/male1" )
   misn.setDesc( bar_desc )   
end


function accept ()
   if not tk.yesno( misn_title, bmsg[1] ) then
      fmsg[2] = fmsg[2]:format(handlerName, handlerName)
      tk.msg(misn_title,fmsg[2])
      misn.finish(false)
   end
   if pilot.player():cargoFree() < cargoSize then
      fmsg[1] = fmsg[1]:format(handlerName, cargoSize)
      tk.msg(misn_title,fmsg[1])
      misn.finish(false)
   end
   
   misn.accept() 
   tk.msg( misn_title, bmsg[2] )
   misn.setTitle( misn_title)
   misn.setReward( misn_reward)
   misn.setDesc( misn_desc)
   missionMarker = misn.markerAdd(combatSys, "high")
   personnelCargo = misn.cargoAdd("Personnel", 5)

   missionStatus = 1

   misn.osdCreate(misn_title,osd)
   misn.osdActive(missionStatus)
   
   hook.jumpin("jumper")
   hook.land("lander")
end

function jumper()
   if missionStatus == 1 and system.cur() == combatSys then --time for combat action.
      
      --update the mission status
      missionStatus = 2
      misn.osdActive(missionStatus)
      
      --set up the system
      pilot.toggleSpawn("Pirate", false)

      --where the enemy ships are gonna be located at
      enemyShipLocX = rnd.rnd(-7500,7500)
      enemyShipLocY = rnd.rnd(-7500,7500)
      enemyShipLoc = vec2.new(enemyShipLocX,enemyShipLocY)
      
      --set up the prototype ship
      if enemyFaction == faction.get("Goddard") then
         protoShip = pilot.add("Goddard Prototype",nil,enemyShipLoc)
      else
         protoShip = pilot.add("Kestrel Prototype",nil,enemyShipLoc)
      end
      protoShip[1]:setVisible()
      protoShip[1]:control()
      protoShip[1]:brake()
      if not protoShip[1]:hostile() then
         hook.pilot(protoShip[1], "attacked", "attacking")
         hook.pilot(protoShip[1], "death", "protoShipDead")
         hook.pilot(protoShip[1], "board", "protoShipBoard")
      end


      --set up supporting ships.
      --they will initially be in a circle around the protoship.
      enemyShip = {}
      for i = 1, 8 do
         enemyShip_new = pilot.add(enemyFaction:name() .. " Lancelot",nil,enemyShipLoc)
         table.insert(enemyShip, enemyShip_new[1])
      end
      angle = math.pi * 2 / #enemyShip
      radius = 80 + #enemyShip * 25
      for i,p in ipairs(enemyShip) do
         p:setVisible()
         p:control()
         p:brake()
         x = radius * math.cos(angle * i)
         y = radius * math.sin(angle * i)
         p:setPos(enemyShipLoc + vec2.new(x,y))
         if not p:hostile() then
            hook.pilot(p, "attacked", "attacking")
         end
      end
   elseif missionStatus == 2 then --if the player jumps out early.
      fmsg[3] = fmsg[3]:format(handlerName, player.name(), enemyFaction:name())
      tk.msg(misn_title,fmsg[3])
      abort()
   end
end

function protoShipDead(pDead, pAttacker)
   if missionStatus == 2 then
      fmsg[4] = fmsg[4]:format(handlerName)
      tk.msg(misn_title,fmsg[4])
      abort()
   end
end

function protoShipBoard(pBoarded, pBoarder)
   if pBoarder == pilot.player() then
      tk.msg(misn_title,boardmsg[1])
      missionStatus = 3
      equipmentCarg = misn.cargoAdd("Equipment", 10)
      misn.osdActive(missionStatus)
      misn.markerMove(missionMarker,startSys)
      player.unboard()
   end
end

--once the player attacks the ships, this sets the whole fleet to hostile.
function attacking(pAttacked, pAttacker)
   --there shouldn't be any hostile ships other than the player, as pirates are disabled, so I'm assuming the attacker is the player. /lazy
   if not protoShip[1]:hostile() then
      protoShip[1]:control(false)
      protoShip[1]:setHostile()
   end
   for _,p in ipairs(enemyShip) do
      if p:exists() and not p:hostile() then --don't want to keep setting control to false if they're already hostile.
         p:control(false)
         p:setHostile()
      end
   end
end

function lander()
   if missionStatus == 2 then
      tk.msg(misn_title,fmsg[5])
   elseif missionStatus == 3 and planet.cur() == startAsset then
      emsg[1] = emsg[1]:format(startAsset:name(), handlerName)
      tk.msg(misn_title,emsg[1])
      misn.cargoRm(personnelCargo)
      misn.cargoRm(equipmentCargo)
      player.pay(misn_reward)
      misn.markerRm(missionMarker)
      misn.finish(true)
   end
end

function abort ()
   if not personnelCargo == nil then
      misn.cargoRm(personnelCargo)
   end
   if not equipmentCargo == nil then
      misn.cargoRm(equipmentCargo)
   end
   misn.finish(false)
end
