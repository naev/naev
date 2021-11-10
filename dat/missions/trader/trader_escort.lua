--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Trader Escort">
 <avail>
  <priority>5</priority>
  <cond>player.numOutfit("Mercenary License") &gt; 0</cond>
  <chance>560</chance>
  <location>Computer</location>
  <faction>Dvaered</faction>
  <faction>Empire</faction>
  <faction>Frontier</faction>
  <faction>Goddard</faction>
  <faction>Independent</faction>
  <faction>Proteron</faction>
  <faction>Sirius</faction>
  <faction>Soromid</faction>
  <faction>Thurion</faction>
  <faction>Traders Guild</faction>
  <faction>Za'lek</faction>
 </avail>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--Escort a convoy of traders to a destination--
local pir = require "common.pirate"
local fleet = require "fleet"
local lmisn = require "lmisn"
local car = require "common.cargo"
local fmt = require "format"
local vntk = require "vntk"


local misn_title = {}
misn_title[1] = _("Escort a tiny convoy to {pnt} in {sys}")
misn_title[2] = _("Escort a small convoy to {pnt} in {sys}")
misn_title[3] = _("Escort a medium convoy to {pnt} in {sys}")
misn_title[4] = _("Escort a large convoy to {pnt} in {sys}")
misn_title[5] = _("Escort a huge convoy to {pnt} in {sys}")

local piracyrisk = {}
piracyrisk[1] = _("#nPiracy Risk:#0 None")
piracyrisk[2] = _("#nPiracy Risk:#0 Low")
piracyrisk[3] = _("#nPiracy Risk:#0 Medium")
piracyrisk[4] = _("#nPiracy Risk:#0 High")

function create()
   --This mission does not make any system claims
   destplanet, destsys, numjumps, traveldist, cargo, avgrisk, tier = car.calculateRoute()

   if destplanet == nil then
      misn.finish(false)
   elseif numjumps == 0 then
      misn.finish(false) -- have to escort them at least one jump!
   elseif avgrisk * numjumps <= 25 then
      misn.finish(false) -- needs to be a little bit of piracy possible along route
   end

   if avgrisk == 0 then
      piracyrisk = piracyrisk[1]
   elseif avgrisk <= 150 then
      piracyrisk = piracyrisk[2]
   elseif avgrisk <= 300 then
      piracyrisk = piracyrisk[3]
   else
      piracyrisk = piracyrisk[4]
   end

   convoysize = rnd.rnd(1,5)

   -- Choose mission reward.
   -- Reward depends on type of cargo hauled. Hauling expensive commodities gives a better deal.
   if convoysize == 1 then
      jumpreward = 6*commodity.price(cargo)
      distreward = math.log(500*commodity.price(cargo))/100
   elseif convoysize == 2 then
      jumpreward = 7*commodity.price(cargo)
      distreward = math.log(700*commodity.price(cargo))/100
   elseif convoysize == 3 then
      jumpreward = 8*commodity.price(cargo)
      distreward = math.log(800*commodity.price(cargo))/100
   elseif convoysize == 4 then
      jumpreward = 9*commodity.price(cargo)
      distreward = math.log(900*commodity.price(cargo))/100
   elseif convoysize == 5 then
      jumpreward = 10*commodity.price(cargo)
      distreward = math.log(1000*commodity.price(cargo))/100
   end
   reward = 2.0 * (avgrisk * numjumps * jumpreward + traveldist * distreward) * (1. + 0.05*rnd.twosigma())

   misn.setTitle( fmt.f( misn_title[convoysize], {pnt=destplanet, sys=destsys} ) )
   car.setDesc( fmt.f(_("A convoy of traders needs protection while they go to {pnt} in {sys}. You must stick with the convoy at all times, waiting to jump or land until the entire convoy has done so."), {pnt=destplanet, sys=destsys} ), cargo, nil, destplanet, nil, piracyrisk )
   misn.markerAdd(destplanet, "computer")
   misn.setReward( fmt.credits(reward) )
end

function accept()
   convoy_ships = {"Rhino", "Mule"}
   convoy_names = {_("Convoy Rhino"), _("Convoy Mule")}
   if convoysize == 1 then
      convoy_n = 3
      convoy_ships = "Llama"
      convoy_names = _("Convoy Llama")
   elseif convoysize == 2 then
      convoy_n = 4
      convoy_ships = "Koala"
      convoy_names = _("Convoy Koala")
   elseif convoysize == 3 then
      convoy_n = 1
      convoy_ships = {"Rhino", "Rhino", "Mule", "Mule", "Mule"}
      convoy_names = {_("Convoy Rhino"), _("Convoy Rhino"), _("Convoy Mule"), _("Convoy Mule"), _("Convoy Mule")}
   elseif convoysize == 4 then
      convoy_n = 3
   elseif convoysize == 5 then
      convoy_n = 4
   end

   if player.jumps() < numjumps then
      if not vntk.yesno( _("Not enough fuel"), fmt.f( _([[The destination is {1} away, but you only have enough fuel for {2}. You cannot stop to refuel. Accept the mission anyway?]]), {fmt.jumps(numjumps), fmt.jumps(player.jumps())} ) ) then
         misn.finish()
      end
   end

   nextsys = lmisn.getNextSystem(system.cur(), destsys) -- This variable holds the system the player is supposed to jump to NEXT.
   origin = planet.cur() -- The place where the AI ships spawn from.

   orig_alive = nil
   alive = nil
   exited = 0
   misnfail = false
   unsafe = false

   misn.accept()
   misn.osdCreate(_("Convey Escort"), {
      fmt.f(_("Escort a convoy of traders to {pnt} in the {sys} system"), {pnt=destplanet, sys=destsys}),
   })

   hook.takeoff("takeoff")
   hook.jumpin("jumpin")
   hook.jumpout("jumpout")
   hook.land("land")
end

function takeoff()
   spawnConvoy()
end

function jumpin()
   if system.cur() ~= nextsys then
      fail( _("MISSION FAILED! You jumped into the wrong system.") )
   else
      spawnConvoy()
   end
end

function jumpout()
   if alive <= 0 or exited <= 0 then
      fail( _("MISSION FAILED! You jumped before the convoy you were escorting.") )
   else
      -- Treat those that didn't exit as dead
      alive = math.min( alive, exited )
   end
   origin = system.cur()
   nextsys = lmisn.getNextSystem(system.cur(), destsys)
end

function land()
   alive = math.min( alive, exited )

   if planet.cur() ~= destplanet then
      vntk.msg(_("You abandoned your mission!"), _("You have landed, abandoning your mission to escort the trading convoy."))
      misn.finish(false)
   elseif alive <= 0 then
      vntk.msg(_("You landed before the convoy!"), _([[You landed at the planet before ensuring that the rest of your convoy was safe. You have abandoned your duties, and failed your mission.]]))
      misn.finish(false)
   else
      if alive >= orig_alive then
         vntk.msg( _("Success!"), _("You successfully escorted the trading convoy to the destination. There wasn't a single casualty and you are rewarded the full amount.") )
         player.pay( reward )
         faction.get("Traders Guild"):modPlayer(1)
      elseif alive / orig_alive >= 0.6 then
         vntk.msg( _("Success with Casualties"), _("You've arrived with the trading convoy more or less intact. Your pay is docked slightly due to the loss of part of the convoy.") )
         player.pay( reward * alive / orig_alive )
      else
         vntk.msg( _("Success with Casualties"), _("You arrive with what's left of the convoy. It's not much, but it's better than nothing. You are paid a steeply discounted amount.") )
         player.pay( reward * alive / orig_alive )
      end
      pir.reputationNormalMission(rnd.rnd(2,3))
      misn.finish( true )
   end
end

function traderDeath()
   alive = alive - 1
   if alive <= 0 then
      fail( _("MISSION FAILED! The convoy you were escorting has been destroyed.") )
   end
end

-- Handle the jumps of convoy.
function traderJump( p, j )
   if j:dest() == lmisn.getNextSystem( system.cur(), destsys ) then
      exited = exited + 1
      if p:exists() then
         player.msg( fmt.f(_("{plt} has jumped to {sys}."), {plt=p, sys=j:dest()} ) )
      end
   else
      traderDeath()
   end
end

--Handle landing of convoy
function traderLand( p, plnt )
   if plnt == destplanet then
      exited = exited + 1
      if p:exists() then
         player.msg( fmt.f(_("{plt} has landed on {pnt}."), {plt=p, pnt=plnt} ) )
      end
   else
      traderDeath()
   end
end


-- Handle the convoy getting attacked.
function traderAttacked( p, _attacker )
   unsafe = true
   p:control( false )
   p:setNoJump( true )
   p:setNoLand( true )

   if not shuttingup then
      shuttingup = true
      p:comm( player.pilot(), _("Convoy ships under attack! Requesting immediate assistance!") )
      hook.timer( 5.0, "traderShutup" ) -- Shuts him up for at least 5s.
   end
end

function traderShutup()
    shuttingup = false
end

function timer_traderSafe()
   hook.timer( 2.0, "timer_traderSafe" )

   if unsafe then
      unsafe = false
      for i, j in ipairs( convoy ) do
         continueToDest( j )
      end
   end
end

function spawnConvoy ()
   --Make it interesting
   local ambush_src = destplanet
   if system.cur() ~= destsys then
      ambush_src = lmisn.getNextSystem( system.cur(), destsys )
   end

   local ambushes = {
      {"Pirate Ancestor", "Pirate Vendetta", "Hyena", "Hyena"},
      {"Pirate Ancestor", "Pirate Vendetta", "Pirate Vendetta", "Pirate Vendetta", "Hyena", "Hyena"},
      {"Pirate Admonisher", "Pirate Rhino", "Pirate Rhino", "Pirate Shark", "Pirate Shark"},
      {"Pirate Admonisher", "Pirate Phalanx", "Pirate Phalanx", "Pirate Shark", "Pirate Shark", "Hyena", "Hyena"},
      {"Pirate Kestrel", "Pirate Admonisher", "Pirate Rhino", "Pirate Shark", "Pirate Shark", "Hyena", "Hyena", "Hyena"},
   }
   if convoysize == 1 then
      ambush = fleet.add( 1, ambushes[1], "Pirate", ambush_src, nil, {ai="baddie_norun"} )
   elseif convoysize == 2 then
      ambush = fleet.add( 1, ambushes[rnd.rnd(1,2)], "Pirate", ambush_src, nil, {ai="baddie_norun"} )
   elseif convoysize == 3 then
      ambush = fleet.add( 1, ambushes[rnd.rnd(2,3)], "Pirate", ambush_src, nil, {ai="baddie_norun"} )
   elseif convoysize == 4 then
      ambush = fleet.add( 1, ambushes[rnd.rnd(2,4)], "Pirate", ambush_src, nil, {ai="baddie_norun"} )
   else
      ambush = fleet.add( 1, ambushes[rnd.rnd(3,5)], "Pirate", ambush_src, nil, {ai="baddie_norun"} )
   end
   for _,p in ipairs(ambush) do
      p:setHostile(true)
   end

   --Spawn the convoy
   convoy = fleet.add( convoy_n,  convoy_ships, "Traders Guild", origin, convoy_names )
   local minspeed = nil
   for i, p in ipairs(convoy) do
      if alive ~= nil and alive < i then
         p:rm()
      end
      if p:exists() then
         for _j, c in ipairs( p:cargoList() ) do
            p:cargoRm( c.name, c.q )
         end
         p:cargoAdd( cargo, p:cargoFree() )

         local myspd = p:stats().speed_max
         if minspeed == nil or myspd < minspeed then
            minspeed = myspd
         end

         p:control()
         p:setHilight(true)
         p:setInvincPlayer()
         continueToDest( p )

         hook.pilot( p, "death", "traderDeath" )
         hook.pilot( p, "attacked", "traderAttacked", p )
         hook.pilot( p, "land", "traderLand" )
         hook.pilot( p, "jump", "traderJump" )
      end
   end

   if minspeed ~= nil then
      for _i, p in ipairs(convoy) do
         if p ~= nil and p:exists() then
            p:setSpeedLimit( minspeed )
         end
      end
   end

   exited = 0
   if orig_alive == nil then
      orig_alive = 0
      for _i, p in ipairs( convoy ) do
         if p ~= nil and p:exists() then
            orig_alive = orig_alive + 1
         end
      end
      alive = orig_alive

      -- Shouldn't happen
      if orig_alive <= 0 then misn.finish(false) end
   end

   hook.timer( 1.0, "timer_traderSafe" )
end

function continueToDest( p )
   if p ~= nil and p:exists() then
      p:control( true )
      p:setNoJump( false )
      p:setNoLand( false )

      if system.cur() == destsys then
         p:land( destplanet, true )
      else
         p:hyperspace( lmisn.getNextSystem( system.cur(), destsys ), true )
      end
   end
end

-- Fail the mission, showing message to the player.
function fail( message )
   if message ~= nil then
      -- Pre-colourized, do nothing.
      if message:find("#") then
         player.msg( message )
      -- Colourize in red.
      else
         player.msg( "#r" .. message .. "#0" )
      end
   end
   misn.finish( false )
end
