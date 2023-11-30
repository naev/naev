--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Rehabilitation">
 <priority>10</priority>
 <cond>faction.playerStanding("Dvaered") &lt; 0</cond>
 <chance>100</chance>
 <location>Computer</location>
</mission>
--]]
--[[
  Rehabilitation Mission
--]]
require("common.rehab").init( faction.get("Dvaered"), {
   desc = _([[Although you have become an enemy of House Dvaered through your despicable actions, Dvaered High Command is willing to give you a second opportunity to atone for your actions. Through a one-time payment, the warlords are willing to overlook your past offenses and give you a chance to recover their trust. However, please note that ANY hostilities against House Dvaered or aiding enemies of House Dvaered will render this agreement void.]]),
   txtaccept = _([[Your application has been processed. House Dvaered forces will no longer attack you on sight. While you may now freely conduct business in House Dvaered space, remember that your criminal record remains in tact. Any violation of the agreement, such as hostilities against House Dvaered or aiding enemies such as the terrorist organization Frontier Liberation Front will immediately void the agreement and return you to your previous criminal record.]]),
   txtsuccess = _([[Dvaered High Command commends you on your efforts to recover your relationship with House Dvaered, and have cleared your previous offense record with the House.]]),
   txtabort = _([[You have once again crossed House Dvaered. Dvaered High Command has revoked your rehabilitation procedure, and your standing has returned to before the agreement. You may start another procedure at a later time.]]),
} )
