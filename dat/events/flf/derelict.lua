--[[
<?xml version='1.0' encoding='utf8'?>
<event name="FLF/DV Derelicts">
 <location>enter</location>
 <chance>60</chance>
 <cond>faction.get("Dvaered"):playerStanding() &gt;= 0 and not (player.misnDone("Take the Dvaered crew home") or player.misnDone("Deal with the FLF agent")) and not (player.misnActive("Deal with the FLF agent") or player.misnActive("Take the Dvaered crew home")) </cond>
 <chapter>[^0]</chapter>
</event>
--]]
--[[
   Derelict Event, spawning either the FLF prelude mission string or the Dvaered anti-FLF string.
--]]

local fmt = require "format"

local boarded, shipDV, shipFLF, timerDV, timerFLF

function create()
   local csys = system.cur()
   local cp = csys:presences()

   -- Must have both Dvaered and FLF presence
   if (cp["Dvaered"] or 0) <= 0 or (cp["FLF"] or 0) <= 0 then
      evt.finish(false)
   end

   -- Must not have inhabited planets
   for _k,p in ipairs(csys:spobs()) do
      local ps = p:services()
      if ps.inhabited then
         evt.finish(false)
      end
   end

   -- Should be somewhere near Raelid
   if csys:jumpDist( system.get("Raelid") ) > 2 then
      evt.finish(false)
   end

   -- Try to claim
   if not evt.claim(csys) then
      evt.finish(false)
   end

   -- Create the derelicts One Dvaered, one FLF.
   pilot.toggleSpawn(false)
   pilot.clear()

   local posDV = vec2.new(7400, 3000)
   local posFLF = vec2.new(-10500, -8500)

   shipDV = pilot.add( "Dvaered Vendetta", "Dvaered", posDV, _("Dvaered Patrol"), {ai="dummy"} )
   shipFLF = pilot.add( "Vendetta", "FLF", posFLF, _("Frontier Patrol"), {ai="dummy"} )

   shipDV:disable()
   shipFLF:disable()

   shipDV:setHilight(true)
   shipFLF:setHilight(true)

   shipDV:setVisplayer()
   shipFLF:setVisplayer()

   timerDV = hook.timer(3.0, "broadcastDV")
   timerFLF = hook.timer(12.0, "broadcastFLF")

   boarded = false

   -- Set a bunch of vars, for no real reason
   var.push("flfbase_sysname", "Sigur") -- Caution: if you change this, change the location for base Sindbad in unidiff.xml as well!

   hook.pilot(shipDV, "board", "boardDV")
   hook.pilot(shipDV, "death", "deathDV")
   hook.pilot(shipFLF, "board", "boardFLF")
   hook.pilot(shipFLF, "death", "deathFLF")
   hook.enter("enter")
end

function broadcastDV()
   -- Ship broadcasts an SOS every 10 seconds, until boarded or destroyed.
   shipDV:broadcast(fmt.f(_("SOS. This is {plt}. Primary systems down. Requesting assistance."), {plt=_("Dvaered Patrol")}), true)
   timerDV = hook.timer(20.0, "broadcastDV")
end

function broadcastFLF()
   -- Ship broadcasts an SOS every 10 seconds, until boarded or destroyed.
   shipFLF:broadcast(fmt.f(_("Calling all ships. This is {plt}. Engines down, ship damaged. Please help."), {plt=_("Frontier Patrol")}), true)
   timerFLF = hook.timer(20.0, "broadcastFLF")
end

function boardFLF()
   if shipDV:exists() then
      shipDV:setHilight(false)
      shipDV:setNoboard(true)
   end
   shipFLF:setHilight(false)
   hook.rm(timerFLF)
   hook.rm(timerDV)
   player.unboard()
   naev.missionStart("Deal with the FLF agent")
   boarded = true
end

function deathDV()
   hook.rm(timerDV)
   if not shipFLF:exists() then
      evt.finish(true)
   end
end

function boardDV()
   if shipFLF:exists() then
      shipFLF:setHilight(false)
      shipFLF:setNoboard(true)
   end
   shipDV:setHilight(false)
   hook.rm(timerDV)
   hook.rm(timerFLF)
   player.unboard()
   naev.missionStart("Take the Dvaered crew home")
   boarded = true
end

function deathFLF()
   hook.rm(timerFLF)
   var.push("flfbase_flfshipkilled", true)
   if not shipDV:exists() then
      evt.finish(true)
   end
end

function enter()
   evt.finish(boarded)
end
