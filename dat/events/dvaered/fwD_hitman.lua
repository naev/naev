--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Division Marius">
  <trigger>enter</trigger>
  <chance>10</chance>
  <cond>player.misnDone("Dvaered Ballet") == true and not (player.misnDone("Dvaered Base") or player.misnActive("Dvaered Base") or player.misnDone("Dvaered Triathlon"))</cond>
  <flags>
   <unique />
  </flags>
  <notes>
   <done_misn name="Dvaered Triathlon"/>
   <campaign>Frontier Invasion</campaign>
  </notes>
 </event>
 --]]
--[[
-- Player is attacked by a hitman. If player disables and boards the hitman, this enables the Dvaered Base mission
--]]
-- TODO: this event is neutralized because the "Dvaered Base" mission has not been written yet
attk_warn = _("You are being attacked: try to catch your attacker alive!")

death_title = _("Target destroyed")
death_text = _([[While your attacker's ship gets transformed into a fireball, you hesitate between feeling relieved to be alive and disappointed not to have taken the pilot alive. There will however probably be other opportunities to do that.]])

board_title = _("Boarding aborted")
board_text = _([[After you have docked with the hostile ship, your crew combat androids start attacking its airlock with their torches. You recieve a message from your attacker:
   "Get away from there! There is a detonator on the airlock! You'll get both of us killed!" At this very moment, a huge explosion illuminates your cockpit. The shock disconnects both ships and dispatches your androids. Your enemy's vessel swerves with a force that breaks its hull apart. You think the pilot is dead, but soon you hear his message:
   "Damn. I thought being transpierced by a spar would hurt more. I guess you're not in a better shape than me. Too bad, none of us will see the Division Marius destroy the Dvaered fleet! Wait, is that my intestine drifting away? Oho! Come back. Hey! You belong to me. Come back into the inside of me, my intestine."]])

function create ()
   source_system = system.cur()
   jumphook = hook.jumpin("begin")
   landhook = hook.land("leave")
end

function begin ()  
   thissystem = system.cur()

   -- thissystem and source_system must be adjacent (for those who use player.teleport)
   areAdj = false
   for _,s in ipairs( source_system:adjacentSystems() ) do
      if thissystem == s then areAdj = true end
   end

   if not evt.claim(thissystem) or not areAdj then
      evt.finish(false)
   end

   hook.timer(10000, "ambusher")
   hook.rm(jumphook)
   hook.jumpout("leave")
end

-- Spawn the ambusher, depending on the player's max velocity
function ambusher()
   local vel = player.pilot():stats().speed_max

   -- TODO: ajust requirements
   if vel >= 350 then
      baddie = pilot.add( "Hyena", "Mercenary", source_system, _("Mercenary") )
   elseif vel >= 210 then
      baddie = pilot.add( "Lancelot", "Mercenary", source_system, _("Mercenary") )
   else
      baddie = pilot.add( "Ancestor", "Mercenary", source_system, _("Mercenary") )
   end

   baddie:setHostile()
   baddie:rmOutfit("all")
   baddie:rmOutfit("cores")
   baddie:cargoRm("__all")

   if vel >= 350 then
      baddie:addOutfit("S&K Ultralight Combat Plating")
      baddie:addOutfit("Milspec Aegis 2201 Core System")
      baddie:addOutfit("Tricon Zephyr Engine")
      baddie:addOutfit("Reactor Class I")
      baddie:addOutfit("Razor MK3",3)
      baddie:addOutfit("Improved Stabilizer")
   elseif vel >= 210 then
      baddie:addOutfit("S&K Light Combat Plating")
      baddie:addOutfit("Milspec Aegis 3601 Core System")
      baddie:addOutfit("Tricon Zephyr II Engine")
      baddie:addOutfit("Reactor Class I",3)
      baddie:addOutfit("TeraCom Headhunter Launcher")
      baddie:addOutfit("Razor MK3",3)
      baddie:addOutfit("Power Regulation Override",2)
   elseif vel >= 140 then
      baddie:addOutfit("S&K Light Combat Plating")
      baddie:addOutfit("Milspec Aegis 3601 Core System")
      baddie:addOutfit("Tricon Zephyr II Engine")
      baddie:addOutfit("Small Shield Booster",2)
      baddie:addOutfit("TeraCom Headhunter Launcher",2)
      baddie:addOutfit("Razor MK3",2)
      baddie:addOutfit("Improved Stabilizer")
   else
      baddie:addOutfit("S&K Light Stealth Plating")
      baddie:addOutfit("Milspec Aegis 3601 Core System")
      baddie:addOutfit("Tricon Zephyr II Engine")
      baddie:addOutfit("Unicorp Caesar IV Launcher",2)
      baddie:addOutfit("Engine Reroute")
   end

   baddie:setHealth(100,100)
   baddie:setEnergy(100)

   atthook = hook.pilot( player.pilot(), "attacked", "playerAttacked")
   hook.pilot( baddie, "death", "baddieDead")
   hook.pilot( baddie, "board", "baddieBoard")
end

-- Player is attacked: remind what to do
function playerAttacked()
   omsg = player.omsgAdd(attk_warn, 5, 50)
   hook.rm(atthook)
end

-- Ambusher is death: event failed
function baddieDead()
   tk.msg(death_title,death_text)
   evt.finish(false)
end

-- Ambusher was boarded: start the Dvaered Base mission
function baddieBoard()
   tk.msg(board_title,board_text)
   -- TODO: unboard and destroy the ship
   naev.missionStart("Dvaered Base")
   evt.finish(true)
end

function leave()
   evt.finish(false)
end
