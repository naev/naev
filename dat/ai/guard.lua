require("ai/tpl/generic")

--[[

   Guard Mission AI. Have to set mem.guardpos to the position to guard.

--]]

-- Settings
mem.aggressive    = true
mem.atk_kill      = true
mem.atk_board     = false
mem.bribe_no      = _("You can't bribe me!")
mem.refuel_no     = _("I won't give you fuel!")
mem.doscans       = true
mem.guardpos      = vec2.new( 0, 0 ) -- defaults to origin
mem.guardbrake    = 500
mem.guarddodist   = 3000 -- distance at which to start activities
mem.guardreturndist = 6000 -- distance at which to return
mem.enemyclose    = mem.guarddodist

function create ()
   -- Choose attack format
   attack_choose()

   -- Finish up creation
   create_post()
end

local function gdist( t )
   return mem.guardpos:dist( t:pos() )
end

-- Default task to run when idle
function idle ()
   -- Aggressives will try to find enemies first, before falling back on
   -- loitering, to avoid weird stuff starting to scan before attacking
   if mem.aggressive then
      local enemy  = ai.getenemy()
      if enemy ~= nil and gdist(enemy) < mem.guarddodist then
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
   end

   -- Then do normal control
   control_generic()
end

