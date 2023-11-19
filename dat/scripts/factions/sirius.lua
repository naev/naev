return {
   fct            = faction.get("Sirius"),
   cap_kill       = 10,
   delta_distress = {-0.5, 0},    -- Maximum change constraints
   delta_kill     = {-5, 1},    -- Maximum change constraints
   cap_misn_def   = 70, --30, -- TODO lower as missions get added
   cap_misn_var   = "_fcap_sirius",
   cap_tags       = {
   },
   -- Based on Zen ranks
   text = {
      [100] = _("Prefect"),
      [90]  = _("Adjunct Prefect"),
      [75]  = _("Senior Instructor"),
      [60]  = _("Instructor"),
      [45]  = _("Monk"),
      [30]  = _("Novice"),
      [10]  = _("Enlightened"),
      [0]   = _("Commoner"),
      [-1]  = _("Nonbeliever"),
      [-30] = _("Infidel"),
      [-50] = _("Enemy"),
   },
   text_friendly  = _("Compassionate"),
   text_neutral   = _("Unconcerned"),
   text_hostile   = _("Litigious"),
   text_bribed    = _("Subsidized"),
}
