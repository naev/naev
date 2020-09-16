/*
 * See Licensing and Copyright notice in naev.h
 */

#ifndef PILOT_FLAGS_H
#define PILOT_FLAGS_H

#define pilot_clearFlagsRaw(a) memset((a), 0, PILOT_FLAGS_MAX) /**< Clears the pilot flags. */
#define pilot_copyFlagsRaw(d,s) memcpy((d), (s), PILOT_FLAGS_MAX) /**< Copies the pilot flags from s to d. */
#define pilot_isFlagRaw(a,f)  ((a)[f]) /**< Checks to see if a pilot flag is set. */
#define pilot_setFlagRaw(a,f) ((a)[f] = 1) /**< Sets flags rawly. */
#define pilot_isFlag(p,f)     ((p)->flags[f]) /**< Checks if flag f is set on pilot p. */
#define pilot_setFlag(p,f)    ((p)->flags[f] = 1) /**< Sets flag f on pilot p. */
#define pilot_rmFlag(p,f)     ((p)->flags[f] = 0) /**< Removes flag f on pilot p. */
enum {
   /* creation */
   PILOT_PLAYER,       /**< Pilot is a player. */
   PILOT_CARRIED,      /**< Pilot usually resides in a fighter bay. */
   PILOT_CREATED_AI,   /** Pilot has already created AI. */
   PILOT_EMPTY,        /**< Do not add pilot to stack. */
   PILOT_NO_OUTFITS,   /**< Do not create the pilot with outfits. */
   /* dynamic */
   PILOT_HAILING,      /**< Pilot is hailing the player. */
   PILOT_NODISABLE,    /**< Pilot can't be disabled. */
   PILOT_INVINCIBLE,   /**< Pilot can't be hit ever. */
   PILOT_HOSTILE,      /**< Pilot is hostile to the player. */
   PILOT_FRIENDLY,     /**< Pilot is friendly to the player. */
   PILOT_COMBAT,       /**< Pilot is engaged in combat. */
   PILOT_AFTERBURNER,  /**< Pilot has their afterburner activated. */
   PILOT_HYP_PREP,     /**< Pilot is getting ready for hyperspace. */
   PILOT_HYP_BRAKE,    /**< PIlot has already braked before jumping. */
   PILOT_HYP_BEGIN,    /**< Pilot is starting engines. */
   PILOT_HYPERSPACE,   /**< Pilot is in hyperspace. */
   PILOT_HYP_END,      /**< Pilot is exiting hyperspace. */
   PILOT_BOARDED,      /**< Pilot has been boarded already. */
   PILOT_NOBOARD,      /**< Pilot can't be boarded. */
   PILOT_BOARDING,     /**< Pilot is currently boarding it's target. */
   PILOT_BRIBED,       /**< Pilot has been bribed already. */
   PILOT_DISTRESSED,   /**< Pilot has distressed once already. */
   PILOT_REFUELING,    /**< Pilot is trying to refueling. */
   PILOT_REFUELBOARDING, /**< Pilot is actively refueling. */
   PILOT_MANUAL_CONTROL, /**< Pilot is under manual control of a mission or event. */
   PILOT_LANDING,      /**< Pilot is landing. */
   PILOT_TAKEOFF,      /**< Pilot is taking off. */
   PILOT_DISABLED,     /**< Pilot is disabled. */
   PILOT_DISABLED_PERM, /**< Pilot is permanently disabled. */
   PILOT_DEAD,         /**< Pilot is in it's dying throes */
   PILOT_DEATH_SOUND,  /**< Pilot just did death explosion. */
   PILOT_EXPLODED,     /**< Pilot did final death explosion. */
   PILOT_DELETE,       /**< Pilot will get deleted asap. */
   PILOT_VISPLAYER,    /**< Pilot is always visible to the player (only player). */
   PILOT_VISIBLE,      /**< Pilot is always visible to other pilots. */
   PILOT_HILIGHT,      /**< Pilot is hilighted when visible (this does not increase visibility). */
   PILOT_INVISIBLE,    /**< Pilot is invisible to other pilots. */
   PILOT_BOARDABLE,    /**< Pilot can be boarded even while active. */
   PILOT_NOJUMP,       /**< Pilot cannot engage hyperspace engines. */
   PILOT_NOLAND,       /**< Pilot cannot land on stations or planets. */
   PILOT_NODEATH,      /**< Pilot can not die, will stay at 1 armour. */
   PILOT_INVINC_PLAYER, /**< Pilot can not be hurt by the player. */
   PILOT_COOLDOWN,     /**< Pilot is in active cooldown mode. */
   PILOT_COOLDOWN_BRAKE, /**< Pilot is braking to enter active cooldown mode. */
   PILOT_BRAKING,      /**< Pilot is braking. */
   PILOT_HASSPEEDLIMIT, /**< Speed limiting is activated for Pilot.*/
   PILOT_PERSIST, /**< Persist pilot on jump. */
   PILOT_FLAGS_MAX     /**< Maximum number of flags. */
};
typedef char PilotFlags[ PILOT_FLAGS_MAX ];

#endif