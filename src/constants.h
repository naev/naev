/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

typedef struct constants {
   // Physics constants
   double PHYSICS_SPEED_DAMP;
   double STEALTH_MIN_DIST;
   // Electronic warfare constants
   double EW_JUMP_BONUS_RANGE;
   double EW_ASTEROID_DIST;
   double EW_JUMPDETECT_DIST;
   double EW_SPOBDETECT_DIST;
   // Gameplay constants
   double PILOT_SHIELD_DOWN_TIME;
   double PILOT_DISABLED_ARMOUR;
} constants;

extern constants CTS;

int constants_init( void );
