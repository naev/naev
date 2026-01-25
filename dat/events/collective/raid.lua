--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Collective Raid">
 <location>enter</location>
 <chance>4</chance>
 <chapter>[^0]</chapter>
 <cond>
local scur = system.cur()
return system.get("Hades"):jumpDist( scur ) &lt;= 2 and scur:faction() == faction.get("Empire") and not diff.isApplied("collective_dead") and not player.evtActive("Collective Raid")
 </cond>
</event>
--]]
--[[
   Some drones from the Collective show up to bully everyone.
--]]
local careful = require "ai.core.misc.careful"
local lanes = require "ai.core.misc.lanes"

function create()
   local scur = system.cur()
   if not evt.claim( {scur}, true ) then evt.finish(false) end
   hook.land("leave")
   hook.jumpout("leave")
   hook.timer( rnd.rnd(4, 20), "fun")
   if rnd.rnd < 0.27 then hook.timer( rnd.rnd(40, 110), "fun") end -- Go again!! (small chance)
   if rnd.rnd < 0.09 then hook.timer( rnd.rnd(90, 200), "fun") end -- Go again!!!! (smaller chance)
end

function fun()
   local scur = system.cur()
   local sfct = scur:faction()
   local jmps = scur:jumps()
   local mode = rnd.rnd(1)
   if mode == 0 then -- Go in loud in one big clump
      local jmp = jmps[ rnd.rnd( #jmps )]
      for i = 1,rnd.rnd(7,26) do
         pilot.add('Drone', 'Collective', jmp, nil )
      end
      for i = 1,rnd.rnd(2,4) do
         pilot.add('Heavy Drone', 'Collective', jmp, nil )
      end
   elseif mode == 1 then -- Spawned in stealth, like space mines
      local L = lanes.get( sfct, "non-hostile" )
      for i = 1,rnd.rnd(18,31) do
         local rad = scur:radius()*0.81*rnd.rnd()^0.62
         local pos = careful.getSafePointL( L, nil, vec2.new(), rad, 2e3, 2e3, 2e3 )
         if pos then
            pilot.add('Drone', 'Collective', pos, nil, {stealth=true} )
         end
      end
      for i = 1,rnd.rnd(4,6) do
         local rad = scur:radius()*0.86*rnd.rnd()^0.66
         local pos = careful.getSafePointL( L, nil, vec2.new(), rad, 2e3, 2e3, 2e3 )
         if pos then
            pilot.add('Heavy Drone', 'Collective', pos, nil, {stealth=true} )
         end
      end
   end
end

function leave ()
   evt.finish()
end
