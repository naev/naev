--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Proteron Discovery">
  <trigger>enter</trigger>
  <chance>100</chance>
  <flags>
   <unique />
  </flags>
 </event>
 --]]
--[[

   Proteron Discovery Event

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

   This event runs in the background to cause the Proteron to be "known"
   once you discover them.

--]]


function create()
   if system.cur():faction() == faction.get("Proteron") then
      faction.get("Proteron"):setKnown( true )
      evt.finish( true )
   end
   evt.finish( false )
end