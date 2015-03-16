--[[

Corporate War mission 3.
The player learns of the Goddard/Kestrel MkIIs. He also learns that someone running shipments for friendly-corp defected to the other one. The player is tasked with disabling the enemy-corps prototype ship, boarding it, allowing some engineers to steal the technology back, and returning with it.
start planet = Zhiru in the Goddard system or Krain Station in the Salvador system.
combat system = somewhere out of the way.
end planet = same as start.

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


bmsg[1] = [[%s greets you as you approach. "Good to see you, %s. If you haven't yet figured it out, %s wanted that delivery because they are building a new ship. A bigger, better, faster ship. Good thing for us is so are we." % grimaces slightly as he furtively peers around. "All of this is top-secret, of course. Now that you know, we need your help. %s needs your help. What do you say? We can reward you well."]] --handlerName, player.name(), enemyfaction, handler name, friendly faction
bmsg[2] = [[%s appears relieved as you answer in the affirmative. "Great. Now listen carefully. We just received word that %s is going to be testing their new prototype. Their ship appears to be further along in development than ours, which isn't good. We need you to take some marines and some engineers, and board their prototype. They will steal some technology, and you need to bring it and the personnel back here. When you get back, we will pay you %d credits." He claps you on the back. "This is big, %s. Thanks for your help." You get up to get prepping.]] --handlername, enemyfaction, misn reward, player name.

fmsg[1] = [[%s frowns at you. "It looks like your ship doesn't have enough cargo room. Can you get a ship with at least %d tons worth of space free? Come back when you have." With that, you get up and walk away.]] --player does not have enough cargo.
fmsg[2] = [[%s looks disappointed. "Well, if you change your mind, come on back and let me know." And with that, %s pointedly turns to his drink.]] --player said no in the bar.
fmsg[3] = [[Your comm blares to life, and you hear %s come on over it. "%s, after you jumped %s was able to get the prototype to safety. Looks like we are out of luck today. Come talk to me when you are ready to try again." Your comm then falls abruptly silent.]] --player left combatsys early.

emsg[1] = [[You land on %s, grateful to have made it back safely with the equipment and the personnel. The engineers and the marines quickly took off with the cargo, with you exiting your ship shortly after them. You see %s walking across the hanger, flagging you down. "Great job! When you are ready for some more, it looks like we may have another job for you. Come meet me in the bar in a few." And with that, he turns and leaves the way he came.]] 

boardmsg[1] = [[You successfully latch on to the disabled prototype ship, and the engineers and marines shuffle over. You hear small arms fire echoing, before the marines and engineers come running back to your ship with several crates in tow. One of the marines yells at you to punch it and get out; you are more than happy to oblige.]]

osd = {}
osd[1] = "Fly to %s and board the prototype ship."
osd[2] = "Return with the personnel and cargo to %s in the %s system."

function create ()

   friendlyFaction = faction.get(var.peek("corpWarFaction"))
   enemyFaction = faction.get(var.peek("corpWarEnemy"))

   startAsset,startSys = planet.cur()
   combatSys = system.get("Mason")

   cargoSize = 15

   if friendlyFaction == faction.get("Goddard") then
      handlerName = "Eryk"
   else
      handlerName = "Thregyn"

   bar_name = bar_name:format(handlerName)
   bar_desc = bar_desc:format(handlerName)

   bmsg[1] = bmsg[1]:format(handlerName, player.name(), enemyFaction:name(), handlerName, friendlyFaction:name())
   bmsg[2] = bmsg[2]:format(handlerName, enemyFaction:name(), misn_reward, player.name())
   fmsg[1] = fmsg[1]:format(handlerName, cargoSize)
   fmsg[2] = fmsg[2]:format(handlerName, handlerName)
   fmsg[3] = fmsg[3]:format(handlerName, player.name(), enemyFaction:name())
   emsg[1] = emsg[1]:format(startAsset:name(), handlerName)
   osd[1] = osd[1]:format(combatSys:name())
   osd[2] = osd[2]:format(startAsset:name(), startSys:name())

   misn.setNPC( bar_name, "neutral/male1" )
   misn.setDesc( bar_desc )   
   misn_reward = 90000 + faction.playerStanding(friendlyFaction) * 3000 
end


function accept ()
   if not tk.yesno( misn_title, bmsg[1] ) then
      tk.msg(misn_title,fmsg[2])
      misn.finish(false)
   end
   if pilot.player():cargoFree() < cargoSize then
      tk.msg(misn_title,fmsg[1])
      misn.finish(false)
   end
   
   misn.accept() 
   tk.msg( misn_title, bmsg[2] )
   misn.setTitle( misn_title)
   misn.setReward( misn_reward)
   misn.setDesc( misn_desc)
   misn.markerAdd(combatSys)
   personnelCargo = misn.cargoAdd("Personnel", 5)

   missionStatus = 1

   misn.osdCreate(misn_title,osd)
   misn.osdActive(missionStatus)
   
   hook.jumpin("jumper")
   hook.land("lander")
end

function jumper()
end

function lander()
end

function abort ()
   if not personellCargo == nil then
      misn.cargoRm(personnelCargo)
   end
   if not equipmentCargo == nil then
      misn.cargoRm(equipmentCargo)
   end
   misn.finish(false)
end
