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

-- Mission details
misn_title = {}
misn_title[1] = _("#rPIRACY:#0: Quick Assassination Job in %s%s")
misn_title[2] = _("#rPIRACY:#0: Small Assassination Job in %s%s")
misn_title[3] = _("#rPIRACY:#0: Moderate Assassination Job in %s%s")
misn_title[4] = _("#rPIRACY:#0: Big Assassination Job in %s%s")
misn_title[5] = _("#rPIRACY:#0: Dangerous Assassination Job in %s%s")
misn_title[6] = _("#rPIRACY:#0: Highly Dangerous Assassination Job in %s%s")

hunters = {}
hunter_hits = {}

function create ()
   -- Lower probability on non-pirate places
   if not pir.factionIsPirate( planet.cur():faction() ) and rnd.rnd() < 0.5 then
      misn.finish(false)
   end

   -- Determine paying faction probabilistic
   paying_faction = pir.systemClanP( system.cur() )
   local faction_text = pir.reputationMessage( paying_faction )

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

   missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( missys ) then misn.finish( false ) end

   target_faction = nil
   while target_faction == nil and #target_factions > 0 do
      local i = rnd.rnd( 1, #target_factions )
      local p = missys:presences()[ target_factions[i] ]
      if p ~= nil and p > 0 then
         target_faction = target_factions[i]
      else
         for j = i, #target_factions do
            target_factions[j] = target_factions[j + 1]
         end
      end
   end

   if target_faction == nil then
      -- Should not happen, but putting this here just in case.
      misn.finish( false )
   end

   jumps_permitted = system.cur():jumpDist(missys, true) + rnd.rnd( 3, 10 )
   if rnd.rnd() < 0.05 then
      jumps_permitted = jumps_permitted - 1
   end

   name = pilotname.generic()
   level = level_setup()
   pship, credits, reputation = bounty_setup()

   -- Set mission details
   if pir.factionIsClan( paying_faction ) then
      misn.setTitle( misn_title[level]:format( missys:name(), string.format(_(" (%s)"), paying_faction:name() ) ) )
   else
      misn.setTitle( misn_title[level]:format( missys:name(), "" ) )
   end

   local mdesc = fmt.f( _("A meddlesome {fctname} pilot known as {pltname} was recently seen in the {sys} system. Local crime lords want this pilot dead. {pltname} is known to be flying a {shipclass}-class ship.{fcttext}"), {fctname=target_faction, pltname=name, sys=missys, shipclass=ship.get(pship):classDisplay(), fcttext=faction_text } )
   if not pir.factionIsPirate( planet.cur():faction() ) then
      -- We're not on a pirate stronghold, so include a warning that the
      -- mission is in fact illegal (for new players).
      mdesc = mdesc .. "\n\n" .. _("#rWARNING:#0 This mission is illegal and will get you in trouble with the authorities!")
   end
   misn.setDesc( mdesc )

   misn.setReward( fmt.credits( credits ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   misn.accept()

   misn.osdCreate( _("Assassination"), {
      fmt.f( _("Fly to the {sys} system"), {sys=missys} ),
      fmt.f( _("Kill {pltname}"), {pltname=name} ),
   } )

   last_sys = system.cur()

   hook.jumpin( "jumpin" )
   hook.jumpout( "jumpout" )
   hook.takeoff( "takeoff" )
end


function jumpin ()
   -- Nothing to do.
   if system.cur() ~= missys then
      return
   end

   local pos = jump.pos( system.cur(), last_sys )
   local offset_ranges = { { -2500, -1500 }, { 1500, 2500 } }
   local xrange = offset_ranges[ rnd.rnd( 1, #offset_ranges ) ]
   local yrange = offset_ranges[ rnd.rnd( 1, #offset_ranges ) ]
   pos = pos + vec2.new( rnd.rnd( xrange[1], xrange[2] ), rnd.rnd( yrange[1], yrange[2] ) )
   spawn_pirate( pos )
end


function jumpout ()
   jumps_permitted = jumps_permitted - 1
   last_sys = system.cur()
   if not job_done and last_sys == missys then
      fail( fmt.f( _("MISSION FAILURE! You have left the {sys} system."), {sys=last_sys} ) )
   end
end


function takeoff ()
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
   local num_pirates = missys:presences()[target_faction]
   local level
   if target_faction == "Independent" then
      level = 1
   elseif target_faction == "Trader" or target_faction == "Traders Guild" then
      if num_pirates <= 100 then
         level = rnd.rnd( 1, 2 )
      else
         level = rnd.rnd( 2, 3 )
      end
   elseif target_faction == "Dvaered" then
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
   elseif target_faction == "Frontier" then
      if num_pirates <= 150 then
         level = rnd.rnd( 1, 2 )
      else
         level = rnd.rnd( 2, 3 )
      end
   elseif target_faction == "Sirius" then
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
   elseif target_faction == "Za'lek" then
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

   if target_faction == "Empire" then
      if level == 1 then
         pship = "Empire Shark"
         credits = 200e3 + rnd.sigma() * 15e3
         reputation = 1
      elseif level == 2 then
         pship = "Empire Lancelot"
         credits = 450e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif level == 3 then
         pship = "Empire Admonisher"
         credits = 800e3 + rnd.sigma() * 80e3
         reputation = 3
      elseif level == 4 then
         pship = "Empire Pacifier"
         credits = 1200e3 + rnd.sigma() * 120e3
         reputation = 6
      elseif level == 5 then
         pship = "Empire Hawking"
         credits = 1800e3 + rnd.sigma() * 200e3
         reputation = 10
      elseif level == 6 then
         pship = "Empire Peacemaker"
         credits = 3e6 + rnd.sigma() * 500e3
         reputation = 20
      end
   elseif target_faction == "Dvaered" then
      if level <= 2 then
         if rnd.rnd() < 0.5 then
            pship = "Dvaered Ancestor"
         else
            pship = "Dvaered Vendetta"
         end
         credits = 450e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif level == 3 then
         pship = "Dvaered Phalanx"
         credits = 800e3 + rnd.sigma() * 80e3
         reputation = 3
      elseif level == 4 then
         pship = "Dvaered Vigilance"
         credits = 1200e3 + rnd.sigma() * 120e3
         reputation = 6
      elseif level >= 5 then
         pship = "Dvaered Goddard"
         credits = 1800e3 + rnd.sigma() * 200e3
         reputation = 10
      end
   elseif target_faction == "Soromid" then
      if level == 1 then
         pship = "Soromid Brigand"
         credits = 200e3 + rnd.sigma() * 15e3
         reputation = 1
      elseif level == 2 then
         if rnd.rnd() < 0.5 then
            pship = "Soromid Reaver"
         else
            pship = "Soromid Marauder"
         end
         credits = 450e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif level == 3 then
         pship = "Soromid Odium"
         credits = 800e3 + rnd.sigma() * 80e3
         reputation = 3
      elseif level == 4 then
         pship = "Soromid Nyx"
         credits = 1200e3 + rnd.sigma() * 120e3
         reputation = 6
      elseif level == 5 then
         pship = "Soromid Ira"
         credits = 1800e3 + rnd.sigma() * 200e3
         reputation = 10
      elseif level == 6 then
         if rnd.rnd() < 0.5 then
            pship = "Soromid Arx"
         else
            pship = "Soromid Vox"
         end
         credits = 3e6 + rnd.sigma() * 500e3
         reputation = 20
      end
   elseif target_faction == "Frontier" then
      if level == 1 then
         pship = "Hyena"
         credits = 100e3 + rnd.sigma() * 7500
         reputation = 0
      elseif level == 2 then
         if rnd.rnd() < 0.5 then
            pship = "Lancelot"
         else
            pship = "Ancestor"
         end
         credits = 450e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif level >= 3 then
         pship = "Phalanx"
         credits = 800e3 + rnd.sigma() * 80e3
         reputation = 3
      end
   elseif target_faction == "Sirius" then
      if level == 1 then
         pship = "Sirius Fidelity"
         credits = 200e3 + rnd.sigma() * 15e3
         reputation = 1
      elseif level == 2 then
         pship = "Sirius Shaman"
         credits = 450e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif level == 3 or level == 4 then
         pship = "Sirius Preacher"
         credits = 800e3 + rnd.sigma() * 80e3
         reputation = 3
      elseif level == 5 then
         pship = "Sirius Dogma"
         credits = 1800e3 + rnd.sigma() * 200e3
         reputation = 10
      elseif level == 6 then
         pship = "Sirius Divinity"
         credits = 3e6 + rnd.sigma() * 500e3
         reputation = 20
      end
   elseif target_faction == "Za'lek" then
      if level <= 3 then
         pship = "Za'lek Sting"
         credits = 80e3 + rnd.sigma() * 80e3
         reputation = 3
      elseif level == 4 then
         pship = "Za'lek Demon"
         credits = 1200e3 + rnd.sigma() * 120e3
         reputation = 6
      elseif level == 5 then
         if rnd.rnd() < 0.5 then
            pship = "Za'lek Diablo"
         else
            pship = "Za'lek Mephisto"
         end
         credits = 1800e3 + rnd.sigma() * 200e3
         reputation = 10
      elseif level == 6 then
         pship = "Za'lek Hephaestus"
         credits = 3e6 + rnd.sigma() * 500e3
         reputation = 20
      end
   elseif target_faction == "Trader" then
      if level == 1 then
         if rnd.rnd() < 0.5 then
            pship = "Gawain"
         else
            pship = "Koala"
         end
         credits = 75e3 + rnd.sigma() * 5e3
         reputation = 0
      elseif level == 2 then
         if rnd.rnd() < 0.5 then
            pship = "Llama"
         else
            pship = "Quicksilver"
         end
         credits = 350e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif level >= 3 then
         if rnd.rnd() < 0.5 then
            pship = "Rhino"
            credits = 800e3 + rnd.sigma() * 80e3
         else
            pship = "Mule"
            credits = 700e3 + rnd.sigma() * 80e3
         end
         reputation = 3
      end
   elseif target_faction == "Traders Guild" then
      if level == 1 then
         if rnd.rnd() < 0.5 then
            pship = "Gawain"
         else
            pship = "Koala"
         end
         credits = 100e3 + rnd.sigma() * 5e3
         reputation = 0
      elseif level == 2 then
         if rnd.rnd() < 0.5 then
            pship = "Llama"
         else
            pship = "Quicksilver"
         end
         credits = 400e3 + rnd.sigma() * 50e3
         reputation = 2
      elseif level >= 3 then
         if rnd.rnd() < 0.5 then
            pship = "Rhino"
            credits = 900e3 + rnd.sigma() * 80e3
         else
            pship = "Mule"
            credits = 800e3 + rnd.sigma() * 80e3
         end
         reputation = 3
      end
   elseif target_faction == "Independent" then
      local choices = {}
      choices[1] = "Schroedinger"
      choices[2] = "Hyena"
      choices[3] = "Gawain"
      choices[4] = "Llama"

      pship = choices[ rnd.rnd( 1, #choices ) ]
      credits = 50e3 + rnd.sigma() * 5e3
      reputation = 0
   end
   return pship, credits, reputation
end


-- Spawn the ship at the location param.
function spawn_pirate( param )
   if job_done or system.cur() ~= missys then
      return
   end

   if jumps_permitted < 0 then
      fail( _("MISSION FAILURE! Target got away.") )
      return
   end

   misn.osdActive( 2 )
   target_ship = pilot.add( pship, target_faction, param )
   target_ship:rename( name )
   target_ship:setHilight( true )
   hook.pilot( target_ship, "attacked", "pilot_attacked" )
   hook.pilot( target_ship, "death", "pilot_death" )
   hook.pilot( target_ship, "jump", "pilot_jump" )
   hook.pilot( target_ship, "land", "pilot_jump" )
   return target_ship
end


-- Succeed the mission
function succeed ()
   player.msg( "#g" .. _("MISSION SUCCESS! Pay has been transferred into your account.") .. "#0" )
   player.pay( credits )

   -- Pirate rep cap increase
   local bounty_done = var.peek( "pir_bounty_done" )
   var.push( "pir_bounty_done", true )
   if bounty_done ~= true then
      var.push( "_fcap_pirate", var.peek( "_fcap_pirate" ) + 5 )
   end

   if level >= 5 then
      local bounty_dangerous_done = var.peek( "pir_bounty_dangerous_done" )
      var.push( "pir_bounty_dangerous_done", true )
      if not bounty_dangerous_done then
         pir.modReputation( 2 )
         pir.modDecayFloor( 2 )
      end

      if level >= 6 then
         local bounty_highly_dangerous_done = var.peek( "pir_bounty_highly_dangerous_done" )
         var.push( "pir_bounty_highly_dangerous_done", true )
         if not bounty_highly_dangerous_done then
            pir.modReputation( 3 )
            pir.modDecayFloor( 3 )
         end
      end
   end

   paying_faction:modPlayerSingle( reputation )
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
