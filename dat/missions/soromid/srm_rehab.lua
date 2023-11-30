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
} )
