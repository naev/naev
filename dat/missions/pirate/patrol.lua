--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Patrol">
 <avail>
  <priority>4</priority>
  <chance>560</chance>
  <location>Computer</location>
  <faction>Wild Ones</faction>
  <faction>Black Lotus</faction>
  <faction>Raven Clan</faction>
  <faction>Dreamer Clan</faction>
  <faction>Pirate</faction>
 </avail>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   Pirate Patrol

   Pirate version of the patrol mission.

--]]
local pir = require "common.pirate"
require "missions.neutral.patrol"

pay_text    = {}
pay_text[1] = _("The crime boss grins and hands you your pay.")
pay_text[2] = _("The local crime boss pays what you were promised, though not before trying (and failing) to pick your pocket.")
pay_text[3] = _("You are hit in the face with something and glare in the direction it came from, only to see the crime boss waving at you. When you look down, you see that it is your agreed-upon payment, so you take it and let out a grin.")
pay_text[4] = _("You are handed your pay in what seems to be a million different credit chips by the crime boss, but sure enough, it adds up to exactly the amount promised.")

abandon_text    = {}
abandon_text[1] = _("You are sent a message informing you that landing in the middle of the job is considered to be abandonment. As such, your contract is void and you will not receive payment.")


-- Mission details
misn_title  = _("#rPIRACY:#0 Patrol of the %s System")
misn_desc   = _("A local crime boss has offered a job to patrol the %s system in an effort to keep outsiders from discovering this Pirate stronghold. You will be tasked with checking various points and eliminating any outsiders along the way.")

-- Messages
msg    = {}
msg[1] = _("Point secure.")
msg[2] = _("Outsiders detected. Eliminate all outsiders.")
msg[3] = _("Outsiders eliminated.")
msg[4] = _("Patrol complete. You can now collect your pay.")
msg[5] = _("MISSION FAILURE! You showed up too late.")
msg[6] = _("MISSION FAILURE! You have left the %s system.")

osd_msg    = {}
osd_msg[1] = _("Fly to the {sys} system")
osd_msg[2] = "(null)"
osd_msg[3] = _("Eliminate outsiders")
osd_msg[4] = _("Land in %s territory to collect your pay")
osd_msg["__save"] = true

use_hidden_jumps = true

local create_original = create
function create ()
   paying_faction = pir.systemClanP()
   if pir.factionIsClan( paying_faction ) then
      misn_title = misn_title..string.format(_(" (%s)"), paying_faction:name() )
      misn_desc = misn_desc..pir.reputationMessage( paying_faction )
   end

   create_original()
end
