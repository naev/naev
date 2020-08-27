--[[Insane Businessman Part 2]]--

lang = naev.lang()
if lang == "es" then
elseif lang == "de" then
else

--[[Mission Text]]--

npc_name = "Mr. Crumb"
bar_desc = "Mr. Crumb, the notorious businessman."
title = "Insane Businessman Part 2"
pre_accept = {}

pre_accept[1] = [["Glad you could join me again,"says Crumb. "I'm gonna need you to do some more poking around."]] 

pre_accept[2] = [["A former associate of mine, Phillip Samson, has been hanging around the %s system. I don't know what planet, that's something for you to find out. Find him and get him to talk. I know he's involved in the plots against me. Oh, and be sure to equip your ship with disabling weapons. He might get the drop on you and try to run. Samson knows I'm looking for him. But whatever you do, don't kill him, at least until he's talked he, he, he."]]

pre_accept[3] = [["Does this sound like something you could do?" Accept the mission?]]

decline = [["Ah, well some other time maybe..."]]

misn_desc = "Fly to the %s system and find Samson. Once you find him, interrogate him and get him to talk."
reward_desc = "%s credits on completion."
post_accept = {}
post_accept[1] = [["Good, then find Samson in the %s system and get him to talk. I don't care how, just get me some real information."]]

misn_investigate = [[You check out all the local hangouts but turn up nothing. You think you're completely out of luck when at last fortune shines upon you. You're talking to one of your contacts when a man in a suit overhears you. 

"Excuse me," he interrupts, "I know Mr. Samson. We ran a hedgefund together some years ago. Just had lunch with him as a matter of fact."

"You do? You did?" The words escape you before you can control yourself. Regaining your composure, you continue,"Tell me, where can I find him?"

"Oh, you can't. He's leaving the planet. Told me someone was looking for him. What a character." The man chuckles not realizing you represent Mr. Crumb, and you both surely know Mr. Crumb means business.

You rush over to the spaceport where you come accross some more good luck. You see a ship take off that matches the description of Samson's ship. You run the identification number through the database and confirm it is Samson. Catch Samson before he escapes!]]

misn_accomplished = [[Once again, Crumb impatiently waits for you at the spaceport. "Well, what did you find out?" He asks, practically the moment you step off your ship.

"Nothing," you say, "Apparently Samson knew nothing of a plot against you." 

"Nothing?! Did you talk to him? What did he say?"

"Well," you think twice about telling him what Samson said, but Crumb implores you. 

"Tell me! I'm not paying you for nothing." It is true, Crumb is paying you to report on Samson. You may as well tell him the truth. 

"Samson said there probably are no plots against you. He said you are paranoid."

"Paranoid! That good for nothing, worthless, son of a...well whatever. Here are your credits. Meet me at the bar if you're interested in helping me uncover this plot to kill me."
]]

board_title = [[Samson's Ship]]
board_msg = [[You board Samson's ship and find him cowering in a corner, fumbling with a blaster gun. "Please don't hurt me!" Samson pleads as he drops the blaster and raises his hands. 

"I'm not going to hurt you," you reply, "I just want information." 

"You want information? For Crumb? What can I say, I don't know anything. Crumb is crazy, he's paranoid!"

"Go on," you demand.

"Well, we used to be partners, friends even. Everything changed when an electrical fire killed three people in one of his hotels. After that, Crumb got paranoid and untrusting. It got even worse when the Empire investigated him for negligence. Now he sends people out to intimidate former associates, no offense." 

"None taken," you reply and for some reason, you feel Samson's story is credible, at least if not more credible than Crumb's own conspiracies. Concluding that there is not much else to gain from this conversation you return to your ship wondering what it is you are going to tell Crumb.]]

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

   landhook = hook.land("land")
   takeoffhook = hook.takeoff("takeoff")
   jumpinhook = hook.jumpin("jumpin")
end

function land ()
   -- Are we at our destination and did we find Samson?
   if planet.cur() == startworld and interrogated then
      player.pay( reward )
      tk.msg( title, string.format(misn_accomplished) )
      abort(true,nil)
   end
   
    -- First time we land on secret planet. Samson escapes..
   if planet.cur() == targetworld and not found then
      tk.msg( title, string.format(misn_investigate) )
      found = true
      player.takeoff()      
   end
    
   -- If the pilot has spawned and the player lands on a planet before boarding it, the mission fails. 
   if found and spawned and not interrogated then
      abort(true,1)
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
      hook.rm(takeoffhook)
   end
end

-- Moves to the next point on OSD upon jumping into target system
function jumpin ()
   if system.cur() == targetworld_sys and not found then
      misn.osdActive(2)
   end

   -- Player has landed on planet and pilot has spawned. If player leaves system, mission fails. 
   if found and spawned and not interrogated then
      abort(false,1)
   end
      
end

-- Spawn pilot Samson function
function spawn_pilot()
   samson = pilot.addRaw( "Gawain", "mercenary" , targetworld , "Dummy")[1]
   samson:rename("Samson")
   samson:control()
   samson:runaway( player.pilot(), false )
   samson:setHilight( true )
   pilotjumphook = hook.pilot( samson, "jump", "pilot_jump" )
   pilotdeathhook = hook.pilot( samson, "death", "pilot_death" )
   pilotboardhook = hook.pilot( samson, "board", "pilot_board")
end


--Executes when you disable and board the ship
function pilot_board()
   player.unboard()
   tk.msg(board_title, board_msg)
   OSDtable3[1] = OSDdesc4:format( startworld_sys:name() )
   misn.osdCreate( OSDtitle3, OSDtable3 )   
   interrogated = true
   misn.markerMove(landmarker, startworld_sys)
   hook.rm(pilotboardhook)
end

--Executes if pilot jumps out of system
function pilot_jump ()
   if not interrogated then
      abort(false,1)
   end
end

--Executes if you kill him before boarding
function pilot_death()
   if not interrogated then
      abort(false,2)
   end 
end

--Mission fail function.
function abort(status,param)
   
   hooks = { landhook, takeoffhook, jumpinhook, pilotjumphook, pilotdeathhook, pilotboardhook }
   for _, j in ipairs( hooks ) do
      if j ~= nil then
         hook.rm(j)
      end
   end

   if param then
      player.msg( msg[param] )
   end
   misn.finish( status)
   misn.osdDestroy()
end
