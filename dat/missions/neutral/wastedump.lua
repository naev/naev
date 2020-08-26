--[[

   Waste Dump

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

require "numstring.lua"

text = {}
text[1] = _("The waste containers are loaded onto your ship and you are paid %s credits. You begin to wonder if accepting this job was really a good idea.")
text[2] = _("Workers pack your cargo hold full of as much garbage as it can hold, then hastily hand you a credit chip containing %s credits. Smelling the garbage, you immediately regret taking the job.")
text[3] = _("Your hold is crammed full with garbage and you are summarily paid %s credits. By the time the overpowering stench eminating from  your cargo hold is apparent to you, it's too late to back down; you're stuck with this garbage until you can find some place to get rid of it.")

finish_text = {}
finish_text[1] = _("You drop the garbage off, relieved to have it out of your ship.")
finish_text[2] = _("You finally drop off the garbage and proceed to disinfect yourself and your cargo hold to the best of your ability.")
finish_text[3] = _("Finally, the garbage leaves your ship and you breathe a sigh of relief.")
finish_text[4] = _("Wrinkling your nose in disgust, you finally rid yourself of the waste containers you have been charged with disposing of.")

abort_text = {}
abort_text[1] = _("Sick and tired of smelling garbage, you illegally jettison the waste containers into space, hoping that no one notices.")
abort_text[2] = _("You decide that the nearest waste dump location is too far away for you to bother to go to and simply jettison the containers of waste. You hope you don't get caught.")
abort_text[3] = _("You dump the waste containers into space illegally, noting that you should make sure not to get caught by authorities.")

abort_landed_text = _("In your desperation to rid yourself of the garbage, you clumsily eject it from your cargo pod while you are still landed. Garbage spills all over the hangar and local officials immediately take notice. After you apologize profusely and explain the situation away as an accident, the officials let you off with a fine of %s credits.")

noland_msg = _("Get lost, waste dumping scum! We don't want you here!")

misn_title = _("Waste Dump")
misn_reward = _("%s credits per tonne")
misn_desc = _("Take as many waste containers off of here as your ship can hold and drop them off at any authorized garbage collection facility. You will be paid immediately, but any attempt to illegally jettison the waste into space will be severely punished if you are caught.")

osd_title = _("Waste Dump")
osd_msg = {}
osd_msg[1] = _("Land on any garbage collection facility (indicated on your map) to drop off the Waste Containers")

-- List of possible waste dump planets.
dest_planets = { "The Stinker", "Eiroik" }


function create ()
   local dist = nil
   local p, sys
   for i, j in ipairs( dest_planets ) do
      p, sys = planet.get( j )
      if dist == nil or system.cur():jumpDist(sys) < dist then
         dist = system.cur():jumpDist(sys)
      end
   end

   -- Note: this mission makes no system claims

   credits_factor = 1000 * dist
   credits_mod = 10000 * rnd.sigma()

   landed = true

   for i, j in ipairs( dest_planets ) do
      local p, sys
      p, sys = planet.get( j )
      misn.markerAdd( sys, "computer" )
   end

   -- Set mission details
   misn.setTitle( misn_title )
   misn.setDesc( misn_desc )
   misn.setReward( misn_reward:format( numstring( credits_factor ) ) )
end


function accept ()
   misn.accept()
   
   local q = player.pilot():cargoFree()
   credits = credits_factor * q + credits_mod

   local txt = text[ rnd.rnd( 1, #text ) ]
   tk.msg( "", txt:format( numstring( credits ) ) )

   cid = misn.cargoAdd( "Waste Containers", q )
   player.pay( credits )

   misn.osdCreate( osd_title, osd_msg )

   hook.takeoff( "takeoff" )
   hook.land( "land" )
end


function takeoff ()
   landed = false
end


function land ()
   landed = true

   for i, j in ipairs( dest_planets ) do
      if planet.get( j ) == planet.cur() then
         local txt = finish_text[ rnd.rnd( 1, #finish_text ) ]
         tk.msg( "", txt )
         misn.finish( true )
      end
   end
end


function abort ()
   if landed then
      misn.cargoRm( cid )
      local fine = 2 * credits
      tk.msg( "", abort_landed_text:format( numstring( fine ) ) )
      player.pay( -fine )
      misn.finish( false )
   else
      local txt = abort_text[ rnd.rnd( 1, #abort_text ) ]
      tk.msg( "", txt )

      misn.cargoJet( cid )

      -- Make everyone angry
      for i, j in ipairs( pilot.get() ) do
         j:setHostile()
      end

      -- Add some police!
      local presences = system.cur():presences()
      local f = nil
      if presences then
         local strongest_amount = 0
         for k, v in pairs( presences ) do
            if v > strongest_amount then
               f = faction.get(k)
               strongest_amount = v
            end
         end
      end

      local choices
      if f == faction.get( "Empire" ) then
         choices = { "Empire Sml Defense", "Empire Lge Attack", "Empire Med Attack" }
      elseif f == faction.get( "Goddard" ) then
         choices = { "Goddard Goddard", "Goddard Lancelot" }
      elseif f == faction.get( "Dvaered" ) then
         choices = { "Dvaered Big Patrol", "Dvaered Small Patrol", "Dvaered Strike Force" }
      elseif f == faction.get( "Soromid" ) then
         choices = { "Soromid Arx", "Soromid Vox", "Soromid Nyx", "Soromid Odium" }
      elseif f == faction.get( "Za'lek" ) then
         choices = { "Za'lek Hephaestus", "Za'lek Mephisto", "Za'lek Diablo", "Za'lek Demon" }
      elseif f == faction.get( "Sirius" ) then
         choices = { "Sirius Preacher", "Sirius Divinity", "Sirius Dogma" }
      elseif f == faction.get( "Frontier" ) then
         choices = { "Frontier Phalanx", "Frontier Lancelot", "Frontier Ancestor" }
      elseif f == faction.get( "Thurion" ) then
         choices = { "Thurion Apprehension", "Thurion Certitude" }
      elseif f == faction.get( "Proteron" ) then
         choices = { "Proteron Kahan", "Proteron Watson", "Proteron Archimedes" }
      elseif f == faction.get( "Collective" ) then
         choices = { "Collective Lge Swarm", "Collective Sml Swarm" }
      elseif f == faction.get( "FLF" ) then
         choices = { "FLF Pacifier", "FLF Vendetta", "FLF Lancelot" }
      elseif f == faction.get( "Pirate" ) then
         choices = { "Pirate Kestrel", "Pirate Phalanx", "Pirate Admonisher" }
      else
         choices = { "Vendetta Quartet", "Mercenary Pacifier", "Mercenary Ancestor", "Mercenary Vendetta" }
      end

      for n = 1, rnd.rnd( 2, 4 ) do
         for i, j in ipairs( system.cur():jumps() ) do
            local p = pilot.add( choices[ rnd.rnd( 1, #choices ) ], nil, j:dest() )
            for k, v in ipairs( p ) do
               v:setHostile()
            end
         end
      end

      -- No landing, filthy waste dumper!
      player.allowLand( false, noland_msg )

      misn.finish( true )
   end
end
