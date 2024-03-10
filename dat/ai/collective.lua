require 'ai.core.core'

-- Settings
mem.control_rate = 0.5 -- Lower control rate
mem.aggressive = true
mem.land_planet = false
mem.doscans = false

function create ()
   create_pre()
   mem.comm_no = _("No response.")
   create_post()
end
