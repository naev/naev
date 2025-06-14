return {
   -- Constants used by the Engine
   PHYSICS_SPEED_DAMP   = 4, -- Was 3 until 0.13.0
   STEALTH_MIN_DIST     = 1000., -- Minimum distance (excluding system factors)
   SHIP_MIN_MASS        = 0.5, -- Minimum amount of the ship mass it can reach (was 0 until 0.13.0)

   EW_JUMP_BONUS_RANGE  = 2500, -- Range from jump at which a stealth bonus is applied
   EW_ASTEROID_DIST     = 7.5e3, -- Range at which an asteroid with hide==1 is detected
   EW_JUMPDETECT_DIST   = 7.5e3, -- Range at which a jump with hide==1 is detected
   EW_SPOBDETECT_DIST   = 20e3, -- Range at which a spob with hide==1 is detected

   PILOT_SHIELD_DOWN_TIME = 5, -- Time shield is down after being knocked down to 0
   PILOT_DISABLED_ARMOUR= 0.1, -- Armour rate at which the ship is disabled (was 0 before 0.13.0)

   -- Constants used by multiple Lua scripts
   BITE_ACCEL_MOD = 390, -- like hades_torch (was 500 before 0.13.0)
   BITE_SPEED_MOD = 110, -- like adrenal_glands_ii (was 80 before 0.13.0)
}
