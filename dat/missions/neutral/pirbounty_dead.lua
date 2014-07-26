--[[

   Dead or Alive Pirate Bounty

   Generalized replacement for bobbens' Empire Pirate Bounty mission.
   Can work with any faction.

   Author: onpon4
           Based on Empire Pirate Bounty mission by bobbens.

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
   pay_kill_text[1] = [[After verifying that you finished the job, an officer hands you your pay.]]

   pay_capture_text    = {}
   pay_capture_text[1] = [[An officer takes the criminal into custody and hands you your pay.]]

   -- Mission details
   misn_title  = "%s Dead or Alive Bounty in %s"
   misn_reward = "%s credits"
   misn_desc   = "The pirate known as %s was recently seen in the %s system. This pirate is wanted dead or alive."

   misn_level    = {}
   misn_level[1] = "Tiny"      -- Pirate Hyena
   misn_level[2] = "Small"     -- Pirate Shark
   misn_level[3] = "Moderate"  -- Pirate Vendetta or Pirate Ancestor
   misn_level[4] = "High"      -- Pirate Admonisher or Pirate Phalanx
   misn_level[5] = "Dangerous" -- Pirate Kestrel

   -- Messages
   msg    = {}
   msg[1] = "MISSION FAILURE! %s got away."
   msg[2] = "MISSION FAILURE! Somebody else eliminated %s."
   msg[3] = "MISSION FAILURE! You have left the %s system."

   osd_title = "Bounty Hunt"
   osd_msg    = {}
   osd_msg[1] = "Fly to the %s system"
   osd_msg[2] = "Kill or capture %s"
   osd_msg[3] = "Land on the nearest %s planet and collect your bounty"
   osd_msg["__save"] = true
end


function create ()
   if not player.numOutfit( "Mercenary License" ) then
      -- Player is not qualified for the mission.
      misn.finish( false )
   end

   paying_faction = planet.cur():faction()

   local systems = getsysatdistance( system.cur(), 0, 4,
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

   level = rnd.rnd( 1, #misn_level )
   name = pirate_name()
   ship = "Pirate Hyena"
   credits = 50000
   reputation = 0
   bounty_setup()

   -- Set mission details
   misn.setTitle( misn_title:format( misn_level[level], missys:name() ) )
   misn.setDesc( misn_desc:format( name, missys:name() ) )
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
   spawn_pirate( last_sys )
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
      if target_killed then
         tk.msg( pay_title, pay_kill_text[ rnd.rnd( 1, #pay_kill_text ) ] )
      else
         tk.msg( pay_title, pay_capture_text[ rnd.rnd( 1, #pay_capture_text ) ] )
      end
      misn.finish( true )
   end
end


function pilot_board ()
   player.unboard()
   local t = subdue_text[ rnd.rnd( 1, #subdue_text ) ]:format( name )
   tk.msg( subdue_title, t )
   succeed()
   target_killed = false
   target_ship:control()
   target_ship:setHilight( false )
   if death_hook ~= nil then hook.rm( death_hook ) end
end


function pilot_death( p, attacker )
   if attacker == player.pilot() then
      succeed()
      target_killed = true
   else
      fail( msg[2]:format( name ) )
   end
end


function pilot_jump ()
   fail( msg[1]:format( name ) )
end


-- Set up the ship, credits, and reputation based on the level.
function bounty_setup ()
   if level == 1 then
      ship = "Pirate Hyena"
      credits = 50000 + rnd.sigma() * 15000
      reputation = 1
   elseif level == 2 then
      ship = "Pirate Shark"
      credits = 100000 + rnd.sigma() * 30000
      reputation = 1
   elseif level == 3 then
      if rnd.rnd() < 0.5 then
         ship = "Pirate Vendetta"
      else
         ship = "Pirate Ancestor"
      end
      credits = 200000 + rnd.sigma() * 60000
      reputation = 2
   elseif level == 4 then
      if rnd.rnd() < 0.5 then
         ship = "Pirate Admonisher"
      else
         ship = "Pirate Phalanx"
      end
      credits = 500000 + rnd.sigma() * 150000
      reputation = 3
   elseif level == 5 then
      ship = "Pirate Kestrel"
      credits = 1000000 + rnd.sigma() * 300000
      reputation = 5
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
         death_hook = hook.pilot( target_ship, "death", "pilot_death" )
         hook.pilot( target_ship, "jump", "pilot_jump" )
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
end


-- Fail the mission, showing message to the player.
function fail( message )
   if message ~= nil then
      player.msg( message )
   end
   misn.finish( false )
end
