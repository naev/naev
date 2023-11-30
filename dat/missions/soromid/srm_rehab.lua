--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Soromid Rehabilitation">
 <priority>10</priority>
 <cond>faction.playerStanding("Soromid") &lt; 0</cond>
 <chance>100</chance>
 <location>Computer</location>
</mission>
--]]
--[[
   Rehabilitation Mission
--]]
require("common.rehab").init( faction.get("Soromid"), {
   desc = _([[Although your actions have offended the Soromid Tribes, the Tribal Council finds it in its heart to forgive you for your past actions for a price. The ritual will restore your faction standing as if your reputation were neutral, and allow you to regain the trust of the Tribes. Note that any offense towards the Soromid during this time will immediately nullify the ritual and cause you to be once more an enemy of the Soromid.]]),
   txtaccept = _([[The ritual is underway. Tribal patrols will no longer attack your ship, and you can wander among the Soromid stars. However, this does not expiate you of past transgressions. ANY action against the interest of any of the Soromid Tribes will result in the nullification of the ritual and your newly found privileges.]]),
   txtsuccess = _([[Congratulations Wanderer! The Soromid Tribal Councils has found consensus in atoning you of your past transgression and welcoming you once more as a wanderer of Soromid space.]]),
   txtabort = _([[You have transgressed against the Soromid Tribes and the rehabilitation ritual is nullified! Your presence is no longer welcome in Soromid Space. However, it should be possible to initialize a new rehabilitation ritual at a later time.]]),
} )
