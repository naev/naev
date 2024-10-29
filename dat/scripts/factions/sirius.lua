return {
   fct            = faction.get("Sirius"),
   rep_max        = 70, --30, -- TODO lower as missions get added
   rep_max_var    = "_fcap_sirius",
   -- Much more strict about scanning.
   scan_min       = -30,
   scan_mod       = 0.2,
   rep_max_tags   = {},
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
