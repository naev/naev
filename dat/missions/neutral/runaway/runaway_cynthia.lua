--[[
This is the "The Runaway" mission as described on the wiki.
There will be more missions to detail how you are percieved as the kidnapper of "Cynthia"
--]]

npc_name = _("Young Teenager")
bar_desc = _("A pretty teenager sits alone at a table.")
title = _("The Runaway")
cargoname = _("Person")
misn_desc_pre_accept = _([[She looks out of place in the bar. As you approach, she seems to stiffen.
    "H..H..Hi", she stutters. "My name is Cynthia. Could you give me a lift? I really need to get out of here.
    I can't pay you much, just what I have on me, %s credits." You wonder who she must be to have this many credits on her person. "I need you to take me to Zhiru."
    You wonder who she is, but you dare not ask. Do you accept?]])
not_enough_cargospace = _("Your cargo hold doesn't have enough free space.")
misn_desc = _("Deliver Cynthia safely to %s in the %s system.")
reward_desc = _("%s credits on delivery.")

post_accept = {}
post_accept[1] = _([["Thank you. But we must leave now, before anyone sees me."]])
misn_accomplished = _([[As you walk into the docking bay, she warns you to look out behind yourself.
    When you look back to where she was, nothing remains but a tidy pile of credit chips and a worthless pendant.]])

osd_text = {}
osd_text[1] = _("Deliver Cynthia to Zhiru in the Goddard system")


function create ()
   startworld, startworld_sys = planet.cur()

   targetworld_sys = system.get("Goddard")
   targetworld = planet.get("Zhiru")

   --if not misn.claim ( {targetworld_sys} ) then
   --   abort()
   --end

   reward = 75000

   misn.setNPC( npc_name, "neutral/miner2" )
   misn.setDesc( bar_desc )
end


function accept ()
   --This mission does not make any system claims
   if not tk.yesno( title, string.format( misn_desc_pre_accept, reward, targetworld:name() ) ) then
      misn.finish()
   end

   --Our *cargo* weighs nothing
   --This will probably cause a mess if this fails
   if player.pilot():cargoFree() < 0 then
      tk.msg( title, not_enough_cargospace )
      misn.finish()
   end

   misn.accept()

   misn.osdCreate(title,osd_text)
   misn.osdActive(1)

   cargoID = misn.cargoAdd( cargoname, 0 )

   misn.setTitle( title )

   misn.setReward( string.format( reward_desc, reward ) )

   misn.setDesc( string.format( misn_desc, targetworld:name(), targetworld_sys:name() ) )
   misn.markerAdd( targetworld_sys, "high")

   tk.msg( title, post_accept[1] )

   hook.land("land")
end

function land ()
  --If we land, check if we're at our destination
   if planet.cur() == targetworld then
      misn.cargoRm( cargoID )
      player.pay( reward )

      tk.msg( title, string.format(misn_accomplished, reward) )

      misn.finish(true)
   end
end

function abort ()
  --Clean up
   misn.cargoRm( cargoID )
   misn.osdDestroy()
   misn.finish( false )
end
