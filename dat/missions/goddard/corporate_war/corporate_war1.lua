--[[

Corporate War mission 1.
The player needs to head to Za'lek space to pickup a shipment, and then deliver that shipment back to Goddard space.
start planet = Zhiru in the Goddard system.
pickup planet = Topkapi Station in the Damien system.
end planet = Zhiru in the Goddard system.

--]]

-- This section stores the strings (text) for the mission.

-- Bar information, describes how the person appears in the bar
   bar_name = "An oddly dressed man."
   bar_desc = "A man sits at the counter, looking at the other patrons cooly."

-- Mission details. We store some text for the mission with specific variables.
   misn_title = "Goddard Delivery Mission" 
   misn_desc = "Pickup and deliver a shipment from Za'lek space bound for the Goddard corporation."

   bmsg = {} --beginning messages
   pmsg = {} --pickup messages
   emsg = {} --ending messages
   fmsg = {} --failure messages
   bmsg[1] = [[You approach the man, who grins as he sees you. "Ah, you there! Might you be interested in a job?" You sit down next to him as he continues, "I'm Eryk, I work for House Goddard, and we have a shipment we need picked up from a Za'lek station and delivered back here. Would you be interested?"]]     --Use double brackets [[]] for block quotes over several lines.

   bmsg[2] = [[Eryk grins again as he says, "Fantastic! We need you to fly to %s in the %s system, and then return to %s. The sooner the better, but we know it's a long trip. Will give you %d credits when you get back." He gets up and quickly mutters something about having to use the facilities before disappearing out of the bar.]] --pickupAsset:name(), pickupSys:name(), returnAsset:name(), misn_reward

   pmsg[1] = [[You barely have time to land before some Za'lek are pushing crates onto your ship. Before long, one of them comes up to you to tell you that the crates are all loaded, and the ship is ready to leave when you are.]]

   fmsg[1] = [[Eryk appears dismayed, but nods anyways. "Fair enough. If you change your mind, I should be hanging around here somewhere." He gets up from the counter and mutters something about having to use the facilities before disappearing.]] --player said no in the bar
   fmsg[2] = [[One of the dockworkers come up to you to inform you that you don't have enough cargo space, and as such they can't load the components on board. He dryly recommends you get more cargo space.]] --player does not have enough cargo
   
   emsg[1] = [[You land on %s, only to find Eryk already waiting for you. He grins as you approach. "Glad to see you again, friend! We'll get that cargo out in no time, and credits into your hand." He turns to look at your ship. "If you wouldn't mind meeting me in the bar soon, I may be able to put together another courier run for you if your interested." Eryk shakes your hand before disappearing.]]

   osd = {}
   osd[1] = "Land on %s in the %s system."
   osd[2] = "Return with the shipment to %s in the %s system."

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
   
   misn.accept()  -- For missions from the Bar only.
   tk.msg( misn_title, bmsg[2] )
   misn.setTitle( misn_title)
   misn.setReward( misn_reward)
   misn.setDesc( misn_desc)
   misn.markerAdd(system.get("Damien"),"high") --change as appropriate to point to a system object and marker style.

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

function abort ()
   if not missionCargo == nil then
      misn.cargoRm(missionCargo)
   end
   misn.finish(false)
end
