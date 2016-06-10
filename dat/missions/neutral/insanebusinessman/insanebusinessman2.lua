--[[Insane Businessman Part 2]]--

lang = naev.lang()
if lang == "es" then
elseif lang == "de" then
else

--[[Mission Text]]--

npc_name = "A Businessman"
bar_desc = "A disheveled but familiar looking businessman."
title = "Insane Businessman Part 2"
pre_accept = {}

pre_accept[1] = [["Glad you could join me again,"says Crumb. "I think I know who's been feeding me the misinformation."]] 

pre_accept[2] = [["A former associate of mine, Phillip Samson, has been hanging around the %s system. Find him and get him to talk. I know he's involved in the plots against me."]]

pre_accept[3] = [[If there even are plots against him, you can't help but think. Do you accept the mission?]]

decline = [["Ah, well some other time maybe..."]]

misn_desc = "Fly to the %s system and find Samson. Once you find him, interrogate him and get him to talk."
reward_desc = "%s credits on completion."
post_accept = {}
post_accept[1] = [["Find Samson in the %s system and get him to talk. I don't care how, just get me some information."]]

misn_investigate = [[You check out all the local hangouts that Crumb told you Samson patrons. Turning up nothing you think you're completely out of luck when at last fortune shines upon you. A nondescript bartender tells you that he knows Samson. He also tells you Samson knows you're looking for him as someone has tipped him off. He finally says Samson was headed to the spaceport and was planning on leaving the planet and system.]]

misn_accomplished = [[When you tell Crumb what Samson has told you Crumb is enraged. He takes a deep breath and immediately calms down. "Rubbish! Someone is trying to kill me and I'm going to find out who it is! Well, here's your credits."

Oh so now someone is trying to kill him, you can't help but think. Funny how things are progressing. 

"I need you to do one more mission for me. Meet me at the bar," Crumb says, before storming off. 
]]

board_title = [[Samson's Ship]]
board_msg = [[You board Samson's ship. "Please don't hurt me!" Samson yells. 

"I'm not going to hurt you," you reply, "I just want information." 

"What's there to say. Crumb is crazy. We used to be associates. We ran a business together. Selling and buying property. It's just that after..."

"After what?," you inquire.

"There was an electrical fire at one of his hotels. Killed like three people.  The Empire went after him pretty hard for negligence. Ever since then he's progressively gotten more paranoid, alienating his friends and family. We used to be pretty close you know, but now...now, I think he's trying to kill me. Well, you can understand why I ran." 

Strange. Crumb thinks Samson is trying to ruin him, but Samson thinks Crumb sent you to kill him! Better return to Crumb and report this.
]]

-- OSD

OSDtitle1 = "Find Samson."
OSDdesc1 = "Go to the %s system."
OSDdesc2 = "Find Samson."
OSDdesc3 = "Catch Samson before he gets away."
OSDdesc4 = "Return to %s and report to Crumb."

OSDtitle2 = "Catch Samson before he gets away."
OSDtable1 = {}
OSDtable2 = {}

OSDtitle3 = "Report to Crumb."
OSDtable3 = {}

-- Messages

msg    = {}
msg[1] = "MISSION FAILURE! Samson got away."
msg[2] = "MISSION FAILURE! You killed Samson."

end

function create ()

   startworld, startworld_sys = planet.cur()

   -- Set our target system and planet. Picks a random planet target system
   targetworld_sys = system.get("Delta Pavonis")
   targetworlds = {}
   i = 0
   for key, planet in ipairs( targetworld_sys:planets() ) do
      if planet:canLand() then
         i = i + 1
         targetworlds[i] = planet  
      end 
   end   

   targetworld_number = rnd.rnd(1,i)
   targetworld = targetworlds[targetworld_number]

   if not misn.claim ( {targetworld_sys} ) then
      misn.finish()
   end

   reward = 10000

   misn.setNPC( npc_name, "neutral/unique/shifty_merchant" )
   misn.setDesc( bar_desc )
end


function accept ()
  
   -- Some background mission text
   tk.msg( title, pre_accept[1] )
   tk.msg( title, string.format( pre_accept[2], targetworld_sys:name() ) )

   if not tk.yesno( title, string.format( pre_accept[3], targetworld:name() ) ) then

      tk.msg( title, decline )

      misn.finish()
   end

   -- Onboard Computer 
   misn.setTitle( title )
   misn.setReward( string.format( reward_desc, reward ) )
   misn.setDesc( string.format( misn_desc, targetworld_sys:name() ) )

   -- Marker
   landmarker = misn.markerAdd( targetworld_sys, "low")

   misn.accept()

   tk.msg( title, string.format( post_accept[1], targetworld_sys:name() ) )

   -- Set OSD after mission accept
   OSDtable1[1] = OSDdesc1:format( targetworld_sys:name() )
   OSDtable1[2] = OSDdesc2
   misn.osdCreate( OSDtitle1, OSDtable1 )
   misn.osdActive(1)


-- Uncomment  this line to reveal the planet Samson is in.
--   tk.msg( title, targetworld:name() )

   hook.land("land")
   hook.takeoff("takeoff")
   hook.jumpin("jumpin")
end

function land ()
   -- Are we at our destination and did we find Samson?
   if planet.cur() == startworld and interrogated then
      
      player.pay( reward )
      tk.msg( title, string.format(misn_accomplished) )
      misn.finish(true)

   end
   
    -- First time we land on secret planet. Samson escapes..
   if planet.cur() == targetworld and not found then
      tk.msg( title, string.format(misn_investigate) )
      found = true
   end
    
   -- If the pilot has spawned and the player lands on a planet before boarding it, the mission fails. 
   if found and spawned and not interrogated then
      fail(1)
   end

end

function takeoff ()
   
   -- Make sure player landed on planet but pilot has not spawned yet. Set OSD. Spawn pilot. 
   if system.cur() == targetworld_sys and found and not spawned then
      OSDtable2[1] = OSDdesc3
      misn.osdDestroy()
      misn.osdCreate( OSDtitle2, OSDtable2 )
      spawn_pilot()
      spawned = true
   end
end

-- Moves to the next point on OSD upon jumping into target system
function jumpin ()
   if system.cur() == targetworld_sys and not found then
      misn.osdActive(2)
   end

   -- Player has landed on planet and pilot has spawned. If player leaves system, mission fails. 
   if found and spawned and not interrogated then
      fail(1) 
   end
      
end

-- Spawn pilot Samson function
function spawn_pilot()
   samson = pilot.addRaw( "Gawain", "mercenary" , targetworld , "Dummy")[1]
   samson:rename("Samson")
   samson:control()
   samson:runaway( player.pilot(), false )
   samson:setHilight( true )
   hook.pilot( samson, "jump", "pilot_jump" )
   hook.pilot( samson, "death", "pilot_death" )
   hook.pilot( samson, "board", "pilot_board")
end


--Executes when you disable and board the ship
function pilot_board()
   player.unboard()
   tk.msg(board_title, board_msg)
   OSDtable3[1] = OSDdesc4:format( startworld_sys:name() )
   misn.osdCreate( OSDtitle3, OSDtable3 )   
   interrogated = true
   misn.markerMove(landmarker, startworld_sys)
end

--Executes if pilot jumps out of system
function pilot_jump ()
   if not interrogated then
      fail(1)
   end
end

--Executes if you kill him before boarding
function pilot_death()
   if not interrogated then
      fail(2)
   end 
end

--Mission fail function.
function fail(param)
   player.msg( msg[param] )
   misn.finish( false )
end
