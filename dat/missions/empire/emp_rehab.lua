--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Empire Rehabilitation">
 <priority>10</priority>
 <cond>faction.playerStanding("Empire") &lt; 0</cond>
 <chance>100</chance>
 <location>Computer</location>
</mission>
--]]
--[[
   Rehabilitation Mission
--]]
require("common.rehab").init( faction.get("Empire"), {
   desc = _([[The Empire is giving you a chance to rejoin society by paying a fine and filling the appropriate paperwork. This will allow you to do business in the Empire as a neutral pilot, however, you still have to recover the trust of the Empire. Furthermore, ANY hostilities against the Empire will immediately void the paperwork. Note that this rehabilitation only affects the Empire itself, not any of the Houses.]]),
   txtaccept = _([[Your paperwork and payment has been processed. Imperial Patrols will no longer attack you on sight, and you may travel freely in Imperial space. However, remember that this does not clear you of your past responsibilities with the Empire. Any violation of this treaty such as piracy will immediately void the agreement.]]),
   txtsuccess = _([[Congratulations! You have completed the requisites for becoming a productive member of the Empire. With this final paperwork, your past offenses have been erased.]]),
   txtabort = _([[You have violated your agreement with the Empire! Your rehabilitation procedure has been immediately revoked, and your reputation is once again tarnished! You may start another rehabilitation procedure at a later time.]]),
} )
