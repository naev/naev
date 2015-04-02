--[[

   Alive Pirate Bounty
   Copyright 2014, 2015 Julian Marchant

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

   Bounty mission where you must capture the target alive.
   Can work with any faction.

--]]

include "dat/missions/neutral/pirbounty_dead.lua"

-- Localization
lang = naev.lang()
if lang == "es" then
else -- Default to English
   pay_capture_text    = {}
   pay_capture_text[1] = "An officer takes %s into custody and hands you your pay."
   pay_capture_text[2] = "The officer seems to think your acceptance of the alive bounty for %s was insane. " .. { "He", "She" }[rnd.rnd( 1, 2 )] .. " carefully takes the pirate off your hands, taking precautions you think are completely unnecessary, and then hands you your pay"
   pay_capture_text[3] = "The officer you deal with seems to especially dislike %s. " .. { "He", "She" }[rnd.rnd( 1, 2 )] .. " takes the pirate off your hands and hands you your pay without speaking a word."
   pay_capture_text[4] = "A fearful-looking officer rushes %s into a secure hold, pays you the appropriate bounty, and then hurries off."
   pay_capture_text[5] = "The officer you deal with thanks you profusely for capturing %s alive, pays you, and sends you off."
   pay_capture_text[6] = "Upon learning that you managed to capture %s alive, the previously depressed-looking officer suddenly brightens up. " .. { "He", "She" }[rnd.rnd( 1, 2 )] .. " takes the pirate into custody and hands you your pay."
   pay_capture_text[7] = "When you ask the officer for your bounty on %s, " .. { "he", "she" }[rnd.rnd( 1, 2 )] .. " sighs, takes the pirate into custody, goes through some paperwork, and hands you your pay, mumbling something about how useless capturing pirates alive is."

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
