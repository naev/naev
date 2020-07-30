--[[

   Shiplog Event

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

--

   This event records events into a travel log. The travel log is limited
   to a certain number of entries so it doesn't overwhelm the save file.

--]]

attacked_text = _("Hostility met in the %s system")
jump_text = _("Jumped from the %s system to the %s system")
land_text = _("Landed on %s in the %s system")


function create ()
   shiplog.createLog( "travel", _("Travel Log"), _("Travel"), false, 20 )

   lastsys = system.cur()
   attacked = false

   hook.pilot( player.pilot(), "attacked", "player_attacked" )
   hook.jumpin( "jumpin" )
   hook.land( "land" )
end


function player_attacked ()
   if not attacked then
      shiplog.appendLog( "travel", attacked_text:format( system.cur():name() ) )
      attacked = true
   end
end


function jumpin ()
   local s = system.cur()
   shiplog.appendLog( "travel", jump_text:format( lastsys:name(), s:name() ) )
   lastsys = s
   attacked = false
end


function land ()
   local p = planet.cur()
   local s = p:system()
   shiplog.appendLog( "travel", land_text:format( p:name(), s:name() ) )
   evt.finish( false )
end
