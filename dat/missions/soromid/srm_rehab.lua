--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Soromid Rehabilitation">
  <avail>
   <priority>10</priority>
   <cond>faction.playerStanding("Soromid") &lt; 0</cond>
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

require "dat/missions/rehab_common.lua"

fac = faction.get("Soromid")