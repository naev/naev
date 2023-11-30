--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="FLF Rehabilitation">
 <priority>10</priority>
 <cond>faction.playerStanding("FLF") &lt; 0</cond>
 <chance>100</chance>
 <faction>Frontier</faction>
 <faction>FLF</faction>
 <location>Computer</location>
</mission>
--]]
--[[
   Rehabilitation Mission
--]]
require("common.rehab").init( faction.get("FLF"), {
   ignoreenemies = true, -- Always available
   desc = _([[Although you have crossed the Frontier Liberation Front, the organization is willing to offer a chance to redeem yourself through a significant donation. If you accept, your reputation will be temporarily changed to neutral until you recover all the lost standing. Any hostile action during this time against the FLF will immediately void the agreement.]]),
   txtbroke = _([[You need to donate at least {credits} for the FLF to temporarily forgive you.]]),
   txtaccept = _([[Thank you for supporting the FLF. Remember that any collaboration with the dirty Dvaered or actions against the FLF will immediately void this agreement. During this time if you gain enough standing with the FLF, your past offenses will be erased from our records.]]),
   txtsuccess = _([[You have successfully cleared past negative record with the FLF and are in good standing again.]]),
   txtabort = _([[You have once again crossed the FLF! The truce agreement is void and you are once again in bad standing. You may do another donation to restart the procedure at a later time.]]),
} )
