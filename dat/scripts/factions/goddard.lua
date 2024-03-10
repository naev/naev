return {
   fct            = faction.get("Goddard"),
   cap_kill       = 5,
   delta_distress = {-1, 0},    -- Maximum change constraints
   delta_kill     = {-5, 1},    -- Maximum change constraints
   cap_misn_def   = 30, -- Doesn't matter atm
   cap_misn_var   = "_fcap_goddard",
   cap_tags       = {},
}
