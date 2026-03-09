return {
   fct            = faction.get("Traders Society"),
   rep_max        = 30,
   rep_max_var    = "_fcap_trader",
   destroy_max    = 10,
   distress_mod   = 0.1, -- They lose from distressing
   board_mod      = 0.5, -- Dislike being stolen from
   rep_max_tags   = {
      ["trader_cap_small"]  = { val=5, max=100 },
      ["trader_cap_medium"] = { val=10, max=100 },
   }
}
