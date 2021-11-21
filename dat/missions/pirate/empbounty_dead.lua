--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Empire Patrol Bounty">
 <avail>
  <priority>3</priority>
  <cond>player.numOutfit("Mercenary License") &gt; 0 or planet.cur():blackmarket()</cond>
  <chance>1260</chance>
  <location>Computer</location>
  <faction>Wild Ones</faction>
  <faction>Black Lotus</faction>
  <faction>Raven Clan</faction>
  <faction>Dreamer Clan</faction>
  <faction>Pirate</faction>
  <faction>Independent</faction>
  <faction>FLF</faction>
 </avail>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   Pirate Empire bounty

   Based on the pirate bounty mission (hence the weird terminology in
   many variable names). Replacement for the old "Pirate Empire bounty"
   mission.

   Unlike Pirate bounties, this mission is kill-only. A nice side-effect
   of that is that you can plunder your target before killing them if
   you manage to board them. >:)

--]]
local pir = require "common.pirate"
local fmt = require "format"
local pilotname = require "pilotname"
local lmisn = require "lmisn"

local bounty_setup, fail, level_setup, spawn_pirate, succeed -- Forward-declared functions
-- luacheck: globals jumpin jumpout pilot_attacked pilot_death pilot_jump takeoff (Hook functions passed by name)

-- Mission details
local misn_title = {}
misn_title[1] = _("#rPIRACY:#0: Quick Assassination Job in {sys}{msg}")
misn_title[2] = _("#rPIRACY:#0: Small Assassination Job in {sys}{msg}")
misn_title[3] = _("#rPIRACY:#0: Moderate Assassination Job in {sys}{msg}")
misn_title[4] = _("#rPIRACY:#0: Big Assassination Job in {sys}{msg}")
misn_title[5] = _("#rPIRACY:#0: Dangerous Assassination Job in {sys}{msg}")
misn_title[6] = _("#rPIRACY:#0: Highly Dangerous Assassination Job in {sys}{msg}")

local hunters = {}
local hunter_hits = {}

function create ()
   -- Lower probability on non-pirate places
   if not pir.factionIsPirate( planet.cur():faction() ) and rnd.rnd() < 0.5 then
      misn.finish(false)
   end

   -- Determine paying faction probabilistic
   mem.paying_faction = pir.systemClanP( system.cur() )
   local faction_text = pir.reputationMessage( mem.paying_faction )

   local target_factions = {
      "Independent",
      "Dvaered",
      "Empire",
      "Frontier",
      "Independent",
      "Sirius",
      "Soromid",
      "Trader",
      "Traders Guild",
      "Za'lek",
   }

   local systems = lmisn.getSysAtDistance( system.cur(), 1, 6,
      function(s)
         for i, j in ipairs(target_factions) do
            local p = s:presences()[j]
            if p ~= nil and p > 0 then
               return true
            end
         end
         return false
      end, nil, true )

   if #systems == 0 then
      -- No enemy presence nearby
      misn.finish( false )
   end

   mem.missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( mem.missys ) then misn.finish( false ) end

   mem.target_faction = nil
   while mem.target_faction == nil and #target_factions > 0 do
      local i = rnd.rnd( 1, #target_factions )
      local p = mem.missys:presences()[ target_factions[i] ]
      if p ~= nil and p > 0 then
         mem.target_faction = target_factions[i]
      else
         for j = i, #target_factions do
            target_factions[j] = target_factions[j + 1]
         end
      end
   end

   if mem.target_faction == nil then
      -- Should not happen, but putting this here just in case.
      misn.finish( false )
   end

   mem.jumps_permitted = system.cur():jumpDist(mem.missys, true) + rnd.rnd( 3, 10 )
   if rnd.rnd() < 0.05 then
      mem.jumps_permitted = mem.jumps_permitted - 1
   end

   mem.name = pilotname.generic()
   mem.level = level_setup()
   mem.pship, mem.credits, mem.reputation = bounty_setup()

   -- Set mission details
   if pir.factionIsClan( mem.paying_faction ) then
      misn.setTitle( fmt.f( misn_title[mem.level], {sys=mem.missys:name(), msg=fmt.f(_(" ({fct})"), {fct=mem.paying_faction} )} ) )
   else
      misn.setTitle( fmt.f( misn_title[mem.level], {sys=mem.missys, msg=""} ) )
   end

   local mdesc = fmt.f( _("A meddlesome {fct} pilot known as {plt} was recently seen in the {sys} system. Local crime lords want this pilot dead. {plt} is known to be flying a {shipclass}-class ship.{msg}"), {fct=mem.target_faction, plt=mem.name, sys=mem.missys, shipclass=ship.get(mem.pship):classDisplay(), msg=faction_text } )
   if not pir.factionIsPirate( planet.cur():faction() ) then
      -- We're not on a pirate stronghold, so include a warning that the
      -- mission is in fact illegal (for new players).
      mdesc = mdesc .. "\n\n" .. _("#rWARNING:#0 This mission is illegal and will get you in trouble with the authorities!")
   end
   misn.setDesc( mdesc )

   misn.setReward( fmt.credits( mem.credits ) )
   mem.marker = misn.markerAdd( mem.missys, "computer" )
end


function accept ()
   misn.accept()

   misn.osdCreate( _("Assassination"), {
      fmt.f( _("Fly to the {sys} system"), {sys=mem.missys} ),
      fmt.f( _("Kill {plt}"), {plt=mem.name} ),
   } )

   mem.last_sys = system.cur()

   hook.jumpin( "jumpin" )
   hook.jumpout( "jumpout" )
   hook.takeoff( "takeoff" )
end


function jumpin ()
   -- Nothing to do.
   if system.cur() ~= mem.missys then
      return
   end

   local pos = jump.pos( system.cur(), mem.last_sys )
   local offset_ranges = { { -2500, -1500 }, { 1500, 2500 } }
   local xrange = offset_ranges[ rnd.rnd( 1, #offset_ranges ) ]
   local yrange = offset_ranges[ rnd.rnd( 1, #offset_ranges ) ]
   pos = pos + vec2.new( rnd.rnd( xrange[1], xrange[2] ), rnd.rnd( yrange[1], yrange[2] ) )
   spawn_pirate( pos )
end


function jumpout ()
   mem.jumps_permitted = mem.jumps_permitted - 1
   mem.last_sys = system.cur()
   if mem.last_sys == mem.missys then
      fail( fmt.f( _("MISSION FAILURE! You have left the {sys} system."), {sys=mem.last_sys} ) )
   end
end


function takeoff ()
   -- Nothing to do.
   if system.cur() ~= mem.missys then
      return
   end

   spawn_pirate()
end


function pilot_attacked( _p, attacker, dmg )
   if attacker ~= nil then
      local found = false

      for i, j in ipairs( hunters ) do
         if j == attacker then
            hunter_hits[i] = hunter_hits[i] + dmg
            found = true
         end
      end

      if not found then
         local i = #hunters + 1
         hunters[i] = attacker
         hunter_hits[i] = dmg
      end
   end
end


function pilot_death( _p, attacker )
   if attacker == player.pilot() or attacker:leader() == player.pilot() then
      succeed()
   else
      local top_hunter = nil
      local top_hits = 0
      local player_hits = 0
      local total_hits = 0
      for i, j in ipairs( hunters ) do
         total_hits = total_hits + hunter_hits[i]
         if j ~= nil and j:exists() then
            if j == player.pilot() or j:leader() == player.pilot() then
               player_hits = player_hits + hunter_hits[i]
            elseif hunter_hits[i] > top_hits then
               top_hunter = j
               top_hits = hunter_hits[i]
            end
         end
      end

      if top_hunter == nil or player_hits >= top_hits then
         succeed()
      elseif player_hits >= top_hits / 2 and rnd.rnd() < 0.5 then
         succeed()
      else
         fail( _("MISSION FAILURE! Another pilot eliminated your target.") )
      end
   end
end


function pilot_jump ()
   fail( _("MISSION FAILURE! Target got away.") )
end


-- Set up the level of the mission
function level_setup ()
   local num_pirates = mem.missys:presences()[mem.target_faction]
   local level
   if mem.target_faction == "Independent" then
      level = 1
   elseif mem.target_faction == "Trader" or mem.target_faction == "Traders Guild" then
      if num_pirates <= 100 then
         level = rnd.rnd( 1, 2 )
      else
         level = rnd.rnd( 2, 3 )
      end
   elseif mem.target_faction == "Dvaered" then
      if num_pirates <= 200 then
         level = 2
      elseif num_pirates <= 300 then
         level = rnd.rnd( 2, 3 )
      elseif num_pirates <= 600 then
         level = rnd.rnd( 2, 4 )
      elseif num_pirates <= 800 then
         level = rnd.rnd( 3, 5 )
      else
         level = rnd.rnd( 4, 5 )
      end
   elseif mem.target_faction == "Frontier" then
      if num_pirates <= 150 then
         level = rnd.rnd( 1, 2 )
      else
         level = rnd.rnd( 2, 3 )
      end
   elseif mem.target_faction == "Sirius" then
      if num_pirates <= 150 then
         level = 1
      elseif num_pirates <= 200 then
         level = rnd.rnd( 1, 2 )
      elseif num_pirates <= 500 then
         level = rnd.rnd( 1, 3 )
      elseif num_pirates <= 800 then
         level = rnd.rnd( 2, 5 )
      else
         level = rnd.rnd( 3, #misn_title )
      end
      -- House Sirius does not have a Destroyer class ship
      if level == 4 then level = 3 end
   elseif mem.target_faction == "Za'lek" then
      if num_pirates <= 300 then
         level = 3
      elseif num_pirates <= 500 then
         level = rnd.rnd( 3, 4 )
      elseif num_pirates <= 800 then
         level = rnd.rnd( 3, 5 )
      else
         level = rnd.rnd( 4, #misn_title )
      end
   else
      if num_pirates <= 150 then
         level = 1
      elseif num_pirates <= 200 then
         level = rnd.rnd( 1, 2 )
      elseif num_pirates <= 300 then
         level = rnd.rnd( 1, 3 )
      elseif num_pirates <= 500 then
         level = rnd.rnd( 1, 4 )
      elseif num_pirates <= 800 then
         level = rnd.rnd( 2, 5 )
      else
         level = rnd.rnd( 3, #misn_title )
      end
   end
   return level
end


-- Set up the ship, credits, and reputation based on the level.
function bounty_setup ()
   local pship = "Schroedinger"
   local credits = 50e3
   local reputation = 0

   if mem.target_faction == "Empire" then
      if mem.level == 1 then
         pship = "Empire Shark"
         credits = 200e3 + rnd.sigma() * 15e3
         reputation = 1
      elseif mem.level == 2 then
         pship = "Empire Lancelot"
         credits = 350e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif mem.level == 3 then
         pship = "Empire Admonisher"
         credits = 600e3 + rnd.sigma() * 80e3
         reputation = 3
      elseif mem.level == 4 then
         pship = "Empire Pacifier"
         credits = 850e3 + rnd.sigma() * 100e3
         reputation = 6
      elseif mem.level == 5 then
         pship = "Empire Hawking"
         credits = 1500e3 + rnd.sigma() * 200e3
         reputation = 10
      elseif mem.level == 6 then
         pship = "Empire Peacemaker"
         credits = 2.3e6 + rnd.sigma() * 300e3
         reputation = 20
      end
   elseif mem.target_faction == "Dvaered" then
      if mem.level <= 2 then
         if rnd.rnd() < 0.5 then
            pship = "Dvaered Ancestor"
         else
            pship = "Dvaered Vendetta"
         end
         credits = 350e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif mem.level == 3 then
         pship = "Dvaered Phalanx"
         credits = 600e3 + rnd.sigma() * 80e3
         reputation = 3
      elseif mem.level == 4 then
         pship = "Dvaered Vigilance"
         credits = 900e3 + rnd.sigma() * 120e3
         reputation = 6
      elseif mem.level >= 5 then
         pship = "Dvaered Goddard"
         credits = 1.9e6 + rnd.sigma() * 200e3
         reputation = 10
      end
   elseif mem.target_faction == "Soromid" then
      if mem.level == 1 then
         pship = "Soromid Brigand"
         credits = 150e3 + rnd.sigma() * 15e3
         reputation = 1
      elseif mem.level == 2 then
         if rnd.rnd() < 0.5 then
            pship = "Soromid Reaver"
         else
            pship = "Soromid Marauder"
         end
         credits = 350e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif mem.level == 3 then
         pship = "Soromid Odium"
         credits = 600e3 + rnd.sigma() * 80e3
         reputation = 3
      elseif mem.level == 4 then
         pship = "Soromid Nyx"
         credits = 900e3 + rnd.sigma() * 120e3
         reputation = 6
      elseif mem.level == 5 then
         pship = "Soromid Ira"
         credits = 1.8e6 + rnd.sigma() * 200e3
         reputation = 10
      elseif mem.level == 6 then
         if rnd.rnd() < 0.5 then
            pship = "Soromid Arx"
         else
            pship = "Soromid Vox"
         end
         credits = 2.5e6 + rnd.sigma() * 300e3
         reputation = 20
      end
   elseif mem.target_faction == "Frontier" then
      if mem.level == 1 then
         pship = "Hyena"
         credits = 100e3 + rnd.sigma() * 7.5e3
         reputation = 0
      elseif mem.level == 2 then
         if rnd.rnd() < 0.5 then
            pship = "Lancelot"
         else
            pship = "Ancestor"
         end
         credits = 350e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif mem.level >= 3 then
         pship = "Phalanx"
         credits = 500e3 + rnd.sigma() * 80e3
         reputation = 3
      end
   elseif mem.target_faction == "Sirius" then
      if mem.level == 1 then
         pship = "Sirius Fidelity"
         credits = 150e3 + rnd.sigma() * 15e3
         reputation = 1
      elseif mem.level == 2 then
         pship = "Sirius Shaman"
         credits = 350e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif mem.level == 3 or mem.level == 4 then
         pship = "Sirius Preacher"
         credits = 600e3 + rnd.sigma() * 80e3
         reputation = 3
      elseif mem.level == 5 then
         pship = "Sirius Dogma"
         credits = 1.5e6 + rnd.sigma() * 200e3
         reputation = 10
      elseif mem.level == 6 then
         pship = "Sirius Divinity"
         credits = 2.2e6 + rnd.sigma() * 300e3
         reputation = 20
      end
   elseif mem.target_faction == "Za'lek" then
      if mem.level <= 3 then
         pship = "Za'lek Sting"
         credits = 600e3 + rnd.sigma() * 80e3
         reputation = 3
      elseif mem.level == 4 then
         pship = "Za'lek Demon"
         credits = 900e3 + rnd.sigma() * 120e3
         reputation = 6
      elseif mem.level == 5 then
         if rnd.rnd() < 0.5 then
            pship = "Za'lek Diablo"
         else
            pship = "Za'lek Mephisto"
         end
         credits = 1.9e6 + rnd.sigma() * 200e3
         reputation = 10
      elseif mem.level == 6 then
         pship = "Za'lek Hephaestus"
         credits = 3e6 + rnd.sigma() * 300e3
         reputation = 20
      end
   elseif mem.target_faction == "Trader" then
      if mem.level == 1 then
         if rnd.rnd() < 0.5 then
            pship = "Gawain"
         else
            pship = "Llama"
         end
         credits = 100e3 + rnd.sigma() * 5e3
         reputation = 0
      elseif mem.level == 2 then
         if rnd.rnd() < 0.5 then
            pship = "Koala"
         else
            pship = "Quicksilver"
         end
         credits = 300e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif mem.level >= 3 then
         if rnd.rnd() < 0.5 then
            pship = "Rhino"
            credits = 700e3 + rnd.sigma() * 80e3
         else
            pship = "Mule"
            credits = 600e3 + rnd.sigma() * 80e3
         end
         reputation = 3
      end
   elseif mem.target_faction == "Traders Guild" then
      if mem.level == 1 then
         if rnd.rnd() < 0.5 then
            pship = "Gawain"
         else
            pship = "Llama"
         end
         credits = 100e3 + rnd.sigma() * 5e3
         reputation = 0
      elseif mem.level == 2 then
         if rnd.rnd() < 0.5 then
            pship = "Koala"
         else
            pship = "Quicksilver"
         end
         credits = 300e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif mem.level >= 3 then
         if rnd.rnd() < 0.5 then
            pship = "Rhino"
            credits = 700e3 + rnd.sigma() * 80e3
         else
            pship = "Mule"
            credits = 600e3 + rnd.sigma() * 80e3
         end
         reputation = 3
      end
   elseif mem.target_faction == "Independent" then
      local choices = {}
      choices[1] = "Schroedinger"
      choices[2] = "Hyena"
      choices[3] = "Gawain"
      choices[4] = "Llama"

      pship = choices[ rnd.rnd( 1, #choices ) ]
      credits = 60e3 + rnd.sigma() * 5e3
      reputation = 0
   end
   return pship, credits, reputation
end


-- Spawn the ship at the location param.
function spawn_pirate( param )
   if mem.jumps_permitted < 0 then
      fail( _("MISSION FAILURE! Target got away.") )
      return
   end

   misn.osdActive( 2 )
   local target_ship = pilot.add( mem.pship, mem.target_faction, param, mem.name )
   target_ship:setHilight( true )
   hook.pilot( target_ship, "attacked", "pilot_attacked" )
   hook.pilot( target_ship, "death", "pilot_death" )
   hook.pilot( target_ship, "jump", "pilot_jump" )
   hook.pilot( target_ship, "land", "pilot_jump" )
   return target_ship
end


-- Succeed the mission
function succeed ()
   player.msg( "#g" .. fmt.f(_("MISSION SUCCESS! Pay of {credits} has been transferred into your account."),{credits=fmt.credits(mem.credits)}) .. "#0" )
   player.pay( mem.credits )

   -- Pirate rep cap increase
   local bounty_done = var.peek( "pir_bounty_done" )
   var.push( "pir_bounty_done", true )
   if bounty_done ~= true then
      var.push( "_fcap_pirate", var.peek( "_fcap_pirate" ) + 5 )
   end

   if mem.level >= 5 then
      local bounty_dangerous_done = var.peek( "pir_bounty_dangerous_done" )
      var.push( "pir_bounty_dangerous_done", true )
      if not bounty_dangerous_done then
         pir.modReputation( 2 )
         pir.modDecayFloor( 2 )
      end

      if mem.level >= 6 then
         local bounty_highly_dangerous_done = var.peek( "pir_bounty_highly_dangerous_done" )
         var.push( "pir_bounty_highly_dangerous_done", true )
         if not bounty_highly_dangerous_done then
            pir.modReputation( 3 )
            pir.modDecayFloor( 3 )
         end
      end
   end

   mem.paying_faction:modPlayerSingle( mem.reputation )
   misn.finish( true )
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
