require 'ai.core.core'

--[[
   Generic Mission baddie AI
]]--

-- Settings
mem.aggressive     = true
mem.safe_distance  = 500
mem.armour_run     = 80
mem.armour_return  = 100
mem.atk_kill       = true
mem.atk_board      = false
mem.bribe_no       = _("You can't bribe me!")
mem.refuel_no      = _("I won't give you fuel!")

function create ()
   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Choose attack format
   create_post()
end
