--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Empire Rehabilitation">
  <avail>
   <priority>10</priority>
   <cond>faction.playerStanding("Empire") &lt; 0</cond>
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

require "missions/rehab_common.lua"

fac = faction.get("Empire")