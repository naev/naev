/*
 * See Licensing and Copyright notice in naev.h
 */

#include <lualib.h>
#include <math.h>

#include "constants.h"

constants CTS = {
   .PHYSICS_SPEED_DAMP         = 3., /* Default before 0.13.0. */
   .STEALTH_MIN_DIST           = 1000.,
   .SHIP_MIN_MASS              = 0.5, /* 0 before 0.13.0 */
   .EW_JUMP_BONUS_RANGE        = 2500.,
   .EW_ASTEROID_DIST           = 7.5e3,
   .EW_JUMPDETECT_DIST         = 7.5e3,
   .EW_SPOBDETECT_DIST         = 20e3,
   .PILOT_SHIELD_DOWN_TIME     = 5.0,
   .PILOT_STRESS_RECOVERY_TIME = 5.0, /* 0 before 0.14.0 */
   .PILOT_DISABLED_ARMOUR      = 0.1, /* 0 before 0.13.0 */
   .CAMERA_ANGLE               = M_PI_4,
   .WARN_BUY_INTRINSICS        = 1,
   // Meta constant calculated from CAMERA_ANGLE
   .CAMERA_VIEW     = 1.0,
   .CAMERA_VIEW_INV = 1.0,
};
