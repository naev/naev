--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Division Marius">
  <trigger>enter</trigger>
  <chance>10</chance>
  <cond>false</cond>
  <!--<cond>player.misnDone("Dvaered Ballet") and not (player.misnDone("Dvaered Base") or player.misnActive("Dvaered Base") or player.misnDone("Dvaered Triathlon"))</cond>-->
  <flags>
   <unique />
  </flags>
  <notes>
   <done_misn name="Dvaered Ballet"/>
   <campaign>Frontier Invasion</campaign>
  </notes>
 </event>
 --]]
--[[
-- Player is attacked by a hitman. If player disables and boards the hitman, this enables the Dvaered Base mission
--]]
-- TODO: this event is neutralized because the "Dvaered Base" mission has not been written yet

local baddie, atthook, jumphook, source_system -- Non-persistent state.
-- luacheck: globals ambusher baddieBoard baddieDead begin leave playerAttacked (Hook functions passed by name)

function create ()
   source_system = system.cur()
   jumphook = hook.jumpin("begin")
   hook.land("leave")
end

function begin ()
   local thissystem = system.cur()

   -- thissystem and source_system must be adjacent (for those who use player.teleport)
   local areAdj = false
   for _,s in ipairs( source_system:adjacentSystems() ) do
      if thissystem == s then areAdj = true end
   end

   if not evt.claim(thissystem) or not areAdj then
      evt.finish(false)
   end

   hook.timer(10.0, "ambusher")
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
   baddie:outfitRm("all")
   baddie:outfitRm("cores")
   baddie:cargoRm("all")

   if vel >= 350 then
      baddie:outfitAdd("S&K Ultralight Combat Plating")
      baddie:outfitAdd("Milspec Aegis 2201 Core System")
      baddie:outfitAdd("Tricon Zephyr Engine")
      baddie:outfitAdd("Reactor Class I")
      baddie:outfitAdd("Razor MK3",3)
      baddie:outfitAdd("Improved Stabiliser")
   elseif vel >= 210 then
      baddie:outfitAdd("S&K Light Combat Plating")
      baddie:outfitAdd("Milspec Aegis 3601 Core System")
      baddie:outfitAdd("Tricon Zephyr II Engine")
      baddie:outfitAdd("Reactor Class I",3)
      baddie:outfitAdd("TeraCom Headhunter Launcher")
      baddie:outfitAdd("Razor MK3",3)
      baddie:outfitAdd("Power Regulation Override",2)
   elseif vel >= 140 then
      baddie:outfitAdd("S&K Light Combat Plating")
      baddie:outfitAdd("Milspec Aegis 3601 Core System")
      baddie:outfitAdd("Tricon Zephyr II Engine")
      baddie:outfitAdd("Small Shield Booster",2)
      baddie:outfitAdd("TeraCom Headhunter Launcher",2)
      baddie:outfitAdd("Razor MK3",2)
      baddie:outfitAdd("Improved Stabiliser")
   else
      baddie:outfitAdd("S&K Light Stealth Plating")
      baddie:outfitAdd("Milspec Aegis 3601 Core System")
      baddie:outfitAdd("Tricon Zephyr II Engine")
      baddie:outfitAdd("Unicorp Caesar IV Launcher",2)
      baddie:outfitAdd("Engine Reroute")
   end

   baddie:setHealth(100,100)
   baddie:setEnergy(100)

   atthook = hook.pilot( player.pilot(), "attacked", "playerAttacked")
   hook.pilot( baddie, "death", "baddieDead")
   hook.pilot( baddie, "board", "baddieBoard")
end

-- Player is attacked: remind what to do
function playerAttacked()
   player.omsgAdd(_("You are being attacked! You may get information from your attacker if you catch them alive!"), 5, 50)
   hook.rm(atthook)
end

-- Ambusher is death: event failed
function baddieDead()
   tk.msg(_("Target destroyed"),_([[While your attacker's ship gets transformed into a fireball, you hesitate between feeling relieved to be alive and disappointed not to have taken the pilot alive. There will however probably be other opportunities to do that.]]))
   evt.finish(false)
end

-- Ambusher was boarded: start the Dvaered Base mission
function baddieBoard()
   tk.msg(_("Boarding aborted"),_([[After you have docked with the hostile ship, your crew combat androids start attacking its airlock with their torches. You receive a message from your attacker:
   "Get away from there! There is a detonator on the airlock! You'll get both of us killed!" At this very moment, a huge explosion illuminates your cockpit. The shock disconnects both ships and dispatches your androids. Your enemy's vessel swerves with a force that breaks its hull apart. You think the pilot is dead, but soon you hear his message:
   "Damn. I thought being transpierced by a spar would hurt more. I guess you're not in a better shape than me. Too bad, none of us will see the Division Marius destroy the Dvaered fleet! Wait, is that my intestine drifting away? Oho! Come back. Hey! You belong to me. Come back into the inside of me, my intestine."]]))
   -- TODO: unboard and destroy the ship
   naev.missionStart("Dvaered Base")
   evt.finish(true)
end

function leave()
   evt.finish(false)
end
