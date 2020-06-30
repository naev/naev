--[[

   FLF Commodity Delivery Mission

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

--]]

require "dat/missions/neutral/commodity_run.lua"

misn_title = _("FLF: %s Supply Run")
misn_desc = _("There is a need for more %s at this base. Find a planet where you can buy this commodity and bring as much of it back as possible.")

cargo_land_p2 = {}
cargo_land_p2[1] = _("%s%s are carried out of your ship and tallied. After making sure nothing was missed from your cargo hold, you are paid %s credits, thanked for assisting the FLF's operations, and dismissed.")
cargo_land_p2[2] = _("%s%s are quickly and efficiently unloaded and transported to the storage facility. The FLF officer in charge thanks you with a credit chip worth %s credits.")
cargo_land_p2[3] = _("%s%s are unloaded from your vessel by a team of FLF soldiers who are in no rush to finish, eventually delivering %s credits after the number of tonnes is determined.")

osd_title = _("FLF Supply Run")


commchoices = { "Food", "Ore", "Industrial Goods", "Medicine", "Water" }

