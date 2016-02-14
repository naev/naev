--[[

   Handles the randomly generated Za'lek test missions.
   (Based on the ES lua code)

   stages :
             0 : everything normal
             1 : the player has forgotten the engine
]]

include "dat/scripts/cargo_common.lua"
include "dat/scripts/numstring.lua"

lang = naev.lang()
if lang == "es" then
else -- default english
   misn_desc = "A Za'lek research team need you to travel to %s in %s using an engine in order to test it."
   misn_reward = "%s credits"

   title = [[ZT: go to %s in the %s system
Jumps: %d
Travel distance: %d]]

   msg_title = {}
   msg_title[1] = "Mission Accepted"
   msg_title[2] = "Too many missions"
   msg_title[3] = "Successful Landing"
   msg_title[4] = "Didn't you forget something?"

   engines = {"with phase-change material cooling", 
              "controlled with the Zermatt-Henry theory",   --Some random scientists names
              "using a new electron propelling system",
              "using the fifth law of thermodynamics",      --In these times, there will maybe exist more thermo laws...
              "for a system identification using the fe-method",
              "with uncontrolled geometrical singularities",
              "a 10 years-old child invented himself",
              "using the ancestral propellant technology",
              "built of UHP-nanobond",
              }

   znpcs = {}
   znpcs[1] = [[A group of university students greet you. "If your flight goes well, we will validate our aerospace course! The last engine exploded during the flight, but this one is much more reliable... Hopefully."]]
   znpcs[2] = [[A very old Za'lek researcher needs you to fly with his instrumented device in order to take measurements.]]
   znpcs[3] = [[A Za'lek student says: "Hello, I am preparing a Ph.D in system reliability. I need to make precise measurements on this engine in order to validate a stochastic failure model I developed."]]
   znpcs[4] = [[A Za'lek researcher needs you to test the new propelling system he has implemented in this engine.]]

   msg_msg = {}
   msg_msg[1] = "Za'lek technicians give you the engine. You will have to travel to %s in %s with this engine. The system will automatically take measures during the flight. Don't forget to equip the engine."
   msg_msg[2] = "You have too many active missions."
   msg_msg[3] = "Happy to be still alive, you land and give back the engine to a group of Za'lek scientists who were expecting you. You ask them why they aren't using drones for such missions and they answer that they don't want to risk damaging a drone."
   msg_msg[4] = "It seems, you forgot the engine you are supposed to test... Land again and put it in your ship"
   misst = "Mission failed"
   miss = "You traveled without the engine."

   teleport_title = "What the hell happens there?"
   teleport_text = "You suddenly feel a huge acceleration, as if your ship was going to hyperspace. Then a shock makes you pass out. As you wake up, your ship is damaged and your compass indicates that you are in the %s system!"

   slow_title = "Where is the power gone?"
   slow_text = "The engine doesn't seem to want to propel your ship anymore..."
   speed_title = "Power is back."
   speed_text = "It seems, the engine decided to work properly again."

   outOf_title = "This wasn't supposed to happened"
   outOf_text = "Your ship is totally out of control. Apparently, the experimental engine seems to have some defects."
   noAn_title = "Engine is dead"
   noAn_text = "It seems, the engine doesn't work anymore..."
   baTo_title = "Back to normal"
   baTo_text = "The engine works again."

   osd_title = "Za'lek Test"
   osd_msg = {"Fly to %s in the %s system."}
end

function create()
   -- Note: this mission does not make any system claims.

   origin_p, origin_s = planet.cur()
   local routesys = origin_s
   local routepos = origin_p:pos()

   -- target destination
   destplanet, destsys, numjumps, traveldist, cargo, tier = cargo_calculateRoute()
   if destplanet == nil then
      misn.finish(false)
   end

   --All the mission must go to Za'lek planets with a place to change outfits
   if destplanet:faction() ~= faction.get( "Za'lek" ) or destplanet:services()["outfits"] == nil then
      misn.finish(false)
   end

   -- mission generics
   stuperpx   = 0.3 - 0.015 * tier
   stuperjump = 11000 - 75 * tier
   stupertakeoff = 15000
    
   -- Choose mission reward. This depends on the mission tier.
   finished_mod = 2.0 -- Modifier that should tend towards 1.0 as naev is finished as a game
   jumpreward = 1000
   distreward = 0.15
   reward     = 1.5^tier * (numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())
    
   local typeOfEng = engines[rnd.rnd(1, #engines)]

   misn.setTitle("ZT: test of an engine " .. typeOfEng)
   misn.markerAdd(destsys, "computer")
   misn.setDesc(title:format(destplanet:name(), destsys:name(), numjumps, traveldist ))
   misn.setReward(misn_reward:format(numstring(reward)))

end

function accept()

   if misn.accept() then -- able to accept the mission
      stage = 0
      player.addOutfit("Za'lek Test Engine")
      tk.msg( msg_title[1], znpcs[ rnd.rnd(1, #znpcs) ] )
      tk.msg( msg_title[1], string.format( msg_msg[1], destplanet:name(), destsys:name() ))

      osd_msg[1] = string.format( osd_msg[1], destplanet:name(), destsys:name() )
      misn.osdCreate(osd_title, osd_msg)
      takehook = hook.takeoff( "takeoff" )
      enterhook = hook.enter("enter")
   else
      tk.msg( msg_title[2], msg_msg [2] )
      misn.finish(false)
   end

   isSlow = false     --Flag to know if the pilot has limited speed
   curplanet = planet.cur()
end

function takeoff()  --must trigger at every takeoff to check if the player forgot the engine

   if landhook == nil then
      landhook = hook.land( "land" )
   end

   if isMounted("Za'lek Test Engine") then  --everything is OK : now wait for landing
      stage = 0

      else   --Player has forgotten the engine
      stage = 1
      tk.msg( msg_title[4], msg_msg [4] )
   end
end

function enter()  --Generates a random breakdown
   local luck = rnd.rnd()

   --If the player doesn't have the experimental engine, there can't be any bug
   if not isMounted("Za'lek Test Engine") then
      luck = 0.9
   end

   local time = 10000*(1 + 0.3*rnd.twosigma())
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

   if planet.cur() == destplanet and stage == 0 then
      tk.msg( msg_title[3], msg_msg[3])
      player.pay(reward)
      player.rmOutfit("Za'lek Test Engine")

      -- increase faction
      faction.modPlayerSingle("Za'lek", rnd.rnd(1, 2))
      rmTheOutfit()
      misn.finish(true)
   end

   if planet.cur() ~= curplanet and stage == 1 then  --Lands elsewhere without the engine
      tk.msg( misst, miss)
      abort()
   end

   if isSlow then   --The player is still slow and will recover normal velocity
      player.setSpeed(0)
   end

   curplanet = planet.cur()
end

--  Breakdowns

--Player is teleported in another system
function teleport()
   hook.safe("teleportation")
   hook.rm(enterhook)  --It's enough problem for one travel
end

function teleportation()
   local newsyslist = getsysatdistance(system.cur(), 1, 3)
   local newsys = newsyslist[rnd.rnd(1, #newsyslist)]
   player.teleport(newsys)
   tk.msg(teleport_title, teleport_text:format(newsys:name()))
   player.pilot():setHealth(50, 0)
   player.pilot():setEnergy(0)
end

--player is slowed
function slow()

   -- Cancel autonav.
   player.cinematics(true)
   player.cinematics(false)

   local maxspeed = player.pilot():stats().speed
   local speed = maxspeed/3*(1 + 0.1*rnd.twosigma())
   
   hook.timer(1000, "slowtext")

   isSlow = true

   -- If the player is not too unlucky, the velocity is soon back to normal
   if rnd.rnd() > 0.8 then
      local time = 20000*(1 + 0.3*rnd.twosigma())
      hook.timer(time, "backToNormal")
      isSlow = false
   end

   player.pilot():setSpeedLimit(speed)
end

--Player is no longer slowed
function backToNormal()
   player.pilot():setSpeedLimit(0)
   hook.timer(1000, "speedtext")
end

--Player's ship run amok and behaves randomly
function outOfControl()

   -- Cancel autonav.
   player.cinematics(true)

   player.pilot():control()
   for i = 1, 4, 1 do
      local deltax, deltay = rnd.rnd()*1000, rnd.rnd()*1000
      player.pilot():goto ( player.pilot():pos() + vec2.new( deltax ,deltay ), false, false )
   end
   hook.timer(20000, "backToControl")
   hook.timer(1000, "outOftext")
end

--The player can't control his ship anymore
function noAnswer()

   -- Cancel autonav.
   player.cinematics(true)

   player.pilot():control()
   hook.timer(10000, "backToControl")
   hook.timer(1000, "noAntext")
end

--Just de-control the player's ship
function backToControl()
   player.cinematics(false)
   player.pilot():control(false)
   hook.timer(1000, "baTotext")
end

--Displays texts
function slowtext()
   tk.msg(slow_title, slow_text)
end

function speedtext()
   tk.msg(speed_title, speed_text)
end

function outOftext()
   tk.msg(outOf_title, outOf_text)
end

function noAntext()
   tk.msg(noAn_title, noAn_text)
end

function baTotext()
   tk.msg(baTo_title, baTo_text)
end

function abort()
   rmTheOutfit()
   misn.finish(false)
end

function rmTheOutfit()
   if isMounted("Za'lek Test Engine") then
      player.pilot():rmOutfit("Za'lek Test Engine")
   end
   while isOwned("Za'lek Test Engine") do  --to avoid remaining test engines if some error occurs
      player.rmOutfit("Za'lek Test Engine")
   end
end

--Check if the player has an outfit mounted
function isMounted(itemName)
   for i, j in ipairs(player.pilot():outfits()) do
      if j:name() == itemName then
         return true
      end
   end
   return false
end

--Check if the player owns an outfit
function isOwned(itemName)
   for i, j in ipairs(player.outfits()) do
      if j:name() == itemName then
         return true
      end
   end
   return false
end
