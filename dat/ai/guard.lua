require 'ai.core.core'

--[[

   Guard Mission AI. Have to set mem.guardpos to the position to guard.

--]]

-- Settings
mem.aggressive    = true
mem.atk_kill      = true
mem.atk_board     = false
mem.bribe_no      = _("You can't bribe me!")
mem.refuel_no     = _("I won't give you fuel!")
mem.guardpos      = vec2.new( 0, 0 ) -- defaults to origin
mem.guardbrake    = 500
mem.guarddodist   = 3000 -- distance at which to start activities
mem.guardreturndist = 6000 -- distance at which to return
mem.enemyclose    = mem.guarddodist

function create ()
   local p = ai.pilot()
   local ps = p:ship()

   -- Choose attack format
   attack_choose()

   -- Default range stuff
   mem.guardpos      = p:pos() -- Just guard current position
   mem.guarddodist   = 4000 + 1000 * ps:size()
   mem.guardreturndist = mem.guarddodist + 5000
   mem.enemyclose    = mem.guarddodist

   -- Finish up creation
   create_post()
end

local function gdist( t )
   return mem.guardpos:dist( t:pos() )
end

local _should_attack = should_attack
function should_attack( enemy, si )
   if _should_attack( enemy, si ) and gdist(enemy) < mem.guarddodist then
      return true
   end
   return false
end

-- Default task to run when idle
function idle ()
   -- Aggressives will try to find enemies first, before falling back on
   -- loitering, to avoid weird stuff starting to scan before attacking
   if mem.aggressive then
      local enemy  = ai.getenemy()
      if enemy ~= nil and should_attack(enemy) then
         ai.pushtask( "attack", enemy )
         return
      end
   end

   -- get distance
   local guarddist = ai.dist(mem.guardpos)
   if guarddist > mem.guardreturndist then
      -- Go back to Guard target
      ai.pushtask( "moveto", mem.guardpos )
      return
   end

   -- Scan if possible
   if mem.doscans then
      local target = __getscantarget()
      if target and gdist(target) < mem.guarddodist then
         __push_scan( target )
         return
      end
   end

   if guarddist < mem.guardbrake then
      ai.pushtask("brake" )
   else
      -- Just return
      ai.pushtask( "moveto", mem.guardpos )
   end
end

-- Override the control function
control_generic = control
function control ()
   if ai.dist(mem.guardpos) > mem.guardreturndist then
      -- Try to return
      ai.pushtask( "moveto", mem.guardpos )
      return
   end

   -- Then do normal control
   control_generic()
end

