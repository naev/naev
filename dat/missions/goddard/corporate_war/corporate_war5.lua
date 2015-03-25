--[[

Corporate War mission 5.
In the second to last CPW mission, the player must fly to the home system of the enemy, destroy the prototype, land on the enemy base, and escape with his life. Should be less code than CPW4. I hope.

This mission attempts to claim the "targetSys", which will either be Goddard or Salvador, depending on which faction the player has sided with.

framework is pretty much laid, just need to fill it in and do dialogue.
and probably pick a better misn_title.

--]]

   bar_name = "%s"
   bar_desc = "%s sits at a booth, waiting expectantly for you."

   misn_title = "Right in the Kisser"
   misn_desc = "Destroy the prototype ship, and then board the enemy station to delete records."

   bmsg = {} --beginning messages
   emsg = {} --ending messages
   fmsg = {} --failure messages
   bmsg[1] = [[]]

   fmsg[1] = [[]] --player said no in the bar
   
   emsg[1] = [[]]

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

   bar_name = bar_name:format(handlerName)
   bar_desc = bar_desc:format(handlerName)

   misn.setNPC( bar_name, "neutral/male1" )
   misn.setDesc( bar_desc )
   
   misn_reward = 150000 + faction.playerStanding(friendlyFaction) * 3000 
   
end


function accept ()
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
      --turn off pirates and other baddies. we want total control!
      pilot.toggleSpawn("Pirate")
      pilot.toggleSpawn("Krain")
      pilot.toggleSpawn("Goddard")
      --really wish i could use fleet_form.lua here.
      --form the enemies in a random spot.
      enemyFormX = rnd.rnd(-12500,12500)
      enemyFormY = rnd.rnd(-12500,12500)
      enemyGroup = {}
      prototype = pilot.add(enemyFaction():name() .. " Prototype",nil,vec2.new(enemyFormX,enemyFormY))
      prototype[1]:setHilight()
      prototype[1]:setHostile()
      prototype[1]:setNoJump()
      prototype[1]:setNoLand()
      for i = 1, 8 do
         enemyFormOffsetX = rnd.rnd(-250,250)
         enemyFormOffsetY = rnd.rnd(-250,250)
         newPt = pilot.add(enemyFaction():name() .. " Lancelot",nil,vec2.new(enemyFormX,enemyFormY) + vec2.new(enemyFormOffsetX, enemyFormOffsetY))
         table.insert(enemyGroup, newPt[1])
      end
      enemyFormOffsetX = rnd.rnd(-250,250)
      enemyFormOffsetY = rnd.rnd(-250,250)
      if enemyFaction():name() == "Goddard" then
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
      tk.msg(misn_title,fmsg[2])
      misn.finish(false)
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
      pilot.toggleSpanw("Krain")

      --add some baddies for some stress.

      for i = 1, 8 do
         newPt = pilot.add(enemyFaction:name() .. " Lancelot",nil,combatAsset)
         newPt[1]:setHostile()
         newPt[1]:setNoJump()
         newPt[1]:setNoLand()
      end
      
      for i = 1, 2 do
         if enemyFaction:name() == "Goddard" then
            newPt = pilot.add("Goddard Goddard",nil,combatAsset)
         else
            newPt = pilot.add("Krain Kestrel",nil,combatAsset)
         end
         newPt[1]:setHostile()
         newPt[1]:setNoJump()
         newPt[1]:setNoLand()
      end
   end
end
--------------------------------need to finish below.
function lander()
   if missionStatus == 3 and planet.cur() == combatAsset then
      missionStatus = 4
      misn.osdActive(missionStatus)
      --flavatext.
      player.takeoff()
   elseif missionStatus == 2 or missionStatus == 3 and planet.cur() ~= combatAsset then
      --player landed too early or wrong and fails the mission.
   elseif missionStatus == 4 and planet.cur() == returnAsset then
      --mission is over.
   end
end

function abort ()
   misn.finish(false)
end
