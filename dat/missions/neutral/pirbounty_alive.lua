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
