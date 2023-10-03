--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Test">
 <priority>3</priority>
 <cond>
   if faction.playerStanding("Za'lek") &lt; 0 then
      return false
   end
   if spob.cur():services()["outfits"] == nil then
      return false
   end
   return require("misn_test").computer()
 </cond>
 <chance>450</chance>
 <location>Computer</location>
 <faction>Za'lek</faction>
 <notes>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[
   Handles the randomly generated Za'lek test missions.
   (Based on the ES lua code)

   stages :
             0 : everything normal
             1 : the player has forgotten the engine
]]
local car = require "common.cargo"
local fmt = require "format"
local lmisn = require "lmisn"
local zlk = require "common.zalek"
local vntk = require "vntk"

local isMounted, isOwned, rmTheOutfit -- Forward-declared functions

local engines = {
   _("engine with phase-change material cooling"),
   _("engine controlled with Zermatt-Henry theory"), -- Some random scientists names
   _("engine using a new electron propelling system"),
   _("engine using the fifth law of thermodynamics"), -- In these times, there will maybe exist more thermo laws...
   _("engine for system identification using the fe-method"),
   _("engine with uncontrolled geometrical singularities"),
   _("5-cycle-old child's engine invention"),
   _("engine using ancestral propellant technology"),
   _("UHP-nanobond engine"),
   _("ancient engine discovered at an old crash site"),
   _("engine controlled by the MegaSys Wondigs operating system"),
   _("engine controlled by XF Cell-Ethyrial particles"),
   _("engine with ancient Beeline Technology axis"),
   _("unnamed engine prototype"),
   _("engine constructed with experimental Bob-Bens technology"),
   _("new IDS-1024 experimental engine design"),
   _("engine with new Dili-Gent Circle conduction"),
   _("engine with experimental DEI-Z controller"),
}

local znpcs = {
   _([[A group of university students greets you. "If your flight goes well, we will validate our aerospace course! The last engine exploded during the flight, but this one is much more reliable… Hopefully."]]),
   _([[A very old Za'lek researcher needs you to fly with an instrumented device in order to take measurements.]]),
   _([[A Za'lek student says: "Hello, I am preparing a Ph.D in system reliability. I need to make precise measurements on this engine in order to validate a stochastic failure model I developed."]]),
   _([[A Za'lek researcher needs you to test the new propulsion system they have implemented in this engine.]]),
}

function create()
   mem.origin_p, mem.origin_s = spob.cur()

   -- target destination
   mem.destplanet, mem.destsys, mem.numjumps, mem.traveldist, mem.cargo, mem.avgrisk, mem.tier = car.calculateRoute()
   if mem.destplanet == nil then
      misn.finish(false)
   end

   --All the mission must go to Za'lek planets with a place to change outfits
   if mem.destplanet:faction() ~= faction.get( "Za'lek" ) or not mem.destplanet:services()["outfits"] then
      misn.finish(false)
   end

   -- Choose mission reward. This depends on the mission tier.
   local jumpreward = 1500
   local distreward = 0.30
   local riskreward = 50
   mem.reward     = (1.5 ^ mem.tier) * (mem.avgrisk*riskreward + mem.numjumps * jumpreward + mem.traveldist * distreward) * (1. + 0.05*rnd.twosigma())

   local typeOfEng = engines[rnd.rnd(1, #engines)]

   misn.setTitle( fmt.f( zlk.prefix.._("Test of {engine}"), {engine=typeOfEng} ))
   misn.markerAdd(mem.destplanet, "computer")
   car.setDesc( fmt.f(_("A Za'lek research team needs you to travel to {pnt} in {sys} using an engine in order to test it."), {pnt=mem.destplanet, sys=mem.destsys} ), nil, nil, mem.destplanet )
   misn.setReward(mem.reward)
end

function accept()
   if player.misnActive("Za'lek Test") then
       vntk.msg(_("You cannot accept this mission"), _("You are already testing another engine."))
       return
   end

   misn.accept()

   mem.stage = 0
   player.outfitAdd("Za'lek Test Engine")
   vntk.msg( _("Mission Accepted"), {
      znpcs[ rnd.rnd(1, #znpcs) ],
      fmt.f( _("Za'lek technicians give you the engine. You will have to travel to {pnt} in {sys} with this engine. The system will automatically take measurements during the flight. Don't forget to equip the engine."),
         {pnt=mem.destplanet, sys=mem.destsys} ),
   } )

   misn.osdCreate(_("Za'lek Test"), {fmt.f(_("Fly to {pnt} in the {sys} system"), {pnt=mem.destplanet, sys=mem.destsys})})
   mem.takehook = hook.takeoff( "takeoff" )
   mem.enterhook = hook.enter("enter")

   mem.isSlow = false     --Flag to know if the pilot has limited speed
   mem.curplanet = spob.cur()
end

function takeoff()  --must trigger at every takeoff to check if the player forgot the engine

   if mem.landhook == nil then
      mem.landhook = hook.land( "land" )
   end

   if isMounted("Za'lek Test Engine") then  --everything is OK : now wait for landing
      mem.stage = 0

      else   --Player has forgotten the engine
      mem.stage = 1
      vntk.msg( _("Didn't you forget something?"), _("It seems you forgot the engine you are supposed to test. Land again and put it on your ship.") )
   end
end

function enter()  --Generates a random breakdown
   local luck = rnd.rnd()

   --If the player doesn't have the experimental engine, there can't be any bug
   if not isMounted("Za'lek Test Engine") then
      luck = 0.9
   end

   local time = 10.0*(1 + 0.3*rnd.twosigma())
   if luck < 0.06 then  --player is teleported to a random place around the current system
      hook.timer(time, "teleport")
   elseif luck < 0.12 then  --player's ship slows down a lot
      hook.timer(time, "slow")
   elseif luck < 0.18 then   --ship gets out of control for some time
      hook.timer(time, "outOfControl")
   elseif luck < 0.24 then   --ship doesn't answer to command for some time
      hook.timer(time, "noAnswer")
   end

end

function land()

   if mem.isSlow then   --The player is still slow and will recover normal velocity
      player.pilot():setSpeedLimit(0)
      mem.isSlow = false
   end

   if spob.cur() == mem.destplanet and mem.stage == 0 then
      player.pay(mem.reward)

      lmisn.sfxVictory()
      vntk.msg( _("Successful Landing"), _([[Happy to be still alive, you land.  An excited group of Za'lek scientists quickly remove the experimental engine and eagerly download your flight data before paying you your fee.]]).."\n\n"..fmt.reward(mem.reward))
      player.outfitRm("Za'lek Test Engine")

      -- increase faction
      faction.modPlayerSingle("Za'lek", rnd.rnd(1, 2))
      rmTheOutfit()
      misn.finish(true)
   end

   if spob.cur() ~= mem.curplanet and mem.stage == 1 then  --Lands elsewhere without the engine
      vntk.msg( _("Mission failed"), _("You traveled without the engine."))
      abort()
   end

   mem.curplanet = spob.cur()
end

--  Breakdowns

--Player is teleported in another system
function teleport()
   hook.safe("teleportation")
   hook.rm(mem.enterhook)  --It's enough problem for one travel
end

function teleportation()
   local newsyslist = lmisn.getSysAtDistance(system.cur(), 1, 3)
   local newsys = newsyslist[rnd.rnd(1, #newsyslist)]
   player.teleport(newsys)
   vntk.msg(_("What the hell is happening?"), fmt.f(_("You suddenly feel a huge acceleration, as if your ship was going into hyperspace, then a sudden shock causes you to pass out. As you wake up, you find that your ship is damaged and you have ended up somewhere in the {sys} system!"), {sys=newsys}))
   player.pilot():setHealth(50, 0)
   player.pilot():setEnergy(0)
end

--player is slowed
function slow()

   -- Cancel autonav.
   player.autonavAbort()
   camera.shake()
   audio.soundPlay( "empexplode" )

   local maxspeed = player.pilot():stats().speed
   local speed = maxspeed/3*(1 + 0.1*rnd.twosigma())

   hook.timer(1.0, "slowtext")

   mem.isSlow = true

   -- If the player is not too unlucky, the velocity is soon back to normal
   if rnd.rnd() > 0.8 then
      local time = 20.0*(1 + 0.3*rnd.twosigma())
      hook.timer(time, "backToNormal")
      mem.isSlow = false
   end

   player.pilot():setSpeedLimit(speed)
end

--Player is no longer slowed
function backToNormal()
   player.pilot():setSpeedLimit(0)
   hook.timer(1.0, "speedtext")
end

--Player's ship run amok and behaves randomly
function outOfControl()
   -- Cancel autonav.
   player.autonavAbort()
   camera.shake()

   player.pilot():control()
   for i = 1, 4, 1 do
      local deltax, deltay = rnd.rnd()*1000, rnd.rnd()*1000
      player.pilot():moveto ( player.pos() + vec2.new( deltax, deltay ), false, false )
   end
   hook.timer(20.0, "backToControl")
   hook.timer(1.0, "outOftext")
end

--The player can't control his ship anymore
function noAnswer()
   -- Cancel autonav.
   player.autonavAbort()
   camera.shake()

   player.pilot():control()
   hook.timer(10.0, "backToControl")
   hook.timer(1.0, "noAntext")
end

--Just de-control the player's ship
function backToControl()
   player.pilot():control(false)
   hook.timer(1.0, "baTotext")
end

--Displays texts
function slowtext()
   vntk.msg(_("Where has the power gone?"), _("The engine makes a loud noise, and you notice that the engine has lost its ability to accelerate at the rate that it's supposed to."))
end

function speedtext()
   vntk.msg(_("Power is back."), _("It seems the engine decided to work properly again."))
end

function outOftext()
   vntk.msg(_("This wasn't supposed to happen"), _("Your ship is totally out of control. You curse under your breath at the defective engine."))
end

function noAntext()
   vntk.msg(_("Engine is dead"), _("The engine has stopped working. It had better start working again soon; you don't want to die out here!"))
end

function baTotext()
   vntk.msg(_("Back to normal"), _("The engine is working again. You breathe a sigh of relief."))
end

function abort()
   rmTheOutfit( true )
   misn.finish(false)
end

function rmTheOutfit( addengine )
   local pp = player.pilot()
   if isMounted("Za'lek Test Engine") then
      pp:outfitRm("Za'lek Test Engine")
   end

   -- Give them a bad engine just in case they are landed or on a planet
   -- where they can't equip. The bad engine is free so it shouldn't matter.
   if addengine then
      pp:outfitAdd("Beat Up Small Engine")
   end

   -- TODO Remove copies from all ships
   --[[
   for k,v in ipairs(player.ships()) do
      local o = player.shipOutfits(v.name)
      if isMounted("Za'lek Test Engine", o) then
      end
   end
   --]]

   -- Remove owned copies
   while isOwned("Za'lek Test Engine") do
      player.outfitRm("Za'lek Test Engine")
   end
end

--Check if the player has an outfit mounted
function isMounted( itemName, outfits )
   outfits = outfits or player.pilot():outfitsList()
   for i, j in ipairs(outfits) do
      if j == outfit.get(itemName) then
         return true
      end
   end
   return false
end

--Check if the player owns an outfit
function isOwned(itemName)
   for i, j in ipairs(player.outfits()) do
      if j == outfit.get(itemName) then
         return true
      end
   end
   return false
end
