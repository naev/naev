--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Commodity Run">
 <avail>
  <priority>5</priority>
  <cond>false or var.peek("commodity_runs_active") == nil or var.peek("commodity_runs_active") &lt; 3</cond>
  <chance>90</chance>
  <location>Computer</location>
  <faction>Wild Ones</faction>
  <faction>Black Lotus</faction>
  <faction>Raven Clan</faction>
  <faction>Dreamer Clan</faction>
  <faction>Pirate</faction>
 </avail>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
 --]]
--[[

   Pirate Commodity Delivery Mission

--]]
local pir = require "missions.pirate.common"
require "missions.neutral.commodity_run"

misn_title = _("Exploit the Demand for %s")
misn_desc = _("Pirates on %s have been demanding a lot of %s lately, and that's driving the price up. If you find some of it and bring it back, you can make some good money off of them.")

cargo_land = {}
cargo_land[1] = _("The containers of %s are bought by the boatload, eventually earning you %s.")
cargo_land[2] = _("The containers of %s are quickly sold out, earning you %s.")
cargo_land[3] = _("The containers of %s are eventually all sold to the pirates, though it takes some time. Your total earnings in the end amount to %s.")
cargo_land[4] = _("Pirates immediately line up to buy your %s. Before you know it, you have none left, and you find that you're %s richer.")

osd_title = _("Pirate Sales")

paying_faction = pir.systemClanP( system.cur() )
