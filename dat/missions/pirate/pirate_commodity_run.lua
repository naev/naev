--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Commodity Run">
 <priority>5</priority>
 <cond>false or var.peek("commodity_runs_active") == nil or var.peek("commodity_runs_active") &lt; 3</cond>
 <chance>90</chance>
 <location>Computer</location>
 <faction>Wild Ones</faction>
 <faction>Black Lotus</faction>
 <faction>Raven Clan</faction>
 <faction>Dreamer Clan</faction>
 <faction>Pirate</faction>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
 --]]
--[[

   Pirate Commodity Delivery Mission

--]]
local pir = require "common.pirate"
require "missions.neutral.commodity_run"

-- luacheck: globals cargo_land (inherited function)

mem.misn_title = _("Exploit the Demand for {cargo}")
mem.misn_desc = _("Pirates on {pnt} have been demanding a lot of {cargo} lately, and that's driving the price up. If you find some of it and bring it back, you can make some good money off of them.")

cargo_land = {
   _("The containers of {cargo} are bought by the boatload, eventually earning you {credits}."),
   _("The containers of {cargo} are quickly sold out, earning you {credits}."),
   _("The containers of {cargo} are eventually all sold to the pirates, though it takes some time. Your total earnings in the end amount to {credits}."),
   _("Pirates immediately line up to buy your {cargo}. Before you know it, you have none left, and you find that you're {credits} richer."),
}

mem.osd_title = _("Pirate Sales")

mem.paying_faction = pir.systemClanP( system.cur() )
