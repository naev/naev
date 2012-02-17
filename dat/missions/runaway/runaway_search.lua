--[[
This is the "The Runaway" mission as described on the wiki.
There will be more missions to detail how you are percieved as the kidnapper of "Cynthia"
--]]

lang = naev.lang()
if lang == "es" then -- Spanish version of the texts would follow
elseif lang == "de" then -- German version of the texts would follow
else -- default English text

npc_name = "Old Man"
bar_desc = "An old man sits at a table with some missing person papers."
title = "The Search for Cynthia"
misn_desc_pre_accept = [[Approaching him, he hands you a paper. It offers a 100,000 credit reward for the finding of a "Cynthia" person.
"That's my girl. She disappeared quite a few STU ago. We managed to track her down to here, but where she went afterwards remains a mystery. We know she was kidnapped, but if you know anything..." The man begins to cry. "Have you seen any trace of her?"]]
misn_desc = "Search for Cynthia."
reward_desc = "%s credits on delivery."

post_accept = {}
post_accept[1] = [[Looking at the picture, you see that the locket matches the one that Cynthia wore, so you hand it to her father. "I believe that this was hers." Stunned, the man hands you a list of planets that they wanted to look for her on.]]

misn_nifiheim = "After throughly searching the spaceport, you decide that she wasn't there."
misn_nova_shakar = "At last! You find her, but she ducks into a tour bus when she sees you. The schedule says it's destined for Torloth"
misn_torloth = "After chasing Cynthia through most of the station, you find her curled up at the end of a hall, crying. She comes on your ship without complaint."
misn_father = "As Cynthia sees her father, she begins her crying anew. You overhear the father talking about how her abusive mother died. Cynthia becomes visibly happier, so you pick up your payment and depart."

osd_text = {}
osd_text[1] = "Search for Cynthia on Nifiheim in Dorihabi"
osd_text[2] = "Search for Cynthia on Nova Shakar in Shakar"
osd_text[3] = "Catch Cynthia on Torloth in Cygnus"
osd_text[4] = "Return to Cynthia's father on Zhiru in the Goddard system."

end


function create ()
   startworld, startworld_sys = planet.cur()

   targetworld_sys = system.get("Dorihabi")
   targetworld = planet.get("Nifiheim")

   reward = 100000
   
   misn.setNPC( npc_name, "neutral/miner2" )
   misn.setDesc( bar_desc )
end


function accept ()
   --This mission does not make any system claims
   if not tk.yesno( title, string.format( misn_desc_pre_accept, reward, targetworld:name() ) ) then
      misn.finish()
   end
   
   if misn.accept() then
      misn.osdCreate(title,osd_text)
      misn.osdActive(1)
   end

   misn.setTitle( title )

   misn.setReward( string.format( reward_desc, reward ) )

   misn.setDesc( string.format( misn_desc, targetworld:name(), targetworld_sys:name() ) )
   runawayMarker = misn.markerAdd( system.get("Nifiheim"), "low")


   misn.accept()

   tk.msg( title, post_accept[1] )


   hook.land("land")
end

function land ()
  
  --If we land on Nifiheim, display message, reset target and carry on.
   if planet.cur() == targetworld and targetworld == targetworld = planet.get("Nifiheim") then
      targetworld = planet.get("Nova Shakar")
      tk.msg(title, misn_nifiheim)
      misn.osdActive(2)
      misn.markerMove(runawayMarker, system.get("Dorihabi"))
   end
   
   --If we land on Nova Shakar, display message, reset target and carry on.
   if planet.cur() == targetworld and targetworld == targetworld = planet.get("Nova Shakar") then
      targetworld = planet.get("Torloth")
      tk.msg(title, misn_nova_shakar)
      misn.osdActive(3)
      misn.markermove(runawayMarker, system.get("Shakar"))
   end
   
   --If we land on Torloth, display message, reset target and carry on.
   if planet.cur() == targetworld and targetworld == targetworld = planet.get("Torloth") then
      targetworld = planet.get("Zhiru")
      tk.msg(title, misn_torloth)
      misn.osdActive(4)
      misn.markermove(runawayMarker, system.get("Goddard"))
   end
   
   --If we land on Zhiru to finish the mission, clean up, reward, and leave.
   if planet.cur() == targetworld and targetworld == targetworld = planet.get("Zhiru") then
      misn.markerRm(runawayMarker)
      tk.msg(title, misn_father)
      player.pay(reward)
      misn.osdDestroy()
      misn.finish(true)
   end
end

function abort ()
  --Clean up
   misn.osdDestroy()
   misn.finish( false )
end