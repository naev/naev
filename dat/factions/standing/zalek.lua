-- Za'lek faction standing script
require "factions.standing.lib.base"

standing = sbase.newStanding{
   fct            = faction.get("Za'lek"),
   cap_kill       = 15,
   delta_distress = {-0.5, 0},  -- Maximum change constraints
   delta_kill     = {-5, 1},    -- Maximum change constraints
   cap_misn_init  = 30,
   cap_misn_var   = "_fcap_zalek",
}
