--[[

   Pirate Empire bounty
   Copyright 2014, 2015, 2019 Julie Marchant

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

--

   Based on the pirate bounty mission (hence the weird terminology in
   many variable names). Replacement for the old "Pirate Empire bounty"
   mission.

   Unlike Pirate bounties, this mission is kill-only. A nice side-effect
   of that is that you can plunder your target before killing them if
   you manage to board them. >:)

--]]

include "numstring.lua"
include "jumpdist.lua"
include "pilot/empire.lua"


-- Mission details
misn_title  = _("PIRACY: %s Assassination Job in %s")
misn_reward = _("%s credits")
misn_desc   = _("The Empire official known as %s was recently seen in the %s system. Local crime lords want this official dead.")
desc_illegal_warning = _("WARNING: This mission is illegal and will get you in trouble with the authorities!!")

misn_level    = {}
misn_level[1] = _("Minor")            -- Empire Shark
misn_level[2] = _("Small")            -- Empire Lancelot
misn_level[3] = _("Moderate")         -- Empire Admonisher
misn_level[4] = _("Big")              -- Empire Pacifier
misn_level[5] = _("Dangerous")        -- Empire Hawking
misn_level[6] = _("Highly Dangerous") -- Empire Peacemaker

-- Messages
msg    = {}
msg[1] = _("MISSION FAILURE! %s got away.")
msg[2] = _("MISSION FAILURE! Another pilot eliminated %s.")
msg[3] = _("MISSION FAILURE! You have left the %s system.")
msg[4] = _("MISSION SUCCESS! Pay has been transferred into your account.")

osd_title = _("Assassination")
osd_msg    = {}
osd_msg[1] = _("Fly to the %s system")
osd_msg[2] = _("Kill %s")
osd_msg["__save"] = true


hunters = {}
hunter_hits = {}


function create ()
   paying_faction = faction.get("Pirate")

   local systems = getsysatdistance( system.cur(), 1, 3,
      function(s)
         local p = s:presences()["Empire"]
         return p ~= nil and p > 0
      end )

   if #systems == 0 then
      -- No Empire presence nearby
      misn.finish( false )
   end

   missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( missys ) then misn.finish( false ) end

   jumps_permitted = missys:jumpDist() + rnd.rnd( 3, 10 )
   if rnd.rnd() < 0.05 then
      jumps_permitted = jumps_permitted - 1
   end

   local num_pirates = missys:presences()["Empire"]
   if num_pirates <= 150 then
      level = 1
   elseif num_pirates <= 200 then
      level = rnd.rnd( 1, 2 )
   elseif num_pirates <= 300 then
      level = rnd.rnd( 2, 3 )
   elseif num_pirates <= 500 then
      level = rnd.rnd( 3, 4 )
   elseif num_pirates <= 800 then
      level = rnd.rnd( 4, 5 )
   else
      level = rnd.rnd( 4, #misn_level )
   end

   name = empire_name()
   ship = "Empire Shark"
   credits = 200000
   reputation = 0
   bounty_setup()

   -- Set mission details
   misn.setTitle( misn_title:format( misn_level[level], missys:name() ) )

   if planet.cur():faction():name() == "Pirate" then
      misn.setDesc( misn_desc:format( name, missys:name(), paying_faction:name() ) )
   else
      -- We're not on a pirate stronghold, so include a warning that the
      -- mission is in fact illegal (for new players).
      misn.setDesc( misn_desc:format( name, missys:name(), paying_faction:name() ) .. "\n\n" .. desc_illegal_warning )
   end

   misn.setReward( misn_reward:format( numstring( credits ) ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   misn.accept()

   osd_msg[1] = osd_msg[1]:format( missys:name() )
   osd_msg[2] = osd_msg[2]:format( name )
   misn.osdCreate( osd_title, osd_msg )

   last_sys = system.cur()

   hook.jumpin( "jumpin" )
   hook.jumpout( "jumpout" )
   hook.takeoff( "takeoff" )
   hook.land( "land" )
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
      fail( msg[3]:format( last_sys:name() ) )
   end
end


function takeoff ()
   spawn_pirate()
end


function pilot_attacked( p, attacker, dmg )
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


function pilot_death( p, attacker )
   if attacker == player.pilot() then
      succeed()
   else
      local top_hunter = nil
      local top_hits = 0
      local player_hits = 0
      local total_hits = 0
      for i, j in ipairs( hunters ) do
         total_hits = total_hits + hunter_hits[i]
         if j ~= nil then
            if j == player.pilot() then
               player_hits = player_hits + hunter_hits[i]
            elseif j:exists() and hunter_hits[i] > top_hits then
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
         fail( msg[2]:format( name ) )
      end
   end
end


function pilot_jump ()
   fail( msg[1]:format( name ) )
end


-- Set up the ship, credits, and reputation based on the level.
function bounty_setup ()
   if level == 1 then
      ship = "Empire Shark"
      credits = 200000 + rnd.sigma() * 15000
      reputation = 1
   elseif level == 2 then
      ship = "Empire Lancelot"
      credits = 450000 + rnd.sigma() * 50000
      reputation = 2
   elseif level == 3 then
      ship = "Empire Admonisher"
      credits = 800000 + rnd.sigma() * 80000
      reputation = 3
   elseif level == 4 then
      ship = "Empire Pacifier"
      credits = 1200000 + rnd.sigma() * 120000
      reputation = 6
   elseif level == 5 then
      ship = "Empire Hawking"
      credits = 1800000 + rnd.sigma() * 200000
      reputation = 10
   elseif level == 6 then
      ship = "Empire Peacemaker"
      credits = 3000000 + rnd.sigma() * 500000
      reputation = 20
   end
end


-- Spawn the ship at the location param.
function spawn_pirate( param )
   if not job_done and system.cur() == missys then
      if jumps_permitted >= 0 then
         misn.osdActive( 2 )
         target_ship = pilot.add( ship, nil, param )[1]
         target_ship:rename( name )
         target_ship:setHilight( true )
         hook.pilot( target_ship, "attacked", "pilot_attacked" )
         death_hook = hook.pilot( target_ship, "death", "pilot_death" )
         pir_jump_hook = hook.pilot( target_ship, "jump", "pilot_jump" )
      else
         fail( msg[1]:format( name ) )
      end
   end
end


-- Succeed the mission
function succeed ()
   player.msg( "\ag" .. msg[4] .. "\a0" )
   player.pay( credits )
   paying_faction:modPlayerSingle( reputation )
   misn.finish( true )
end


-- Fail the mission, showing message to the player.
function fail( message )
   if message ~= nil then
      -- Pre-colourized, do nothing.
      if message:find("\a") then
         player.msg( message )
      -- Colourize in red.
      else
         player.msg( "\ar" .. message .. "\a0" )
      end
   end
   misn.finish( false )
end
