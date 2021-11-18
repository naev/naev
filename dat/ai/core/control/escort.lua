-- Don't run away from master ship
mem.norun = true
mem.carrier = true -- Is a carried fighter

-- Simple create function
function create ()
   attack_choose()

   -- Inherit some properties from the parent (leader)
   local p = ai.pilot()
   local l = p:leader()
   if l then
      local lmem = l:memory()
      mem.atk_kill = lmem.atk_kill
   end

   -- Disable thinking
   --mem.atk_think = nil
end

-- Just tries to guard mem.escort
-- luacheck: globals idle (AI Task functions passed by name)
function idle ()
   ai.pushtask("follow_fleet")
end
