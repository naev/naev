require("dat/ai/empire_idle.lua")
require("dat/ai/personality/patrol.lua")

-- Settings
mem.aggressive = false
mem.defensive  = false
mem.distressmsg = _("Empire refuel ship under attack!")

function create ()

   -- Broke
   ai.setcredits( 0 )

   -- Get refuel chance
   p = player.pilot()
   if p:exists() then
      mem.refuel = 0
      -- Most likely no chance to refuel
      mem.refuel_msg = _("\"Sure thing.\"")
   end

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   bribe_no = _("I'm out of here.")
end



