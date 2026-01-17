local constants = {
   -- Constants used by the Engine
   PHYSICS_SPEED_DAMP   = 4, -- Was 3 until 0.13.0
   STEALTH_MIN_DIST     = 1000., -- Minimum distance a ship can stealth at (excluding system factors and ship stat modifiers)
   SHIP_MIN_MASS        = 0.5, -- Minimum amount of the ship mass that can reach (was 0 until 0.13.0)
   AUDIO_REF_DISTANCE   = 750, -- Was 500 until 0.13.0, but used inverse model before (now linear)
   AUDIO_MAX_DISTANCE   = 7.5e3, -- Was 25e3 until 0.13.0, but used the inverse model before (now linear)
   EW_JUMP_BONUS_RANGE  = 2500, -- Range from jump at which a stealth bonus is applied
   EW_ASTEROID_DIST     = 7.5e3, -- Range at which an asteroid with hide==1 is detected
   EW_JUMPDETECT_DIST   = 7.5e3, -- Range at which a jump with hide==1 is detected
   EW_SPOBDETECT_DIST   = 20e3, -- Range at which a spob with hide==1 is detected

   PILOT_SHIELD_DOWN_TIME = 5, -- Time shield is down after being knocked down to 0
   PILOT_STRESS_RECOVERY_TIME = 5, -- Time for stress to start recovering after taking disable damage (was 0 before 0.14.0)
   PILOT_DISABLED_ARMOUR= 0.1, -- Armour rate at which the ship is disabled (was 0 before 0.13.0)

   CAMERA_ANGLE         = math.pi/4, -- Camera angle, math.pi/2 would be overhead, math.pi/4 is isometric

   -- Constants used by multiple Lua scripts
   BITE_ACCEL_MOD = 390, -- like hades_torch (was 500 before 0.13.0)
   BITE_SPEED_MOD = 110, -- like adrenal_glands_ii (was 80 before 0.13.0)
}

local cts_list = {}
for k,v in ipairs(file.enumerate( "constants")) do
   local cts = require("constants."..string.gsub(v,".lua",""))()
   cts.priority = cts.priority or 5
   table.insert( cts_list, cts )
end
table.sort( cts_list, function( a, b )
   return a.priority > b.priority
end )
for k,cts in ipairs(cts_list) do
   constants = tmerge( constants, cts )
end

return constants
