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
      --start in-space combat stuff.
      --turn off dem pirates.
      --destroy the prototype, etc.
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
end

function takingoff()
   if missionStatus == 4 and system.cur() == combatSys then
      --create some bad guy ships for some flava.
      --turn off dem pirates.
      --should be the only thing done in this hook
   end
end

function lander()
   if missionStatus == 3 and planet.cur() == combatAsset then
      missionStatus = 4
      misn.osdActive(missionStatus)
      --landed on the planet!
      --do some flava text.
   elseif missionStatus == 2 or missionStatus == 3 and planet.cur() ~= combatAsset then
      --player landed too early or wrong and fails the mission.
   elseif missionStatus == 4 and planet.cur() == returnAsset then
      --mission is over.
   end
end

function abort ()
   misn.finish(false)
end
