--[[

   Dead or Alive Pirate Bounty
   Copyright 2014 Julian Marchant

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

   Generalized replacement for bobbens' Empire Pirate Bounty mission.
   Can work with any faction.

--]]

include "numstring.lua"
include "jumpdist.lua"
include "pilot/pirate.lua"

-- Localization
lang = naev.lang()
if lang == "es" then
else -- Default to English
   subdue_title   = "Captured Alive"
   subdue_text    = {}
   subdue_text[1] = [[You and your crew infiltrate the ship's pathetic security and subdue %s. You transport the pirate to your ship.]]
   subdue_text[2] = [[Your crew has a surprisingly difficult time getting past the ship's security, but eventually succeeds and subdues %s.]]

   pay_title   = "Mission Completed"

   pay_kill_text    = {}
   pay_kill_text[1] = "After verifying that you killed %s, an officer hands you your pay."
   pay_kill_text[2] = "After verifying that %s is indeed dead, the tired-looking officer smiles and hands you your pay."
   pay_kill_text[3] = "The officer seems pleased that %s is finally dead. He thanks you and promptly hands you your pay."
   pay_kill_text[4] = "The paranoid-looking officer takes you into a locked room, where he quietly verifies the death of %s. He then pays you and sends you off."

   pay_capture_text    = {}
   pay_capture_text[1] = "An officer takes %s into custody and hands you your pay."
   pay_capture_text[2] = "The officer seems to think your decision to capture %s alive was insane. He carefully takes the pirate off your hands, taking precautions you think are completely unnecessary, and then hands you your pay"
   pay_capture_text[3] = "The officer you deal with seems to especially dislike %s. He takes the pirate off your hands and hands you your pay without speaking a word."
   pay_capture_text[4] = "A fearful-looking officer rushes %s into a secure hold, pays you the appropriate bounty, and then hurries off."

   share_title   = "A Smaller Reward"
   share_text    = {}
   share_text[1] = [["Greetings. I can see that you were trying to collect a bounty on %s. Well, as you can see, I earned the bounty, but I don't think I would have succeeded without your help, so I've transferred a portion of the bounty into your account."]]
   share_text[2] = [["Sorry about getting in the way of your bounty. I don't really care too much about the money, but I just wanted to make sure the galaxy would be rid of that scum; I've seen the villainy of %s first-hand, you see. So as an apology, I would like to offer you the portion of the bounty you clearly earned. The money will be in your account shortly."]]
   share_text[3] = [["Hey, thanks for the help back there. I don't know if I would have been able to handle %s alone! Anyway, since you were such a big help, I have transferred what I think is your fair share of the bounty to your bank account."]]
   share_text[4] = [["Heh, thanks! I think I would have been able to take out %s by myself, but still, I appreciate your assistance. Here, I'll transfer some of the bounty to you, as a token of my appreciation."]]
   share_text[5] = [["Ha ha ha, looks like I beat you to it this time, eh? Well, I don't do this often, but here, have some of the bounty. I think you deserve it."]]
   

   -- Mission details
   misn_title  = "%s Dead or Alive Bounty in %s"
   misn_reward = "%s credits"
   misn_desc   = "The pirate known as %s was recently seen in the %s system. %s authorities want this pirate dead or alive."

   misn_level    = {}
   misn_level[1] = "Tiny"      -- Pirate Hyena
   misn_level[2] = "Small"     -- Pirate Shark
   misn_level[3] = "Moderate"  -- Pirate Vendetta or Pirate Ancestor
   misn_level[4] = "High"      -- Pirate Admonisher or Pirate Phalanx
   misn_level[5] = "Dangerous" -- Pirate Kestrel

   -- Messages
   msg    = {}
   msg[1] = "MISSION FAILURE! %s got away."
   msg[2] = "MISSION FAILURE! Another pilot eliminated %s."
   msg[3] = "MISSION FAILURE! You have left the %s system."

   osd_title = "Bounty Hunt"
   osd_msg    = {}
   osd_msg[1] = "Fly to the %s system"
   osd_msg[2] = "Kill or capture %s"
   osd_msg[3] = "Land on the nearest %s planet and collect your bounty"
   osd_msg["__save"] = true
end


hunters = {}
hunter_hits = {}


function create ()
   paying_faction = planet.cur():faction()

   local systems = getsysatdistance( system.cur(), 1, 3,
      function(s) return s:presences()["Pirate"] end )

   if #systems == 0 then
      -- No pirates nearby
      misn.finish( false )
   end

   missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( missys ) then misn.finish( false ) end

   jumps_permitted = missys:jumpDist() + rnd.rnd( 5 )
   if rnd.rnd() < 0.05 then
      jumps_permitted = jumps_permitted - 1
   end

   local num_pirates = missys:presences()["Pirate"]
   if num_pirates <= 25 then
      level = 1
   elseif num_pirates <= 50 then
      level = rnd.rnd( 1, 2 )
   elseif num_pirates <= 75 then
      level = rnd.rnd( 2, 3 )
   elseif num_pirates <= 100 then
      level = rnd.rnd( 3, 4 )
   else
      level = rnd.rnd( 4, #misn_level )
   end

   name = pirate_name()
   ship = "Pirate Hyena"
   credits = 50000
   reputation = 0
   bounty_setup()

   -- Set mission details
   misn.setTitle( misn_title:format( misn_level[level], missys:name() ) )
   misn.setDesc( misn_desc:format( name, missys:name(), paying_faction:name() ) )
   misn.setReward( misn_reward:format( numstring( credits ) ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   misn.accept()

   osd_msg[1] = osd_msg[1]:format( missys:name() )
   osd_msg[2] = osd_msg[2]:format( name )
   osd_msg[3] = osd_msg[3]:format( paying_faction:name() )
   misn.osdCreate( osd_title, osd_msg )

   last_sys = system.cur()
   job_done = false
   target_killed = false

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


function land ()
   jumps_permitted = jumps_permitted - 1
   if job_done and planet.cur():faction() == paying_faction then
      local pay_text
      if target_killed then
         pay_text = pay_kill_text[ rnd.rnd( 1, #pay_kill_text ) ]
      else
         pay_text = pay_capture_text[ rnd.rnd( 1, #pay_capture_text ) ]
      end
      tk.msg( pay_title, pay_text:format( name ) )
      player.pay( credits )
      paying_faction:modPlayerSingle( reputation )
      misn.finish( true )
   end
end


function pilot_board ()
   player.unboard()
   local t = subdue_text[ rnd.rnd( 1, #subdue_text ) ]:format( name )
   tk.msg( subdue_title, t )
   succeed()
   target_killed = false
   target_ship:changeAI( "dummy" )
   target_ship:setHilight( false )
   target_ship:disable() -- Stop it from coming back
   if death_hook ~= nil then hook.rm( death_hook ) end
end


function pilot_attacked( p, attacker )
   if attacker ~= nil then
      local found = false
      for i, j in ipairs( hunters ) do
         if j == attacker then
            hunter_hits[i] = hunter_hits[i] + 1
            found = true
         end
      end

      if not found then
         local i = #hunters + 1
         hunters[i] = attacker
         hunter_hits[i] = 1
      end
   end
end


function pilot_death( p, attacker )
   if attacker == player.pilot() then
      succeed()
      target_killed = true
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
         target_killed = true
      elseif player_hits >= top_hits / 2 and rnd.rnd() < 0.5 then
         hailer = hook.pilot( top_hunter, "hail", "hunter_hail", top_hunter )
         credits = credits * player_hits / total_hits
         hook.pilot( top_hunter, "jump", "hunter_leave" )
         hook.pilot( top_hunter, "land", "hunter_leave" )
         hook.jumpout( "hunter_leave" )
         hook.land( "hunter_leave" )
         timer_rehail( top_hunter )
         player.msg( "\027r" .. msg[2]:format( name ) .. "\0270" )
         misn.osdDestroy()
      else
         fail( msg[2]:format( name ) )
      end
   end
end


function pilot_jump ()
   fail( msg[1]:format( name ) )
end


function timer_rehail( arg )
   if rehailer ~= nil then hook.rm( rehailer ) end
   if arg ~= nil and arg:exists() then
      arg:hailPlayer()
      rehailer = hook.timer( 8000, "timer_rehail", arg )
   end
end


function hunter_hail( arg )
   if hailer ~= nil then hook.rm( hailer ) end
   if rehailer ~= nil then hook.rm( rehailer ) end
   player.commClose()

   local text = share_text[ rnd.rnd( 1, #emp_share_text ) ]
   tk.msg( share_title, text:format( name ) )

   player.pay( credits )
   misn.finish( true )
end


function hunter_leave ()
   misn.finish( false )
end


-- Set up the ship, credits, and reputation based on the level.
function bounty_setup ()
   if level == 1 then
      ship = "Pirate Hyena"
      credits = 50000 + rnd.sigma() * 15000
      reputation = 0
   elseif level == 2 then
      ship = "Pirate Shark"
      credits = 150000 + rnd.sigma() * 50000
      reputation = 0
   elseif level == 3 then
      if rnd.rnd() < 0.5 then
         ship = "Pirate Vendetta"
      else
         ship = "Pirate Ancestor"
      end
      credits = 400000 + rnd.sigma() * 80000
      reputation = 1
   elseif level == 4 then
      if rnd.rnd() < 0.5 then
         ship = "Pirate Admonisher"
      else
         ship = "Pirate Phalanx"
      end
      credits = 700000 + rnd.sigma() * 120000
      reputation = 2
   elseif level == 5 then
      ship = "Pirate Kestrel"
      credits = 1200000 + rnd.sigma() * 200000
      reputation = 4
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
         hook.pilot( target_ship, "board", "pilot_board" )
         hook.pilot( target_ship, "attacked", "pilot_attacked" )
         death_hook = hook.pilot( target_ship, "death", "pilot_death" )
         pir_jump_hook = hook.pilot( target_ship, "jump", "pilot_jump" )
      else
         fail( msg[1]:format( name ) )
      end
   end
end


-- Succeed the mission, make the player head to a planet for pay
function succeed ()
   job_done = true
   misn.osdActive( 3 )
   if marker ~= nil then
      misn.markerRm( marker )
   end
   if pir_jump_hook ~= nil then
      hook.rm( pir_jump_hook )
   end
end


-- Fail the mission, showing message to the player.
function fail( message )
   if message ~= nil then
      -- Pre-colourized, do nothing.
      if message:find("\027") then
         player.msg( message )
      -- Colourize in red.
      else
         player.msg( "\027r" .. message .. "\0270" )
      end
   end
   misn.finish( false )
end
