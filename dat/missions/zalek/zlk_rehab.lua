--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Rehabilitation">
 <priority>10</priority>
 <cond>faction.playerStanding("Za'lek") &lt; 0</cond>
 <chance>100</chance>
 <location>Computer</location>
</mission>
--]]
--[[
   Rehabilitation Mission
--]]
require("common.rehab").init( faction.get("Za'lek"), {
   desc = _([[Although your irrational actions have made you an enemy of House Za'lek, however, House Za'lek does not close the door, and, as recommended by the Za'lek Sociologist work group, is willing to allow you to pay reparations for your rehabilitation into Za'lek society. This will temporarily restore your status with House Za'lek to neutral, and as long as you do not further pursue irrational actions against House Za'lek, will let you restore your status. Note that any irrational and hostile action will void the agreement.]]),
   txtaccept = _([[Thank you for doing the rational thing. House Za'lek security forces and automated drone patrols will no longer target your vessels. You are free to travel House Za'lek space, however, remember that this does not rectify your previous irrational acts. Any new irrational act or crime against House Za'lek will immediately reset your status to enemy.]]),
   txtsuccess = _([[Congratulations for returning to the side of rationality! Your standing with House Za'lek has been reset to neutral, and all your previous irrational acts have been scrubbed from the databases. It is time for you to continue as a rational being.]]),
   txtabort = _([[Your irrationality has let down House Za'lek. You are no longer considered neutral, and automated drones and defense forces will target you on sight. You may repeat the rehabilitation course at a later time if you wish to return to rationality.]]),
} )
