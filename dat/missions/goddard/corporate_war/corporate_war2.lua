--[[

Corporate War mission 2.
The player needs to head to Za'lek space to pickup a shipment again, and then deliver that shipment back to Goddard space. There is a chance that a rep from Krain Industries will hail the player, and they could side with Krain from here on out.
start planet = Zhiru in the Goddard system.
pickup planet = Warnecke in the Octavian system.
end planet = Zhiru in the Goddard system or Krain Station in the Salvador system.

--]]

-- Bar information
bar_name = "A grinning Eryk"
bar_desc = "Eryk sits at the counter, happily grinning at you."

misn_title = "Goddard Courier Mission"
misn_desc = "Pickup and deliver a shipment from Za'lek space bound for the Goddard corporation."

bmsg = {} --beginning messages
pmsg = {} --pickup messages
emsg = {} --ending messages
fmsg = {} --failure messages
cmsg = {} --contact by krain industries guy.
bmsg[1] = [[You walk over to Eryk, who looks happy to see you. "Hey big guy," he begins as you sit next to him, "do you want to help me with picking up another shipment? We'll pay you %d for getting this one."]]

bmsg[2] = [[Eryk smiles even bigger. "Awesome! We need you to pick up a shipment on %s in the %s system. We know it's a long haul, so do your best to get it here quick but no pressure." He takes a sip of his drink before clapping you on the back and excusing himself to the men's room.]]

pmsg[1] = [[The dockhands make quick work loading the shipment bound for House Goddard onto your ship.]]

fmsg[1] = [[Eryk smiles a little less, but seems to stay happy. "Hey, that's all right. I'm sure I'll be around here somewhere if you want to go pick up another shipment for me." He takes another sip of his drink before excusing himself to the men's room.]] --player said no in the bar
fmsg[2] = [[A dockworker comes up to you wearing a scowl. He proceeds to lay into you for not having enough cargo space. He recommends a ship with a bigger cargo hold: "At least big enough to fit %d tons in. But maybe you should treat yourself you..." You stop listening there.]] --player does not have enough cargo

cmsg[1] = [[Your comm system blares to life as you make contact with the Krain Industries ship that's been hailing you. A man's voice emits over the speakers, and after clearing his throat, begins speaking. "Hello %s, I am Thregyn, a representative of Krain Industries. We have been told that you are carrying a shipment of spacecraft parts bound for Goddard. Krain Industries would be more than grateful if you could, instead, take it to %s in the %s system for us. Whatever House Goddard is paying you, we will increase it by 5%%. What do you say?"]]
cmsg[2] = [[The disembodied voice of Thregyn sounds pleased. "Glad to hear it! I should be there when you arrive. See you soon." The comm channel goes silent as you begin to readjust your auto-navigation systems.]]
cmsg[3] = [[The disembodied voice of Thregyn sounds angry. "I wish you'd reconsider, but I get the feeling you won't." The comm channel cuts off quickly, and you get the feeling you did the right thing.]]


emsg[1] = [[You land on %s, to be greeted by Eryk, only this time he isn't smiling. He shakes your hand before speaking. "We heard Krain Industries was trying to get ahold of you. I don't know if they reached you or not, and I don't care." He guestures to the shipments. "All that matters is we have these parts now. If you'd meet me in the bar at your earliest convenience, we have something we need to discuss. Also, of course, you've been paid." With that, Eryk disappears after the crates that have been unloaded from your ship.]] --goddard ending
emsg[2] = [[You get clearance to land on %s, and you steer your ship in carefully. The docks are filled with people waiting for your ship. No sooner have you landed than you see people taking the crates out of your ship and hurrying them across the docks. One man approaches you and shakes your hand. "Hello, %s. I am Thregyn. Thank you for choosing to deliver these to us. We've credited your account what we owe you. If you'd like to meet me in the bar at your earliest convenience, I'd be more than grateful." With that, Thregyen walks after the crates, leaving you suddenly alone.]] --krain ending

osd = {}
osd[1] = "Land on %s in the %s system."
osd[2] = "" --will be formatted from osd2temp
osd2temp = "Return with the shipment to %s in the %s system." --format osd[2] from this so the %s doesn't get overwritten.

function create ()
   misn.setNPC( bar_name, "neutral/male1" )
   misn.setDesc( bar_desc )
   
   misn_reward = 75000 + faction.playerStanding("Goddard") * 3000 
   pickupAsset, pickupSys = planet.get("Warnecke")
   returnAsset, returnSys = planet.get("Zhiru")
   krainAsset, krainSys = planet.get("Krain Station")
   cargoSize = 10

   osd[1] = osd[1]:format(pickupAsset:name(), pickupSys:name())
   osd[2] = osd2temp:format(returnAsset:name(), returnSys:name())
   bmsg[1] = bmsg[1]:format(misn_reward)
   bmsg[2] = bmsg[2]:format(pickupAsset:name(), pickupSys:name())
   fmsg[2] = fmsg[2]:format(cargoSize)
   cmsg[1] = cmsg[1]:format(player.name(),krainAsset:name(),krainSys:name())
   emsg[2] = emsg[2]:format(krainAsset:name(),player.name())
end


function accept ()
   if not tk.yesno( misn_title, bmsg[1] ) then
      tk.msg(misn_title,fmsg[1])
      misn.finish(false)
   end
   
   misn.accept() 
   tk.msg( misn_title, bmsg[2] )
   misn.setTitle( misn_title)
   misn.setReward( misn_reward)
   misn.setDesc( misn_desc)
   misnMarker = misn.markerAdd(pickupSys,"high")

   missionStatus = 1

   misn.osdCreate(misn_title,osd)
   misn.osdActive(missionStatus)

   hook.land("lander")
end

function lander()
   if missionStatus == 1 and planet.cur() == pickupAsset then
      if pilot.player():cargoFree() < cargoSize then
         tk.msg(misn_title, fmsg[2])
      else
         tk.msg(misn_title,pmsg[1])
         missionCargo = misn.cargoAdd("Equipment",10)
         missionStatus = 2
         misn.osdActive(missionStatus)
         misn.markerMove(misnMarker, returnSys) 
         hook.jumpin("krainContact")
      end
   elseif missionStatus == 2 and planet.cur() == returnAsset then
      misn.cargoRm(missionCargo)
      if returnAsset == planet.get("Zhiru") then
         tk.msg(misn_title,emsg[1])
         var.push("corpWarFriend","Goddard")
         var.push("corpWarEnemy","Krain")
      elseif returnAsset == planet.get("Krain Station") then
         tk.msg(misn_title,emsg[2])
         var.push("corpWarFriend","Krain")
         var.push("corpWarEnemy","Goddard")
      end
      player.pay(misn_reward)
      misn.markerRm(misnMarker)
      faction.modPlayer(faction.get(var.peek("corpWarFaction")),3)
      faction.modPlayer(faction.get(var.peek("corpWarEnemy")),-5)
      misn.osdDestroy()
      misn.finish(true)
   end

end

function krainContact()
   contactRoller = rnd.rnd(4)
   if contactRoller == 0 and not contactBool then
      krainPilot = pilot.add("Krain Kestrel")
      krainPilot[1]:control(true)
      krainPilot[1]:hailPlayer()
      krainPilot[1]:follow(pilot.player())
      hook.pilot(krainPilot[1], "hail", "contactMade")
   end
end

function contactMade(krainPilot)
   contactBool = true
   krainPilot:hyperspace(nil)
   if tk.yesno(misn_title,cmsg[1]) then
      tk.msg(misn_title,cmsg[2])
      misn_reward = misn_reward * 1.05
      returnAsset, returnSys = planet.get("Krain Station")
      misn.markerMove(misnMarker, returnSys)
      osd[2] = osd2temp:format(returnAsset:name(), returnSys:name())
   else
      tk.msg(misn_title,cmsg[3])
   end
end

function abort ()
   if not missionCargo == nil then
      misn.cargoRm(missionCargo)
   end
   misn.finish(false)
end
