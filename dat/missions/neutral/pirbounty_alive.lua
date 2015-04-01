--[[

   Alive Pirate Bounty
   Copyright 2014 Julian Marchant

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

--

   Bounty mission where you must capture the target alive.
   Can work with any faction.

--]]

include "dat/missions/neutral/pirbounty_dead.lua"

-- Localization
lang = naev.lang()
if lang == "es" then
else -- Default to English
   fail_kill_text = "MISSION FAILURE! %s has been killed."

   misn_title  = "%s Alive Bounty in %s"
   misn_desc   = "The pirate known as %s was recently seen in the %s system. %s authorities want this pirate alive."

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
      credits = 300000 + rnd.sigma() * 100000
      reputation = 1
   elseif level == 3 then
      if rnd.rnd() < 0.5 then
         ship = "Pirate Vendetta"
      else
         ship = "Pirate Ancestor"
      end
      credits = 800000 + rnd.sigma() * 160000
      reputation = 3
   elseif level == 4 then
      if rnd.rnd() < 0.5 then
         ship = "Pirate Admonisher"
      else
         ship = "Pirate Phalanx"
      end
      credits = 1400000 + rnd.sigma() * 240000
      reputation = 5
   elseif level == 5 then
      ship = "Pirate Kestrel"
      credits = 2500000 + rnd.sigma() * 500000
      reputation = 7
   end
end
