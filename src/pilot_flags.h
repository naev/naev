/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#define pilot_clearFlagsRaw( a )                                               \
   memset( ( a ), 0, PILOT_FLAGS_MAX ) /**< Clears the pilot flags. */
#define pilot_copyFlagsRaw( d, s )                                             \
   memcpy( ( d ), ( s ),                                                       \
           PILOT_FLAGS_MAX ) /**< Copies the pilot flags from s to d. */
#define pilot_isFlagRaw( a, f )                                                \
   ( ( a )[f] ) /**< Checks to see if a pilot flag is set. */
#define pilot_setFlagRaw( a, f ) ( ( a )[f] = 1 ) /**< Sets flags rawly. */
#define pilot_isFlag( p, f )                                                   \
   ( ( p )->flags[f] ) /**< Checks if flag f is set on pilot p. */
#define pilot_setFlag( p, f )                                                  \
   ( ( p )->flags[f] = 1 ) /**< Sets flag f on pilot p. */
#define pilot_rmFlag( p, f )                                                   \
   ( ( p )->flags[f] = 0 ) /**< Removes flag f on pilot p. */
enum {
   /*
    * Creation-time flags
    */
   PILOT_PLAYER,       /**< Pilot is a player. */
   PILOT_PLAYER_FLEET, /**< Pilot is part of the player's fleet. */
   PILOT_CARRIED,      /**< Pilot usually resides in a fighter bay. */
   PILOT_CREATED_AI,   /**< Pilot has already created AI. */
   PILOT_NO_OUTFITS,   /**< Do not create the pilot with outfits. */
   PILOT_NO_EQUIP,     /**< Do not run the equip script on the pilot. */
   /*
    * Dynamic flags
    */
   /* Escort stuff. */
   PILOT_NOFREE,  /**< Don't free the pilot memory (but erase from stack). */
   PILOT_PERSIST, /**< Persist pilot on jump. */
   PILOT_NOCLEAR, /**< Pilot isn't removed by pilots_clear(). */
   PILOT_CARRIER_DIED, /**< The carrier carrying the fighter died. */
   /* Hyperspace. */
   PILOT_HYP_PREP,   /**< Pilot is getting ready for hyperspace. */
   PILOT_HYP_BRAKE,  /**< Pilot has already braked before jumping. */
   PILOT_HYP_BEGIN,  /**< Pilot is starting engines. */
   PILOT_HYPERSPACE, /**< Pilot is in hyperspace. */
   PILOT_HYP_END,    /**< Pilot is exiting hyperspace. */
   PILOT_HAILING,    /**< Pilot is hailing the player. */
   /* Boarding. */
   PILOT_BOARDABLE,      /**< Pilot can be boarded even while active. */
   PILOT_BOARDED_PILOT,  /**< Pilot has been boarded by a non-player pilot
                            already. */
   PILOT_BOARDED_PLAYER, /**< Pilot has been boarded by the player already. */
   PILOT_NOBOARD,        /**< Pilot can't be boarded. */
   PILOT_BOARDING,       /**< Pilot is currently boarding its target. */
   /* Disabling. */
   PILOT_NODISABLE,     /**< Pilot can't be disabled. */
   PILOT_DISABLED,      /**< Pilot is disabled. */
   PILOT_DISABLED_PERM, /**< Pilot is permanently disabled. */
   /* Death. */
   PILOT_NODEATH,     /**< Pilot can not die, will stay at 1 armour. */
   PILOT_DEAD,        /**< Pilot is in its dying throes */
   PILOT_DEATH_SOUND, /**< Pilot just did death explosion. */
   PILOT_EXPLODED,    /**< Pilot did final death explosion. */
   PILOT_DELETE,      /**< Pilot will get deleted asap. */
   /* Invincibility. */
   PILOT_INVINCIBLE,    /**< Pilot can't be hit ever. */
   PILOT_INVINC_PLAYER, /**< Pilot can not be hurt by the player. */
   /* Player-related stuff. */
   PILOT_HOSTILE,        /**< Pilot is hostile to the player. */
   PILOT_FRIENDLY,       /**< Pilot is friendly to the player. */
   PILOT_SCANNING,       /**< Pilot is scanning the pilot. */
   PILOT_COMBAT,         /**< Pilot is engaged in combat. */
   PILOT_BRIBED,         /**< Pilot has been bribed already. */
   PILOT_DISTRESSED,     /**< Pilot has distressed once already. */
   PILOT_NONTARGETABLE,  /**< Safe time for when the player is taking off or
                            jumping in. */
   PILOT_PLAYER_SCANNED, /**< Pilot has been scanned by the player. */
   /* Landing stuff. */
   PILOT_LANDING, /**< Pilot is landing. */
   PILOT_TAKEOFF, /**< Pilot is taking off. */
   /* Visibility stuff. */
   PILOT_STEALTH,   /**< Pilot is in stealth mode. */
   PILOT_NORENDER,  /**< Pilot does not get rendered. */
   PILOT_VISPLAYER, /**< Pilot is always visible to the player (only player). */
   PILOT_VISIBLE,   /**< Pilot is always visible to other pilots. */
   PILOT_INVISIBLE, /**< Pilot doesn't appear on the radar nor can be targetted,
                       however, it still can do stuff and is rendered. */
   PILOT_HIDE,    /**< Pilot is invisible to other pilots, nor is it updated. */
   PILOT_HILIGHT, /**< Pilot is hilighted when visible (this does not increase
                     visibility). */
   /* Outfit stuff. */
   PILOT_AFTERBURNER, /**< Pilot has their afterburner activated. */
   /* Refuelling. */
   PILOT_REFUELING,      /**< Pilot is trying to refuelling. */
   PILOT_REFUELBOARDING, /**< Pilot is actively refuelling. */
   /* Cooldown. */
   PILOT_COOLDOWN,       /**< Pilot is in active cooldown mode. */
   PILOT_COOLDOWN_BRAKE, /**< Pilot is braking to enter active cooldown mode. */
   /* Manual control and limits. */
   PILOT_MANUAL_CONTROL, /**< Pilot is under manual control of a mission or
                            event. */
   PILOT_NOJUMP,         /**< Pilot cannot engage hyperspace engines. */
   PILOT_NOLAND,         /**< Pilot cannot land on spobs. */
   PILOT_HASSPEEDLIMIT,  /**< Speed limiting is activated for Pilot.*/
   PILOT_BRAKING,        /**< Pilot is braking. */
   /* Sentinal. */
   PILOT_FLAGS_MAX /**< Maximum number of flags. */
};
typedef char PilotFlags[PILOT_FLAGS_MAX];
