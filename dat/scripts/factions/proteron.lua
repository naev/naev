return {
   fct            = faction.get("Proteron"),
   cap_kill       = 5,
   delta_distress = {-0.5, 0},    -- Maximum change constraints
   delta_kill     = {-5, 1},    -- Maximum change constraints
   cap_misn_def   = 70, --30, -- TODO lower as missions get added
   cap_misn_var   = "_fcap_proteron",
   cap_tags       = {
   },
}
