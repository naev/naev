--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Trader Rehabilitation">
  <avail>
   <priority>10</priority>
   <cond>faction.playerStanding("Trader") &lt; 0</cond>
   <chance>100</chance>
   <location>Computer</location>
  </avail>
 </mission>
 --]]
--[[
--
-- Rehabilitation Mission
--
--]]

require "missions/rehab_common"

fac = faction.get("Traders Guild")
