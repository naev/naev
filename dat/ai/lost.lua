require 'ai.core.core'
local atk = require "ai.core.attack.util"

-- Settings
mem.aggressive     = true
mem.safe_distance  = 2000
mem.armour_run     = 10
mem.armour_return  = 30
mem.atk_kill       = true
mem.atk_board      = false
local msg = _([[You only hear static.]])
mem.bribe_no       = msg
mem.refuel_no      = msg
mem.comm_no        = msg

function drift ()
   if ai.timeup(0) then
      ai.poptask()
   end
end

function idle ()
   local enemy = atk.preferred_enemy( nil, true )
   if enemy ~= nil and should_attack( enemy ) then
      ai.pushtask( "attack", enemy )
      return
   end

   local r = rnd.rnd()
   if r < 0.5 then
      ai.settimer(0 )
      ai.pushtask("drift")
   elseif r < 0.9 then
      -- Choose random point and choose a position between there and current position
      local pos = vec2.newP( rnd.rnd()*system.cur():radius(), rnd.angle() )
      local m = rnd.rnd()
      pos = pos * m + ai.pilot():pos() * (1-m)
      ai.pushtask("moveto_nobrake_raw", pos)
   else
      if rnd.rnd() < 0.5 then
         local spb = ai.landspob(true)
         if spb then
            ai.pushtask("land", spb)
         end
      end
      ai.pushtask("land", mem.goal_planet)
      local tgt = ai.rndhyptarget()
      if tgt then
         ai.pushtask("hyperspace", tgt )
      end
   end
end

function create ()
   create_pre()
   create_post()
end
