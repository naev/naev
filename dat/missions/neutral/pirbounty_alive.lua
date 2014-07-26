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
      credits = 75000 + rnd.sigma() * 22500
      reputation = 1
   elseif level == 2 then
      ship = "Pirate Shark"
      credits = 150000 + rnd.sigma() * 45000
      reputation = 1
   elseif level == 3 then
      if rnd.rnd() < 0.5 then
         ship = "Pirate Vendetta"
      else
         ship = "Pirate Ancestor"
      end
      credits = 300000 + rnd.sigma() * 90000
      reputation = 2
   elseif level == 4 then
      if rnd.rnd() < 0.5 then
         ship = "Pirate Admonisher"
      else
         ship = "Pirate Phalanx"
      end
      credits = 750000 + rnd.sigma() * 225000
      reputation = 3
   elseif level == 5 then
      ship = "Pirate Kestrel"
      credits = 1500000 + rnd.sigma() * 450000
      reputation = 5
   end
end
