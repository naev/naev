require 'ai.core.core'

local constants = require "constants"
local PHYSICS_SPEED_DAMP = constants.PHYSICS_SPEED_DAMP
local BITE_ACCEL_MOD = constants.BITE_ACCEL_MOD
local BITE_SPEED_MOD = constants.BITE_SPEED_MOD

-- Settings
mem.armour_run    = 40
mem.armour_return = 70
mem.aggressive    = true
mem.formation     = "vee" -- TODO random
mem.atk_kill      = true
mem.atk_board     = false
mem.comm_no       = _("No response.")
mem.loiter        = math.huge

function mine_bite( ast )
   if not ast:exists() then
      ai.poptask()
      return
   end

   local p = ai.pilot()

   ai.setasterotarget( ast )

   local target = ast:pos()
   local vel = ast:vel()
   local _dist, angle = vec2.polar( p:pos() - target )

   -- First task : place the ship close to the asteroid
   local goal = ai.face_accurate( target, vel, 0, angle, mem.Kp, mem.Kd )

   local dir  = ai.face(goal)
   if dir < math.rad(30) then
      local dtime = 3
      if mem._o.bite_lust then
         dtime = dtime+2
      end
      if ai.dist( target ) < (
            p:speed() * (1.0+0.01*BITE_SPEED_MOD) +
            p:accel() * (1.0+0.01*BITE_ACCEL_MOD) / PHYSICS_SPEED_DAMP
         )*dtime then
         p:outfitToggle( mem._o.bite, true )
      end
   end

   if dir < math.rad(10) then
      ai.accel()
   end
end

local idle_old = idle
function idle ()
   if not mem._o or not mem._o.bite then
      if rnd.rnd() < 0.6 then
         return idle_old()
      else
         ai.settimer( 0, 2.0 )
         ai.pushtask( "brake" )
         ai.pushtask( "idle_wait" )
         return
      end
   end

   local r = rnd.rnd()
   if r < 0.3 then
      local ast = asteroid.get( mem.mining_field ) -- Get a random asteroid in the system (or current mining field)
      if ast then
         mem.mining_field = ast:field()
         ai.pushtask( "mine_bite", ast )
      end
   elseif r < 0.6 then
      ai.settimer( 0, 2.0 )
      ai.pushtask( "brake" )
      ai.pushtask( "idle_wait" )
   else
      return idle_old()
   end
end
