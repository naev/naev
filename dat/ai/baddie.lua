require 'ai.core.core'
require 'ai.core.idle.wait'

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
mem.bribe_no       = _([["You can't bribe me!"]])
mem.refuel_no      = _([["I won't give you fuel!"]])

function create ()
   create_pre()
   local p = ai.pilot()
   local ps = p:ship()
   local pt = ps:tags()
   mem.isdrone = pt.drone
   if mem.isdrone then
      mem.comm_no = _("ACCESS DENIED.")
   end
   create_post()
end
