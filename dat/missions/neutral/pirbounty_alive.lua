--[[

   Alive Pirate Bounty
   Copyright 2014, 2015 Julie Marchant

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

include "numstring.lua"
include "dat/missions/neutral/pirbounty_dead.lua"

-- Localization
lang = naev.lang()
if lang == "es" then
else -- Default to English
   pronoun = rnd.rnd() < 0.5 and "He" or "She"

   kill_instead_title   = "Better Dead than Free"
   kill_instead_text    = {}
   kill_instead_text[1] = [[As you return to your ship, you are contacted by an officer. "I see you were unable to capture %s," the officer says. "Disappointing. However, we would rather this pirate be dead than roaming free, so you will be paid %s credits if you finish him off right now."]]
   kill_instead_text[2] = [[On your way back to your ship, you receive a message from an officer. It reads, "Your failure to capture %s is disappointing. We really wanted to capture this pirate alive. However, we would rather he be dead than roaming free, so if you kill him now, you will be paid the lesser sum of %s credits."]]
   kill_instead_text[3] = [[When you return to your cockpit, you are contacted by an officer. "Pathetic! If I were in charge, I'd say you get no bounty! Can't fight off a couple low-life pirates?!" He sighs. "But lucky for you, I'm not in charge, and my higher-ups would rather %s be dead than free. So if you finish him off, you'll get %s credits. Just be snappy about it!" And with that, the officer ceases communication.]]
   kill_instead_text[4] = [[When you get back to the ship, you see a message giving you a new mission to kill %s; the reward is %s credits. Well, that's pitiful compared to what you were planning on collecting, but it's better than nothing.]]

   pay_capture_text    = {}
   pay_capture_text[1] = "An officer takes %s into custody and hands you your pay."
   pay_capture_text[2] = "The officer seems to think your acceptance of the alive bounty for %s was insane. " .. pronoun .. " carefully takes the pirate off your hands, taking precautions you think are completely unnecessary, and then hands you your pay."
   pay_capture_text[3] = "The officer you deal with seems to especially dislike %s. " .. pronoun .. " takes the pirate off your hands and hands you your pay without speaking a word."
   pay_capture_text[4] = "A fearful-looking officer rushes %s into a secure hold, pays you the appropriate bounty, and then hurries off."
   pay_capture_text[5] = "The officer you deal with thanks you profusely for capturing %s alive, pays you, and sends you off."
   pay_capture_text[6] = "Upon learning that you managed to capture %s alive, the previously depressed-looking officer suddenly brightens up. " .. pronoun .. " takes the pirate into custody and hands you your pay."
   pay_capture_text[7] = "When you ask the officer for your bounty on %s, " .. pronoun:lower() .. " sighs, takes the pirate into custody, goes through some paperwork, and hands you your pay, mumbling something about how useless capturing pirates alive is."

   pay_kill_text    = {}
   pay_kill_text[1] = "After verifying that you killed %s, an officer hands you your pay."
   pay_kill_text[2] = "After verifying that %s is indeed dead, the officer sighs and hands you your pay."
   pay_kill_text[3] = "This officer is clearly annoyed that %s is dead. " .. pronoun .. " mumbles something about incompetent bounty hunters the entire time as " .. pronoun:lower() .. " takes care of the paperwork and hands you your bounty."
   pay_kill_text[4] = "The officer seems disappointed, yet unsurprised that you failed to capture %s alive. " .. pronoun .. " hands you your lesser bounty without speaking a word."
   pay_kill_text[5] = "When you ask the officer for your bounty on %s, he sighs, leads you into his office, goes through some paperwork, and hands you your pay, mumbling something about how useless bounty hunters are."
   pay_kill_text[6] = "The officer verifies the death of %s, goes through the necessary paperwork, and hands you your pay, looking annoyed the entire time."

   fail_kill_text = "MISSION FAILURE! %s has been killed."

   misn_title  = "%s Alive Bounty in %s"
   misn_desc   = "The pirate known as %s was recently seen in the %s system. %s authorities want this pirate alive."

   osd_msg[2] = "Capture %s"
   osd_msg_kill = "Kill %s"
end


function pilot_death ()
   if board_failed then
      succeed()
      target_killed = true
   else
      fail( fail_kill_text:format( name ) )
   end
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


function board_fail ()
   if rnd.rnd() < 0.25 then
      board_failed = true
      credits = credits / 5
      local t = kill_instead_text[ rnd.rnd( 1, #kill_instead_text ) ]:format(
         name, numstring( credits ) )
      tk.msg( kill_instead_title, t )
      osd_msg[2] = osd_msg_kill:format( name )
      misn.osdCreate( osd_title, osd_msg )
      misn.osdActive( 2 )
   end
end
