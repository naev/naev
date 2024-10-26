return {
   fct            = faction.get("Empire"),
   max_rep        = 70-15, --30, -- TODO lower as more campaigns get added
   max_rep_var    = "_fcap_empire",
   cap_tags       = {
      -- TODO return cap_ch01 to 50 when possible
      ["emp_cap_ch01_sml"] = { val=1, max=70 },
      ["emp_cap_ch01_med"] = { val=3, max=70 },
      ["emp_cap_ch01_lrg"] = { val=5, max=70 },
   }
}
