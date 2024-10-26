return {
   fct            = faction.get("Empire"),
   rep_max        = 70-15, --30, -- TODO lower as more campaigns get added
   rep_max_var    = "_fcap_empire",
   rep_max_tags   = {
      -- TODO return cap_ch01 to 50 when possible
      ["emp_cap_ch01_sml"] = { val=1, max=70 },
      ["emp_cap_ch01_med"] = { val=3, max=70 },
      ["emp_cap_ch01_lrg"] = { val=5, max=70 },
   }
}
