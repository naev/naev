--[[Insane Businessman Part 2]]--



-- localization stuff.
lang = naev.lang()
if lang == "es" then
elseif lang == "de" then
else

--[[Mission text.]]--

npc_name = "A Businessman"
bar_desc = "A disheveled but familiar looking businessman."
title = "Insane Businessman Part 3"
pre_accept = {}

pre_accept[1] = [["Thank you for speaking to me again,"says Crumb. "Don't feel bad you haven't discovered anything. Not all of us can be great pilots." With a stern look on his face, Crumb continues, "Another pilot has achieved what you could not, %s, he has gotten me information about a plot to kill me. That's ok. Your job now is to rendezvous with this pilot and his two associates in the uninhabited %s system. From there all four of you will travel to %s and await further instructions. Look for your rendezvous near the planet %s."]]

pre_accept[2] = [[You think to yourself that you usually work alone.

Do you take the mission anyway?]]

decline = [["Ah, well some other time maybe..."]]

misn_desc = "Fly to the %s system and find the rendezvous. Once you find him, fly to the %s system and await further instructions."
reward_desc = "%s credits on completion."
post_accept = {}
post_accept[1] = [["Rendezvous with your contact in %s then fly to the %s system and await further instructions."]]

misn_accomplished = [[You land back in %s but Crumb is a difficult person to find. He must have fled knowing that you defeated his mercenaries. However he couldn't have gone far...]]

rendezvous_title = [[Rendezvous]]
rendezvous_msg = [["%s! We've been waiting for you." says the voice over the comm. 

"Great. Let's get going."

"In a minute. There's been a slight change of plans. Mr Crumb has decided that your services are no longer necessary. Sorry about this, pal, it's  just business"

click!
]]

-- OSD

OSDtitle1 = "Find your Rendezvous"
OSDdesc11 = "Go to the %s system."
OSDdesc12 = "Find rendezvous."
OSDdesc13 = "Go to the %s system and await orders."

OSDtitle2 = "Confront Crumb"
OSDdesc21 = "Return to the %s system and confront Crumb."

OSDtable1 = {}
OSDtable2 = {}

end


-- Messages
msg = {}
msg[1] = "MISSION FAILED!!!! Don't attack your rendezvous!"
msg[2] = "BUG: Could not claim system."

function create ()

   -- Get the planet and system at which we currently are.
   startworld, startworld_sys = planet.cur()

   -- Set our target system and planet.
   targetworld_sys = system.get("Haven")
   targetworld = planet.get("Haven")
   fake_targetworld_sys = system.get("Alteris")

   if not misn.claim ( {targetworld_sys} ) then
      abort(false,2)
   end

   reward = 10000

   -- Used later to determine all mercenaries were killed
   dead_pilots = 0

   misn.setNPC( npc_name, "neutral/unique/shifty_merchant" )
   misn.setDesc( bar_desc )
end


function accept ()
 
   tk.msg( title, string.format( pre_accept[1], player.name(), targetworld_sys:name(), fake_targetworld_sys:name(), targetworld:name() ) )

   if not tk.yesno( title, pre_accept[2] ) then

      tk.msg( title, decline )

      abort(false,nil)
   end

   misn.setTitle( title )
   misn.setReward( string.format( reward_desc, reward ) )
   misn.setDesc( string.format( misn_desc, targetworld_sys:name(), fake_targetworld_sys:name() ) )

   landmarker = misn.markerAdd( targetworld_sys, "low")

   misn.accept()

   tk.msg( title, string.format( post_accept[1], targetworld_sys:name(), fake_targetworld_sys:name() ) )

   OSDtable1[1] = OSDdesc11:format( targetworld_sys:name() )
   OSDtable1[2] = OSDdesc12
   OSDtable1[3] = OSDdesc13:format( fake_targetworld_sys:name() )
   misn.osdCreate( OSDtitle1, OSDtable1 )
   misn.osdActive(1)


   landhook = hook.land("land")
   jumpouthook = hook.jumpout("jumpout")
   jumpinhook = hook.jumpin("jumpin")
end

function land ()

   -- If you land on start planet and have defeated mercenaries
   if planet.cur() == startworld and victorious then
       
      tk.msg( title,  string.format(misn_accomplished, startworld:name() ) )
      abort(true,nil)   
   end 
 
end

function jumpout()

   -- If you manage to escape you still win.
   if system.cur() == targetworld_sys and hailed then
      victorious = true
   end

   -- If you jump out before accepting hail, osd returns to first point.
   if system.cur() == targetworld_sys and not hailed then
       misn.osdActive(1)
   end

end

-- Moves to the next point on OSD upon jumping into target system
function jumpin ()
   if system.cur() == targetworld_sys and not hailed then
      misn.osdActive(2)
      pilot.clear()
      pilot.toggleSpawn(false)
      spawn_pilots()
      timerhook = hook.timer(3000, "timer")
   end
end

function timer()
   pilot1_pos = pilot1:pos()
   player_pos = player.pilot():pos()
   distance = player_pos:dist(pilot1_pos)
   if distance < 3500 then
      pilot1:hailPlayer()
      hook.rm(timerhook)
      hailing = hook.pilot(pilot1, "hail","hailme")
   else timerhook = hook.timer(3000, "timer")
   end   
end

-- Spawn pilots function
function spawn_pilots()
   
   from_planet = planet.get("Haven")  

   pilot1 = pilot.addRaw( "Admonisher", "mercenary" , from_planet , "Dummy")[1]
   pilot1:rename("Rendezvous")
   pilot1:setHilight( true )
   pilot1:control()
   deathhook1 = hook.pilot( pilot1, "death", "pilot_death" )
   attackhook1 = hook.pilot( pilot1, "attacked", "pilot_attacked")

   pilot2 = pilot.addRaw( "Admonisher", "mercenary" , from_planet , "Dummy")[1]
   pilot2:rename("Rendezvous")
   pilot2:setHilight( true )
   pilot2:control()
   deathhook2 = hook.pilot( pilot2, "death", "pilot_death" ) 
   attackhook2 = hook.pilot( pilot2, "attacked", "pilot_attacked")

   pilot3 = pilot.addRaw( "Admonisher", "mercenary" , from_planet , "Dummy")[1]
   pilot3:rename("Rendezvous")
   pilot3:setHilight( true )
   pilot3:control()
   deathhook3 = hook.pilot( pilots, "death", "pilot_death" )
   attackhook3 = hook.pilot( pilots, "attacked", "pilot_attacked")

end

--If you attack the pilots before answering the hail, they will defend themselves.
--This is also the function that executes after the hail, setting the pilots hostile.
function pilot_attacked()
   if not pilot1:hostile() then

      if pilot1:exists() then
         pilot1:control()
         pilot1:setHostile(true)
         pilot1:attack(player.pilot())
         pilot1:hailPlayer(false)
      end
  
      if pilot2:exists() then
         pilot2:control()
         pilot2:setHostile(true)
         pilot2:attack(player.pilot())
      end
   
      if pilot3:exists() then
         pilot3:control()
         pilot3:setHostile(true)
         pilot3:attack(player.pilot())
      end
      
      hook.rm(attackhook1)
      hook.rm(attackhook2)
      hook.rm(attackhook3)
     
      if not hailed then
         abort(false,1)
      end
   end
end

function hailme()
    player.commClose()
    tk.msg(rendezvous_title, string.format(rendezvous_msg, player.name()))
    hook.rm(hailing)
    hailed = true
    pilot_attacked()
    completeOSDSet()
end

function pilot_death()
   -- Counts how many pilots have been killed
   if dead_pilots < 3 then
      dead_pilots = dead_pilots + 1
   end

   -- If all three pilots killed, you succeed.
   if hailed and dead_pilots == 3 then
      victorious = true
      hook.rm(deathhook1)
      hook.rm(deathhook2)
      hook.rm(deathhook3)   
   end 
end

--This function handles moving the OSD and map marker.
function completeOSDSet()
   misn.osdDestroy ()
   OSDtable2[1] = OSDdesc21:format( startworld_sys:name() )
   misn.osdCreate( OSDtitle2, OSDtable2 )
   misn.markerMove(landmarker, startworld_sys)
end

--Mission fail function
function abort(status,param)
   if param then
      player.msg( msg[param] )
   end

   hooks = { landhook, jumpouthook, jumpinhook, timerhook, hailing, attackhook1, attackhook2, attackhook3, deathhook1, deathhook2, deathhook3 }
   for _, j in ipairs( hooks ) do
      if j ~= nil then
         hook.rm(j)
      end
   end

   misn.finish( status )
   misn.osdDestroy()
end
