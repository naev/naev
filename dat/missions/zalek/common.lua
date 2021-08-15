--[[

   Za'lek Common Functions

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

function zlk_addSciWrongLog( text )
   shiplog.create( "zlk_sciwrong", _("Science Gone Wrong"), _("Za'lek") )
   shiplog.append( "zlk_sciwrong", text )
end

function zlk_addNebuResearchLog( text )
   shiplog.create( "zlk_neburesearch", _("Nebula Research"), _("Za'lek") )
   shiplog.append( "zlk_neburesearch", text )
end

-- Function for adding log entries for miscellaneous one-off missions.
function zlk_addMiscLog( text )
   shiplog.create( "zlk_misc", _("Miscellaneous"), _("Za'lek") )
   shiplog.append( "zlk_misc", text )
end

-- Checks to see if the player has a Za'lek ship.
function zlk_hasZalekShip()
   local shipname = player.pilot():ship():nameRaw()
   return string.find( shipname, "Za'lek" ) ~= nil
end
