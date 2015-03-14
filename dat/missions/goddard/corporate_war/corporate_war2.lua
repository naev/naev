--[[

Corporate War mission 1.
The player needs to head to Za'lek space to pickup a shipment, and then deliver that shipment back to Goddard space.
start planet = Zhiru in the Goddard system.
pickup planet = Topkapi Station in the Damien system.
end planet = Zhiru in the Goddard system.

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
bmsg[1] = [[]]

bmsg[2] = [[]] --pickupAsset:name(), pickupSys:name(), returnAsset:name(), misn_reward

pmsg[1] = [[]]

fmsg[1] = [[]] --player said no in the bar
fmsg[2] = [[]] --player does not have enough cargo

emsg[1] = [[]]

osd = {}
osd[1] = ""
osd[2] = ""

function create ()
   misn.setNPC( bar_name, "neutral/male1" )
   misn.setDesc( bar_desc )
   
   misn_reward = 60000 + faction.playerStanding("Goddard") * 3000 
   pickupAsset, pickupSys = planet.get("Topkapi Station")
   returnAsset, returnSys = planet.get("Zhiru")
   
   bmsg[2] = bmsg[2]:format(pickupAsset:name(), pickupSys:name(), returnAsset:name(), misn_reward)
   emsg[1] = emsg[1]:format(returnAsset:name())
   osd[1] = osd[1]:format(pickupAsset:name(), pickupSys:name())
   osd[2] = osd[2]:format(returnAsset:name(), returnSys:name())
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
   misn.markerAdd(system.get("Damien"),"high")

   missionStatus = 1

   misn.osdCreate(misn_title,osd)
   misn.osdActive(missionStatus)

   hook.land("lander")
end

function lander()
   if missionStatus == 1 and planet.cur() == pickupAsset then
      if pilot.player():cargoFree() < 10 then
         tk.msg(misn_title, fmsg[2])
      else
         tk.msg(misn_title,pmsg[1])
         missionCargo = misn.cargoAdd("Equipment",10)
         missionStatus = 2
         misn.osdActive(missionStatus)
         hook.jumpin("krainContact")
      end
   elseif missionStatus == 2 and planet.cur() == returnAsset then
      misn.cargoRm(missionCargo)
      faction.modPlayer("Goddard",3)
      player.pay(misn_reward)
      tk.msg(misn_title,emsg[1])
      misn.osdDestroy()
      misn.finish(true)
   end

end

function krainContact()
   contactRoller = rnd.rnd(4)
   if contactRoller == 0 and not contactBool then
      krainPilot = pilot.add("Kestrel")
      krainPilot:control()
      krainPilot:hailPlayer()
      krainPilot:follow(pilot.player())
      hook.pilot(krainPilot, "hail", "contactMade")
   end
end

function contactMade()
   contactBool = true
end

function abort ()
   if not missionCargo == nil then
      misn.cargoRm(missionCargo)
   end
   misn.finish(false)
end
