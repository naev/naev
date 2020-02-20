--[[

   Fake ID

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

include "numstring.lua"


misn_title = _("Fake ID")
misn_desc = _([[This fake ID will allow you to conduct business with all major locations where you are currently wanted. It will be as if you were a new person. However, committing any crime will risk discovery of your identity by authorities, no gains in reputation under your fake ID will ever improve your real name's reputation under any circumstance, and gaining too much reputation with factions where you are wanted can lead to the discovery of your true identity as well. Note that fake ID does not work on pirates, for whom your reputation will be affected as normal (whether good or bad).
Cost: %s credits]])
misn_reward = _("None")

lowmoney = _("You don't have enough money to buy a fake ID. The price is %s credits.")

noticed_onplanet = _([[During a routine check, you hand over your fake ID as usual, but the person checking your ID eyes it strangely for a time that seems to be periods long. Eventually you are handed your ID back, but this is not a good sign.
    When you check, you see that the secrecy of your identity is in jeopardy. You're safe for now, but you make a mental note to prepare for the worst when you take off, because your fake ID probably won't be of any further use by then.]])

noticed_offplanet = _("It seems your actions have led to the discovery of your identity. As a result, your fake ID is now useless.")


factions = {
   "Empire", "Goddard", "Dvaered", "Za'lek", "Sirius", "Soromid", "Frontier",
   "Trader", "Miner"
}
misnvars = {
   Empire = "_fcap_empire", Goddard = "_fcap_goddard",
   Dvaered = "_fcap_dvaered", ["Za'lek"] = "_fcap_zalek",
   Sirius = "_fcap_sirius", Soromid = "_fcap_soromid",
   Frontier = "_fcap_frontier", Trader = "_fcap_trader",
   Miner = "_fcap_miner"
}
orig_standing = {}
orig_standing["__save"] = true
orig_cap = {}
orig_cap["__save"] = true

temp_cap = 10
next_discovered = false


function create ()
   local nhated = 0
   for i, j in ipairs( factions ) do
      if faction.get(j):playerStanding() < 0 then
         nhated = nhated + 1
      end
   end
   if nhated <= 0 then misn.finish( false ) end

   credits = 50000 * nhated

   misn.setTitle( misn_title )
   misn.setDesc( misn_desc:format( numstring( credits ) ) )
   misn.setReward( misn_reward )
end


function accept ()
   if player.credits() < credits then
      tk.msg( "", lowmoney:format( numstring( credits ) ) )
      misn.finish()
   end

   player.pay( -credits )
   misn.accept()

   for i, j in ipairs( factions ) do
      local f = faction.get(j)
      if f:playerStanding() < 0 then
         orig_standing[j] = f:playerStanding()
         orig_cap[j] = var.peek( misnvars[j] )
         var.push( misnvars[j], temp_cap )
         f:setPlayerStanding( 0 )
      end
   end

   landed = true
   standhook = hook.standing( "standing" )
   hook.takeoff( "takeoff" )
   hook.land( "land" )
end


function standing( f, delta )
   local fn = f:name()
   if orig_standing[fn] ~= nil then
      if delta >= 0 then
         if var.peek( misnvars[fn] ) ~= temp_cap then
            abort()
         end
      else
         abort()
      end
   elseif fn == "Pirate" and delta >= 0 then
      local sf = system.cur():faction()
      if sf ~= nil and orig_standing[sf:name()] ~= nil then
         -- We delay choice of when you are discovered to prevent players
         -- from subverting the system to eliminate the risk.
         if next_discovered then
            abort()
         else
            next_discovered = rnd.rnd() < 0.1
         end
      end
   end
end


function takeoff ()
   landed = false
end


function land ()
   landed = true
end


function abort ()
   if standhook ~= nil then hook.rm(standhook) end
   
   for i, j in ipairs( factions ) do
      if orig_standing[j] ~= nil then
         if misnvars[j] ~= nil then
            if orig_cap[j] ~= nil then
               var.push( misnvars[j], orig_cap[j] + var.peek( misnvars[j] ) - temp_cap )
            else
               var.pop( misnvars[j] )
            end
         end
         faction.get(j):setPlayerStanding( orig_standing[j] )
      end
   end
   
   local msg = noticed_offplanet
   if landed then
      local f = planet.cur():faction()
      if f ~= nil and orig_standing[f:name()] ~= nil then
         msg = noticed_onplanet
      end
   end

   tk.msg( "", msg )
   misn.finish( false )
end
