return {
   fct            = faction.get("Empire"),
   cap_kill       = 15,
   delta_distress = {-1, 0},    -- Maximum change constraints
   delta_kill     = {-5, 1},    -- Maximum change constraints
   cap_misn_def   = 70-15, --30, -- TODO lower as more campaigns get added
   cap_misn_var   = "_fcap_empire",
   cap_tags       = {
      ["emp_cap_ch01_sml"] = { val=1, max=50 },
      ["emp_cap_ch01_med"] = { val=3, max=50 },
      ["emp_cap_ch01_lrg"] = { val=5, max=50 },
   }
}
