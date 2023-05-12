--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Alive Bounty">
 <priority>4</priority>
 <cond>
   return require("misn_test").mercenary()
 </cond>
 <chance>360</chance>
 <location>Computer</location>
 <faction>Empire</faction>
 <faction>Frontier</faction>
 <faction>Goddard</faction>
 <faction>Independent</faction>
 <faction>Sirius</faction>
 <faction>Soromid</faction>
 <faction>Za'lek</faction>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   Alive Pirate Bounty

   Bounty mission where you must capture the target alive.
   Can work with any faction.

--]]
local fmt = require "format"
local lmisn = require "lmisn"
local vntk = require "vntk"
require "missions.neutral.pirbounty_dead"

-- luacheck: globals pay_capture_text pay_kill_text misn_title pilot_death succeed bounty_setup board_fail (inherited from mission above, probably best to clean up )

local kill_instead_text = {
   _([[As you return to your ship, you are contacted by an officer. "I see you were unable to capture {plt}," the officer says. "Disappointing. However, we would rather this pirate be dead than roaming free, so you will be paid {credits} if you finish them off right now."]]),
   _([[On your way back to your ship, you receive a message from an officer. It reads, "Your failure to capture {plt} is disappointing. We really wanted to capture this pirate alive. However, we would rather he be dead than roaming free, so if you kill the pirate now, you will be paid the lesser sum of {credits}."]]),
   _([[When you return to your cockpit, you are contacted by an officer. "Pathetic! If I were in charge, I'd say you get no bounty! Can't fight off a couple low-life pirates?!" He sighs. "But lucky for you, I'm not in charge, and my higher-ups would rather {plt} be dead than free. So if you finish that scum off, you'll get {credits}. Just be snappy about it!" Having finished delivering the message, the officer then ceases communication.]]),
   _([[When you get back to the ship, you see a message giving you a new mission to kill {plt}; the reward is {credits}. Well, that's pitiful compared to what you were planning on collecting, but it's better than nothing.]]),
}

pay_capture_text = {
   _("An officer takes {plt} into custody and hands you your pay."),
   _("The officer seems to think you were foolish to accept a live bounty for {plt}. They carefully take the pirate off your hands, taking precautions you think are completely unnecessary, and then hand you your pay."),
   _("The officer you deal with seems to especially dislike {plt}. They take the pirate off your hands and hand you your pay without speaking a word."),
   _("A fearful-looking officer rushes {plt} into a secure hold, pays you the appropriate bounty, and then hurries off."),
   _("The officer you deal with thanks you profusely for capturing {plt} alive, pays you, and sends you off."),
   _("Upon learning that you managed to capture {plt} alive, the officer who previously sported a defeated look suddenly brightens up. The pirate is swiftly taken into custody as you are handed your pay."),
   _("When you ask the officer for your bounty on {plt}, they sigh, take the pirate into custody, go through some paperwork, and hand you your pay, mumbling something about how useless capturing pirates alive is."),
}

pay_kill_text = {
   _("After verifying that you killed {plt}, an officer hands you your pay."),
   _("After verifying that {plt} is indeed dead, the officer sighs and hands you your pay."),
   _("This officer is clearly annoyed that {plt} is dead. They mumble something about incompetent bounty hunters the entire time as they take care of the paperwork and hand you your bounty."),
   _("The officer seems disappointed, yet unsurprised, that you failed to capture {plt} alive. You are handed your lesser bounty without a word."),
   _("When you ask the officer for your bounty on {plt}, they sigh, lead you into an office, go through some paperwork, and hand you your pay, mumbling something about how useless bounty hunters are."),
   _("The officer verifies the death of {plt}, goes through the necessary paperwork, and hands you your pay, looking annoyed the entire time."),
}

misn_title = {
   _("Tiny Alive Bounty in {sys}"),
   _("Small Alive Bounty in {sys}"),
   _("Moderate Alive Bounty in {sys}"),
   _("High Alive Bounty in {sys}"),
   _("Dangerous Alive Bounty in {sys}"),
}

mem.misn_desc = _([[The pirate known as {pirname} was recently seen in the {sys} system. {fct} authorities want this pirate alive. {pirname} is believed to be flying a {shipclass}-class ship. The pirate may disappear if you take too long to reach the {sys} system.

#nTarget:#0 {pirname} ({shipclass}-class ship)
#nWanted:#0 Alive
#nLast Seen:#0 {sys} system]])

mem.osd_msg[2] = _("Capture {plt}")

function pilot_death ()
   if mem.board_failed then
      succeed()
      mem.target_killed = true
   else
      lmisn.fail( fmt.f( _("{plt} has been killed."), {plt=mem.name} ) )
   end
end


local _bounty_setup = bounty_setup -- Store original one
-- Set up the ship, credits, and reputation based on the level.
function bounty_setup ()
   local pship, credits, reputation = _bounty_setup()
   credits = credits * 1.5
   reputation = reputation * 1.5
   return pship, credits, reputation
end


function board_fail ()
   if rnd.rnd() < 0.25 then
      mem.board_failed = true
      mem.credits = mem.credits / 5
      local t = fmt.f( kill_instead_text[ rnd.rnd( 1, #kill_instead_text ) ],
         {plt=mem.name, credits=fmt.credits(mem.credits)} )
      vntk.msg( _("Better Dead than Free"), t )
      mem.osd_msg[2] = fmt.f( _("Kill {plt}"), {plt=mem.name} )
      misn.osdCreate( mem.osd_title, mem.osd_msg )
      misn.osdActive( 2 )
   end
end
