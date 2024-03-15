/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/**
 * @brief Used to generalize what the weapon is targetting.
 */
typedef enum TargetType_ {
   TARGET_NONE,
   TARGET_PILOT,
   TARGET_WEAPON,
   TARGET_ASTEROID,
} TargetType;

/**
 * @brief Represents a weapon target.
 */
typedef struct Target_ {
   TargetType type; /* Target type. */
   union {
      unsigned int id; /* For pilot/weapons. */
      struct {
         int anchor;
         int asteroid;
      } ast; /* For asteroids. */
   } u;
} Target;
