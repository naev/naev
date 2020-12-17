require("ai/tpl/generic")

-- Don't run away from master ship
mem.norun = true


-- Simple create function
function create ()
   mem.carrier = true
   attack_choose()

   -- Disable thinking
   mem.atk_think = nil
end

-- Just tries to guard mem.escort
function idle ()
   ai.pushtask("follow_fleet")
end
