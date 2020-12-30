--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Enter Tutorial Event">
 <trigger>enter</trigger>
 <chance>100</chance>
</event>
--]]
--[[
   Enter Tutorial Event

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

nebu_volat_title = _("Volatility Rising")
nebu_volat_msg = _([[As you jump the system you notice a small alarm lights up in the control panel:
    %s
    Looking over the systems it seems like the nebula is unstable here and is beginning to damage the ship's shields. If the volatility gets any stronger, it could be fatal to the %s. Going deeper into the nebula could prove to be a very risky endeavour.]])
nebu_volat_wng = "#r" .. _("WARNING: NEBULA VOLATILITY DETECTED") .. "#0"


function create ()
   local sys = system.cur()
   local nebu_dens, nebu_volat = sys:nebula()
   if not var.peek( "tutorial_nebula_volatility" ) and nebu_volat > 0 then
      tk.msg( nebu_volat_title, string.format(
            nebu_volat_msg, nebu_volat_wng, player.ship() ) )
      var.push( "tutorial_nebula_volatility", true )
   end
   evt.finish()
end

