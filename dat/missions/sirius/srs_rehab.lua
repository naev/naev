--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Sirius Rehabilitation">
 <priority>10</priority>
 <cond>faction.playerStanding("Sirius") &lt; 0</cond>
 <chance>100</chance>
 <location>Computer</location>
</mission>
--]]
--[[
   Rehabilitation Mission
--]]
require("common.rehab").init( faction.get("Sirius"), {
   desc = _([[Although you have committed destructive actions against House Sirius, the Tribunal of Arch-Canters is willing to give you another opportunity if you swear an oath to cease harming House Sirius and collaborate to improve relationships. By swearing the oath, your reputation will be temporarily restored to neutral, however, any hostile action against House Sirius will immediately restore your standing to the current value.]]),
   txtaccept = _([[By swearing the oath, House Sirius forces will no longer view you as an enemy. However, remember that this does not atone you of your past sins against House Sirius. Furthermore, any new hostile action against House Sirius will immediately void your oath, and restore your standing to the previous value.]]),
   txtsuccess = _([[The Tribunal of Arch-Canters lauds your efforts to atone for your past sins against House Sirius. Your standing has been restored to a clean slate for you to continue making Sirichana smile.]]),
   txtabort = _([[You have committed a sin against House Sirius! Your oath has been immediately made void and you are once again an enemy of the House. You still can perform another oath at a later time.]]),
} )
