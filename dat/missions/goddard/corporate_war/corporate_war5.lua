--[[

Corporate War mission 5.
In the second to last CPW mission, the player must fly to the home system of the enemy, destroy the prototype, land on the enemy base, and escape with his life. Should be less code than CPW4. I hope.

This mission attempts to claim the "targetSys", which will either be Goddard or Salvador, depending on which faction the player has sided with.

framework is pretty much laid, just need to fill it in and do dialogue.
and probably pick a better misn_title.

TODO:
needs balancing.
needs to feel more epic.

--]]

   bar_name = "%s"
   bar_desc = "%s sits at a booth, waiting expectantly for you."

   misn_title = "Corporate Revenge"
   misn_desc = "Destroy the prototype ship, and then board the enemy station to delete records."

   bmsg = {} --beginning messages
   emsg = {} --ending messages
   fmsg = {} --failure messages
   lmsg = {} --landed on combatAsset messages

   bmsg[1] = [[You walk up to %s, and sit down in the booth across from him. He flashes a grin at you. "Good job testing the prototype. We never expected %s to take a shot like that." He pauses and settles into the seat a little more. "We've heard that %s is developing a prototype of their own. In fact, we hear that soon, they are going to be testing it. We need to retaliate. Would you be interested in a little payback?" He sits back in his chair and waits for you to respond.]]
   bmsg[2] = [[%s grins again. "Good. I'm not going to lie to you, this isn't going to be easy. And you are going to be the only friendly ship out there. You need to take a team of engineers to %s, in the %s system. First, you have to destroy the prototype. Then, you need to land. The engineers are going to put a virus in their network to wipe out any data about their prototype." %s grins mischievously. "This should put them back behind us. And teach us a lesson. The engineers are already boarding. Good luck, %s." He sits back in his seat as you get up to get ready.]]

   fmsg[1] = [[You politely decline the offer to a grimacing %s. "Well, if you change your mind, come back and talk to me. We have a small window of opportunity here." You get up and take your leave.]] --player said no in the bar
   fmsg[2] = [[As soon as you finish your jump, your comm blares to life with the frenetic voice of %s. "Great, thanks for jumping out! That gave them time to get the prototype to safety and harden their defenses. Come talk to me in a bit, and we will give it another go." The comm falls silent.]] --player jumped early
   fmsg[3] = [[You pilot yourself through the atmosphere, about to make your final approach when the comm blares to life with the voice of %s. "While you were busy landing, %s was able to get it's prototype ship to safety and harden it's defences! Come see me in a bit, and we can talk about giving it another go." Your comm becomes quiet once again.]] --player landed early/wrong

   emsg[1] = [[You land on %s, only to be greeted by a smiling %s. "Great job out there! You really gave it to them. Listen, we have another job for you. Come talk to me in the bar when you are ready." %s turns and strides away, and you find yourself a little bit richer.]]

   lmsg[1] = [[You guide your ship onto %s, despite warning messages blaring. You dock hard, and open the doors quickly to see the engineers sprinting out. After what seems like an eternity, you hear small arms fire followed by shouts, and see some nearby consoles on the docks explode. Some of the engineers come sprinting from a nearby corridor and throw themselves in your ship. You hear them yelling for you to punch it. You happily oblige.]]

   osd = {}
   osd[1] = "Fly to the %s system."
   osd[2] = "Destroy the %s prototype."
   osd[3] = "Land on %s."
   osd[4] = "Return safely to %s."

function create ()

   friendlyFaction = faction.get(var.peek("corpWarFaction"))
   enemyFaction = faction.get(var.peek("corpWarEnemy"))

   if friendlyFaction == faction.get("Goddard") then
      handlerName = "Eryk"
      combatAsset, combatSys = planet.get("Krain Station")
   else
      handlerName = "Thregyn"
      combatAsset, combatSys = planet.get("Manuel Station")
   end

   returnAsset = planet.cur()

   if not misn.claim(combatSys) then
      misn.finish(false)
   end

   bar_name = bar_name:format(handlerName)
   bar_desc = bar_desc:format(handlerName)

   misn.setNPC( bar_name, "neutral/male1" )
   misn.setDesc( bar_desc )
   
   misn_reward = 150000 + faction.playerStanding(friendlyFaction) * 3000 
   
   osd[1] = osd[1]:format(combatSys:name())
   osd[2] = osd[2]:format(enemyFaction:name())
   osd[3] = osd[3]:format(combatAsset:name())
   osd[4] = osd[4]:format(returnAsset:name())

end


function accept ()

   bmsg[1] = bmsg[1]:format(handlerName,enemyFaction:name(),enemyFaction:name())
   bmsg[2] = bmsg[2]:format(handlerName,combatAsset:name(),combatSys:name(),handlerName,player.name())
   fmsg[1] = fmsg[1]:format(handlerName)

   if not tk.yesno( misn_title, bmsg[1] ) then
      tk.msg(misn_title,fmsg[1])
      misn.finish(false)
   end
   
   misn.accept()
   tk.msg(misn_title, bmsg[2])
   misn.setTitle(misn_title)
   misn.setReward(misn_reward)
   misn.setDesc(misn_desc)
   misnMarker = misn.markerAdd(combatSys,"high") --change as appropriate to point to a system object and marker style.

   missionStatus = 1
   empireStanding = faction.get("Empire"):playerStanding() --we don't want empire rep to suffer because of this campaign.

   misn.osdCreate(misn_title,osd)
   misn.osdActive(missionStatus)

   hook.jumpin("jumper")
   hook.takeoff("takingoff")
   hook.land("lander")
end

function jumper()
   if missionStatus == 1 and system.cur() == combatSys then
      missionStatus = 2
      misn.osdActive(missionStatus)
      --turn off pirates and other baddies and remove ships already in the system. we want total control!
      pilot.toggleSpawn("Pirate")
      pilot.toggleSpawn("Krain")
      pilot.toggleSpawn("Empire")
      pilot.toggleSpawn("Goddard")
      for _,p in ipairs(pilot.get()) do
         if p ~= pilot.player() then
            p:rm()
         end
      end
      --really wish i could use fleet_form.lua here.
      --form the enemies in a random spot.
      enemyFormX = rnd.rnd(-12500,12500)
      enemyFormY = rnd.rnd(-12500,12500)
      enemyGroup = {}
      if enemyFaction:name() == "Goddard" then
         shipName = "Goddard Prototype"
      else
         shipName = "Kestrel Prototype"
      end
      prototype = pilot.add(shipName,nil,vec2.new(enemyFormX,enemyFormY))
      prototype[1]:setHilight()
      prototype[1]:setHostile()
      prototype[1]:setNoJump()
      prototype[1]:setNoLand()
      prototype[1]:setVisplayer()
      for i = 1, 8 do
         enemyFormOffsetX = rnd.rnd(-250,250)
         enemyFormOffsetY = rnd.rnd(-250,250)
         newPt = pilot.add(enemyFaction:name() .. " Lancelot",nil,vec2.new(enemyFormX,enemyFormY) + vec2.new(enemyFormOffsetX, enemyFormOffsetY))
         table.insert(enemyGroup, newPt[1])
      end
      enemyFormOffsetX = rnd.rnd(-250,250)
      enemyFormOffsetY = rnd.rnd(-250,250)
      if enemyFaction:name() == "Goddard" then
         newPt = pilot.add("Goddard Goddard",nil,vec2.new(enemyFormX,enemyFormY) + vec2.new(enemyFormOffsetX,enemyFormOffsetY))
      else
         newPt = pilot.add("Krain Kestrel",nil,vec2.new(enemyformX,enemyFormY) + vec2.new(enemyFormOffsetX,enemyFormOffsetY))
      end
      table.insert(enemyGroup, newPt[1])

      --enemies created. now to just make sure they're hostile.
      for _,p in ipairs(enemyGroup) do
         --should we set them visible? i'm opting for no.
         p:setHostile()
         p:setNoJump()
         p:setNoLand()
      end
      --at this point, we only care about the prototype being dead.
      hook.pilot(prototype[1],"exploded","stationAssault")
   elseif missionStatus == 2 or missionStatus == 3 then
      --player failed. they can retry later.
      fmsg[2] = fmsg[2]:format(handlerName)
      tk.msg(misn_title,fmsg[2])
      misn.finish(false)
   elseif missionStatus == 4 then
      player.allowSave(true)
   end
end

function stationAssault()
   --prototype should be ded [sic].
   missionStatus = 3
   misn.osdActive(missionStatus)
   --this might be wrong.
   if not combatAsset:canLand() then
      combatAsset:landOverride(true)
   end
end

function takingoff()
   if missionStatus == 4 and system.cur() == combatSys then
      --create some bad guy ships for some flava.
      --turn off dem pirates.
      pilot.toggleSpawn("Pirate")
      pilot.toggleSpawn("Goddard")
      pilot.toggleSpawn("Krain")
      for _,p in ipairs(pilot.get()) do
         if p ~= pilot.player() then
            p:rm()
         end
      end

      --add some baddies for some stress.

      for i = 1, rnd.rnd(2,4) do
         newPt = pilot.add(enemyFaction:name() .. " Lancelot",nil,combatAsset)
         newPt[1]:setHostile()
         newPt[1]:setNoJump()
         newPt[1]:setNoLand()
      end
      
      for i = 1, rnd.rnd(4,8) do
         newPt = pilot.add(enemyFaction:name() .. " Lancelot",nil,combatAsset:pos() + vec2.new(rnd.rnd(-5000,5000),rnd.rnd(-5000,5000)))
         newPt[1]:setHostile()
         newPt[1]:setNoJump()
         newPt[1]:setNoLand()
      end

      for i = 1, 2 do
         if enemyFaction:name() == "Goddard" then
            newPt = pilot.add("Goddard Goddard",nil,combatAsset:pos() + vec2.new(rnd.rnd(-5000,5000),rnd.rnd(-5000,5000)))
         else
            newPt = pilot.add("Krain Kestrel",nil,combatAsset:pos() + vec2.new(rnd.rnd(-5000,5000),rnd.rnd(-5000,5000)))
         end
         newPt[1]:setHostile()
         newPt[1]:setNoJump()
         newPt[1]:setNoLand()
      end
   end
end

function lander()
   if missionStatus == 3 and planet.cur() == combatAsset then
      player.allowSave(false)
      missionStatus = 4
      misn.osdActive(missionStatus)
      lmsg[1] = lmsg[1]:format(combatAsset:name())
      tk.msg(misn_title,lmsg[1])
      player.takeoff()
   elseif missionStatus == 2 or missionStatus == 3 and planet.cur() ~= combatAsset then
      --player landed too early or wrong and fails the mission.
      fmsg[3] = fmsg[3]:format(handlerName,enemyFaction:name())
      tk.msg(misn_title,fmsg[3])
      misn.finish(false)
   elseif missionStatus == 4 and planet.cur() == returnAsset then
      --mission is over.
      emsg[1] = emsg[1]:format(returnAsset:name(),handlerName,handlerName)
      tk.msg(misn_title,emsg[1])
      player.pay(misn_reward)
      faction.modPlayerRaw("Empire",empireStanding - faction.get("Empire"):playerStanding())
      misn.finish(true)
   end
end

function abort ()
   misn.finish(false)
end
