--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Wild Space Chaos">
 <location>enter</location>
 <chance>100</chance>
 <cond>system.cur():tags().wildspace ~= nil</cond>
</event>
--]]

--[[
   Event for Wild Space systems. Ships will occasional change factions and kill each other for no real reason.

   ...And weird derelicts.
--]]
local vntk = require "vntk"

local flost = faction.get("Lost")
local fnewlost
function create ()
   -- TODO add derelicts

   -- TODO make the lost drop scraps?

   fnewlost = faction.dynAdd( flost, "newlost" )
   fnewlost:dynEnemy( flost )
   hook.timer( 15 + 20*rnd.rnd(), "chaos" )

   hook.pilot( nil, "boardall", "lost_boarded" )
end

function chaos ()
   local plts = {}
   for k,p in ipairs(pilot.get( {flost, fnewlost} )) do
      if not p:mothership() then
         table.insert( plts, p )
      end
   end

   local p = plts[ rnd.rnd(1,#plts) ]
   if p then
      p:setLeader() -- Clear leader in case deployed
      p:taskClear()
      -- Flip faction
      if p:faction() == flost then
         p:setFaction( fnewlost )
      else
         p:setFaction( flost )
      end
   end
   hook.timer( 15 + 20*rnd.rnd(), "chaos" )
end

function lost_boarded( boarded, boarder )
   local bf = boarder:faction()
   if bf==flost or bf==fnewlost then
      -- May potentially not die if gets regenerated
      boarded:kill()
      if boarded == player.pilot() then
         vntk(_([[Not Like This]]),_([[Upon being boarded by the Lost, you quickly learn that they do not spare anyone.]]))
         player.gameover()
      end
   end
end
