--[[

   Pirate Commodity Delivery Mission

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

include "dat/missions/neutral/commodity_run.lua"

lang = naev.lang()
if lang == "notreal" then
else -- default english
   misn_title = "Exploit the Demand for %s"
   misn_desc = "Pirates have been demanding a lot of %s lately, and that's driving the price up. If you find some of it and bring it back, you can make some good money off of them."

   cargo_land_p2 = {}
   cargo_land_p2[1] = " are bought by the boatload, eventually earning you %s credits."
   cargo_land_p2[2] = " are quickly sold out, earning you %s credits."
   cargo_land_p2[3] = " are eventually all sold to the pirates, though it takes some time. Your total earnings in the end amount to %s credits."

   osd_title = "Pirate Sales"
end

