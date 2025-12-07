--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="FLF Commodity Run">
 <priority>5</priority>
 <cond>var.peek("commodity_runs_active") == nil or var.peek("commodity_runs_active") &lt; 3</cond>
 <chance>90</chance>
 <location>Computer</location>
 <faction>FLF</faction>
 <notes>
   <done_misn name="Deal with the FLF agent"/>
 </notes>
</mission>
 --]]
--[[

   FLF Commodity Delivery Mission

--]]
require "missions.neutral.commodity_run"

-- luacheck: globals cargo_land commchoices (inheriting variables, TODO get rid of)

mem.misn_title = _("FLF: {cargo} Supply Run")
mem.misn_desc = _("There is a need to supply {pnt} with more {cargo}. Find a planet where you can buy this commodity and bring as much of it back as possible.")

cargo_land = {
   _("The containers of {cargo} are carried out of your ship and tallied. After making sure nothing was missed from your cargo hold, you are paid {credits}, thanked for assisting the FLF's operations, and dismissed."),
   _("The containers of {cargo} are quickly and efficiently unloaded and transported to the storage facility. The FLF officer in charge thanks you with a credit chip worth {credits}."),
   _("The containers of {cargo} are unloaded from your vessel by a team of FLF soldiers who are in no rush to finish, eventually delivering {credits} after the number of tonnes is determined."),
}

mem.osd_title = _("FLF Supply Run")


commchoices = { "Food", "Ore", "Industrial Goods", "Medicine", "Water" }
