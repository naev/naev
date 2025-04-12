return {
   -- Constants used by the Engine
   PHYSICS_SPEED_DAMP   = 4, -- Was 3 until 0.13.0
   EW_JUMP_BONUS_RANGE  = 2500, -- Range from jump at which a stealth bonus is applied
   EW_ASTEROID_DIST     = 7.5e3, -- Range at which an asteroid with hide==1 is detected
   EW_JUMPDETECT_DIST   = 7.5e3, -- Range at which a jump with hide==1 is detected
   EW_SPOBDETECT_DIST   = 20e3, -- Range at which a spob with hide==1 is detected

   -- Constants used by Lua scripts
   BITE_ACCEL_MOD = 500,
   BITE_SPEED_MOD = 80,
}
