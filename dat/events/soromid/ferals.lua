--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Hypergate Construction">
 <location>enter</location>
 <chance>100</chance>
 <cond>system.cur():tags().haze==true</cond>
 <chapter>0</chapter>
</event>
--]]

--luacheck: globals leave pheromones spawn_ferals (Hook functions passed by name)

function create ()
   local scur = system.cur()
   evt.finish() -- Disabled for now

   -- Inclusive claim
   if not evt.claim( scur, nil, true ) then evt.finish() end

   -- Must be uninhabited
   for k,p in ipairs(scur:spobs()) do
      local s = p:services()
      if s.land and s.inhabited then
         evt.finish(false)
         return
      end
   end

   hook.jumpout("leave")
   hook.land("leave")
   hook.custom("bioship_pheromones", "pheromones")
end

--event ends on player leaving the system or landing
function leave ()
    evt.finish()
end

local spawned = false
function pheromones ()
   if not spawned then
      spawned = true
      hook.timer( 5, "spawn_ferals" )
   end
end

function spawn_ferals ()
end
