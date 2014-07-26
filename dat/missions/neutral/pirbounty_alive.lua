--[[

   Alive Pirate Bounty

   Bounty mission where you must capture the target alive.
   Can work with any faction.

   Author: onpon4

--]]

include "dat/missions/neutral/pirbounty_dead.lua"

-- Localization
lang = naev.lang()
if lang == "es" then
else -- Default to English
   fail_kill_text = "MISSION FAILURE! %s has been killed."

   misn_title  = "%s Alive Bounty in %s"
   misn_desc   = "The pirate known as %s was recently seen in the %s system. This pirate is wanted alive."

   osd_msg[2] = "Capture %s"
end


function pilot_death ()
   fail( fail_kill_text:format( name ) )
end


-- Set up the ship, credits, and reputation based on the level.
function bounty_setup ()
   if level == 1 then
      ship = "Pirate Hyena"
      credits = 100000 + rnd.sigma() * 30000
      reputation = 0
   elseif level == 2 then
      ship = "Pirate Shark"
      credits = 200000 + rnd.sigma() * 60000
      reputation = 1
   elseif level == 3 then
      if rnd.rnd() < 0.5 then
         ship = "Pirate Vendetta"
      else
         ship = "Pirate Ancestor"
      end
      credits = 400000 + rnd.sigma() * 120000
      reputation = 3
   elseif level == 4 then
      if rnd.rnd() < 0.5 then
         ship = "Pirate Admonisher"
      else
         ship = "Pirate Phalanx"
      end
      credits = 1000000 + rnd.sigma() * 300000
      reputation = 5
   elseif level == 5 then
      ship = "Pirate Kestrel"
      credits = 2000000 + rnd.sigma() * 600000
      reputation = 7
   end
end
