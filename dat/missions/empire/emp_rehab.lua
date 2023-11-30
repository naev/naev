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
} )
