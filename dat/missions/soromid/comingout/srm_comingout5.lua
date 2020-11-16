--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Garbage Person">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>2</priority>
  <done>Visiting Family</done>
  <chance>30</chance>
  <location>Bar</location>
  <faction>Soromid</faction>
 </avail>
 <notes>
  <campaign>Coming Out</campaign>
 </notes>
</mission>
--]]
--[[

   Waste Collector

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
require "missions/soromid/comingout/srm_comingout3.lua"
require "missions/soromid/common.lua"


title = {}
text = {}

title[1] = _("The Dirty Job")
text[1] = _([[You walk over to Chelsea to greet them when you notice an unpleasant odor coming off of them. Chelsea notices you. "Ah! %s! Uh, sorry about the smell. I don't know why the hell I did this, but I took a job from some guy here and now I'm stuck with it." You ask what kind of job it is. "Erm, I kind of agreed to take their trash from them." You grimace. "Yeah," Chelsea says, "it's gross. And what's worse, I'm in over my head. I've already taken the garbage and my new ship is packed to the brim with the stuff, but there's thugs outside that seem to be waiting for me." A look of rage appears on their face as they turn to the side. "You can probably guess who's responsible for that." The thought had crossed your mind as well.
    Chelsea turns back to you. "I know I ask a lot of you, but could you help me once again? I just need an escort to %s so I can drop off this garbage there. I'll give you %s credits for the trouble. What do you say?"]])

text[2] = _([["I appreciate it very much. I'll wait at the hangar until you're ready to take off. Get ready for a fight when we get out of the atmosphere; it's going to be a bumpy ride."]])

text[3] = _([["OK, I understand. I guess I'll have to find some other way to get rid of all this garbage..."]])

text[4] = _([["I'm not having any luck coming up with a plan to get rid of all of this garbage without getting jumped by those thugs. Is there any chance you could reconsider being my escort? It would be a big help."]])

title[6] = _("The Unbearable Smell Now Ends")
text[6] = _([[As you dock, you can't help but notice the foul smell of garbage all around you. The planet really does fit the name. You grimace as you watch workers unload what must be hundreds of tonnes of garbage from Chelsea's ship, some of which is leaking. Eventually Chelsea's ship is emptied and you and Chelsea are handed your credit chips for the job. You and Chelsea part ways, vowing to take a shower immediately while Chelsea vows to scrub the cargo hold of their ship clean.]])

misn_title = _("Waste Collector")
misn_desc = _("Chelsea needs an escort to %s so they can get rid of the garbage now filling their ship.")

npc_name = _("Chelsea")
npc_desc = _("Chelsea seems like they're stressed. Maybe you should see how they're doing?")

log_text = _([[You helped Chelsea get rid of a load of garbage they naively agreed to take to The Stinker as a mission, defending them from thugs along the way.]])


function create ()
   misplanet, missys = planet.get( "The Stinker" )
   if misplanet == nil or missys == nil or system.cur():jumpDist(missys) > 4 then
      misn.finish( false )
   end

   credits = 500000
   started = false

   misn.setNPC( npc_name, "soromid/unique/chelsea" )
   misn.setDesc( npc_desc )
end


function accept ()
   local txt
   if started then
      txt = text[4]
   else
      txt = text[1]:format( player.name(), misplanet:name(), numstring( credits ) )
   end
   started = true

   if tk.yesno( title[1], txt ) then
      tk.msg( title[1], text[2] )

      misn.accept()

      misn.setTitle( misn_title )
      misn.setDesc( misn_desc:format( misplanet:name() ) )
      misn.setReward( creditstring( credits ) )
      marker = misn.markerAdd( missys, "low" )

      osd_desc[1] = osd_desc[1]:format( misplanet:name(), missys:name() )
      misn.osdCreate( misn_title, osd_desc )

      startplanet = planet.cur()

      hook.takeoff( "takeoff" )
      hook.jumpout( "jumpout" )
      hook.jumpin( "jumpin" )
      hook.land( "land" )
   else
      tk.msg( title[1], text[3] )
      misn.finish()
   end
end


function spawnChelseaShip( param )
   chelsea = pilot.addRaw( "Rhino", "mercenary", param, "Comingout_associates" )
   chelsea:rmOutfit( "all" )
   chelsea:rmOutfit( "cores" )
   chelsea:addOutfit( "Unicorp PT-600 Core System" )
   chelsea:addOutfit( "Melendez Buffalo XL Engine" )
   chelsea:addOutfit( "S&K Medium Cargo Hull" )
   chelsea:addOutfit( "Heavy Ripper Turret", 2 )
   chelsea:addOutfit( "Enygma Systems Turreted Fury Launcher", 2 )
   chelsea:addOutfit( "Fury Missile", 80 )
   chelsea:addOutfit( "Droid Repair Crew" )
   chelsea:addOutfit( "Milspec Scrambler" )
   chelsea:addOutfit( "Boarding Androids MK1" )
   chelsea:addOutfit( "Cargo Pod", 4 )

   chelsea:setHealth( 100, 100 )
   chelsea:setEnergy( 100 )
   chelsea:setTemp( 0 )
   chelsea:setFuel( true )

   chelsea:cargoAdd( "Waste Containers", chelsea:cargoFree() )

   chelsea:setFriendly()
   chelsea:setHilight()
   chelsea:setVisible()
   chelsea:setInvincPlayer()
   chelsea:rename( "Chelsea" )

   hook.pilot( chelsea, "death", "chelsea_death" )
   hook.pilot( chelsea, "jump", "chelsea_jump" )
   hook.pilot( chelsea, "land", "chelsea_land" )
   hook.pilot( chelsea, "attacked", "chelsea_attacked" )

   chelsea_jumped = false
end


function spawnThug( param )
   local shiptypes = { "Hyena", "Hyena", "Shark", "Lancelot", "Admonisher" }
   local shiptype = shiptypes[ rnd.rnd( 1, #shiptypes ) ]
   thug = pilot.addRaw( shiptype, "baddie", param, "Comingout_thugs" )

   thug:setHostile()
   thug:rename( "Thug " .. shiptype )

   hook.pilot( thug, "death", "thug_removed" )
   hook.pilot( thug, "jump", "thug_removed" )
   hook.pilot( thug, "land", "thug_removed" )
end


function takeoff ()
   spawnChelseaShip( startplanet )
   jumpNext()
   spawnThug()
   spawnThug()
end


function land ()
   if planet.cur() == misplanet then
      tk.msg( title[6], text[6]:format( numstring( credits ) ) )
      player.pay( credits )

      local t = time.get():tonumber()
      var.push( "comingout_time", t )

      srm_addComingOutLog( log_text )

      misn.finish( true )
   else
      tk.msg( title[5], text[5] )
      misn.finish( false )
   end
end


function thug_timer ()
   spawnThug()
   spawnThug()
   if system.cur() == missys then
      spawnThug( lastsys )
   end
end
