--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--

   Commodity delivery missions. Player receives a bonus for bringing
   more of a commodity back.
--]]

include "dat/scripts/numstring.lua"

lang = naev.lang()
if lang == "es" then
else -- default english

   --Mission Details
   misn_title = "%s Delivery"
   misn_reward = "%s credits per ton"
   misn_desc = "There is an insufficient supply %s on this planet to satisfy the current demand. Go to any planet which sells this commodity and bring as much of it back as possible."

   cargo_land_title = "Delivery success!"

   cargo_land_p1 = {}
   cargo_land_p1[1] = "The crates of "
   cargo_land_p1[2] = "The drums of "
   cargo_land_p1[3] = "The containers of "

   cargo_land_p2 = {}
   cargo_land_p2[1] = " are carried out of your ship and tallied. After several different men double-check the register to confirm the amount, you paid %s credits and summarily dismissed."
   cargo_land_p2[2] = " are quickly and efficiently unloaded, labeled, and readied for distribution. The delivery manager thanks you with a credit chip worth %s credits."
   cargo_land_p2[3] = " are unloaded from your vessel by a team of dockworkers who are in no rush to finish, eventually delivering %s credits after the number of tons is determined."
   cargo_land_p2[4] = " are unloaded by robotic drones that scan and tally the contents. The human overseerer hands you %s credits when they finish."

   osd_title = "Commodity Delivery"
   osd_msg    = {}
   osd_msg[1] = "Buy as much %s as possible"
   osd_msg[2] = "Take the %s to %s in the %s system"
   osd_msg["__save"] = true

end


function create ()
   -- Note: this mission does not make any system claims.
 
   misplanet = planet.cur()
   missys = system.cur()
   
   -- TODO: find a better way to index all available commodities
   local commchoices = planet.commoditiesSold("Darkshed")

   chosen_comm = commchoices[ rnd.rnd( 1, #commchoices ) ]
   local mult = rnd.rnd( 1, 3 ) + math.abs( rnd.threesigma() * 2 )
   price = commodity.price( chosen_comm:name() ) * mult

   for k, v in pairs( planet.cur():commoditiesSold() ) do
      if v == chosen_comm then
         misn.finish(false)
      end
   end

   -- Set Mission Details
   misn.setTitle( misn_title:format( chosen_comm:name() ) )
   misn.markerAdd( system.cur(), "computer" )
   misn.setDesc( misn_desc:format( chosen_comm:name() ) )
   misn.setReward( misn_reward:format( math.floor( price ) ) )
    
end


function accept ()
   misn.accept()

   osd_msg[1] = osd_msg[1]:format( chosen_comm:name() )
   osd_msg[2] = osd_msg[2]:format( chosen_comm:name(), misplanet:name(), missys:name() )
   misn.osdCreate( osd_title, osd_msg )

   hook.enter( "enter" )
   hook.land( "land" )
end


function enter ()
   if pilot.cargoHas( player.pilot(), chosen_comm:name() ) > 0 then
      misn.osdActive( 2 )
   else
      misn.osdActive( 1 )
   end
end


function land ()
   local amount = pilot.cargoHas( player.pilot(), chosen_comm:name() )
   local reward = amount * price

   if planet.cur() == misplanet and amount > 0 then
      local txt = (
         cargo_land_p1[ rnd.rnd( 1, #cargo_land_p1 ) ] ..
         chosen_comm:name() ..
         cargo_land_p2[ rnd.rnd( 1, #cargo_land_p2 ) ]:format( numstring( reward ) )
         )
      tk.msg( cargo_land_title, txt )
      pilot.cargoRm( player.pilot(), chosen_comm:name(), amount )
      player.pay( reward )
      misn.finish( true )
   end
end

