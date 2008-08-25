include("ai/tpl/generic.lua")

-- Settings
armour_run = 40
armour_return = 70
aggressive = true

function create ()
   -- Escorts have nothingon them
end

-- When pilot is idle he'll escort the player
function idle ()
   ai.pushtask(0, "escort", ai.getPlayer())
end

-- Escorts the target
function escort ()
   target = ai.targetid()
   
   dir = ai.face(target)
   dist = ai.dist( ai.pos(target) )

   if dir < 10 and dist > 200 then
      ai.accel()
   end
end
