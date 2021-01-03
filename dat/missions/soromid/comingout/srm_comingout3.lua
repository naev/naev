--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="A Friend's Aid">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>2</priority>
   <done>Coming of Age</done>
   <chance>30</chance>
   <location>Bar</location>
   <faction>Soromid</faction>
   <cond>var.peek("comingout_time") == nil or time.get() &gt;= time.fromnumber(var.peek("comingout_time")) + time.create(0, 20, 0)</cond>
  </avail>
  <notes>
   <campaign>Coming Out</campaign>
   <tier>3</tier>
  </notes>
 </mission>
 --]]
--[[

   A Friend's Aid

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

require "numstring"
require "cargo_common"
require "nextjump"
require "missions/soromid/common"


title = {}
text = {}

title[1] = _("A Friend's Aid")
text[1] = _([[Chelsea gleefully waves you over. "It's nice to see you again!" she says. The two of you chat a bit about her venture into the piloting business; all is well from the sound of it. "Say," she says, "someone offered me a really interesting mission recently, but I had to decline because my ship isn't really up to task. If you could just escort my ship through that mission I could share the pay with you! Your half would be %s. How about it?"]])

text[2] = _([["Awesome! I really appreciate it!
    "So the mission is in theory a pretty simple one: I need to deliver some cargo to %s in the %s system. Trouble is apparently I'll be getting in the middle of some sort of trade dispute with a shady Soromid company that's bribed the local Soromid pilots. Needless to say we can expect to be attacked by some thugs and the Soromid military isn't likely to be of much help.
    "That's where you come in. I just need you to follow me along, make sure I finish jumping or landing before you do, and if we encounter any hostilities, help me shoot them down. Shouldn't be too too hard as long as you've got a decent ship. I'll meet you out in space!"]])

text[3] = _([["Ah, OK. Let me know if you change your mind."]])

text[4] = _([["I could still use your help with that mission! Could you help me out?"]])

title[5] = _("Mission Failed")
text[5] = _("You have lost contact with Chelsea and therefore failed the mission.")

title[6] = _("Another Happy Landing")
text[6] = _([[You successfully land and dock alongside Chelsea and she approaches the worker for the cargo delivery. The worker gives her a weird look, but collects the cargo with the help of some robotic drones and hands her a credit chip. When you get back to your ships, Chelsea transfers the sum of %s to your account, and you idly chat with her for a while.
    "Anyway, I should probably get going now," she says. "But I really appreciated the help there! Get in touch with me again sometime. We make a great team!" You agree, and you both go your separate ways once again.]])

misn_title = _("A Friend's Aid")
misn_desc = _("Chelsea needs you to escort her to %s.")

npc_name = _("Chelsea")
npc_desc = _("You see Chelsea looking contemplative.")

osd_desc    = {}
osd_desc[1] = _("Escort Chelsea to %s in the %s system.")

cheljump_msg = _("Chelsea has jumped to %s.")
chelland_msg = _("Chelsea has landed on %s.")
chelkill_msg = _("MISSION FAILED: A rift in the space-time continuum causes you to have never met Chelsea in that bar.")
chelflee_msg = _("MISSION FAILED: Chelsea has abandoned the mission.")
plflee_msg = _("MISSION FAILED: You have abandoned the mission.")

log_text = _([[You helped escort Chelsea through a dangerous cargo delivery mission where you had to protect her from the thugs of a shady company. She said that she would like to get back in touch with you again sometime for another mission.]])


function create ()
   misplanet, missys, njumps, tdist, cargo, avgrisk = cargo_calculateRoute()
   if misplanet == nil or missys == nil or avgrisk > 0 then
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
      txt = text[1]:format( creditstring( credits ) )
   end
   started = true

   if tk.yesno( title[1], txt ) then
      tk.msg( title[1], text[2]:format( misplanet:name(), missys:name() ) )

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


function cargo_selectMissionDistance ()
   return 3
end


function spawnChelseaShip( param )
   chelsea = pilot.addRaw( "Llama", "mercenary", param, "Comingout_associates" )
   chelsea:rmOutfit( "all" )
   chelsea:rmOutfit( "cores" )
   chelsea:addOutfit( "Unicorp PT-80 Core System" )
   chelsea:addOutfit( "Unicorp Hawk 300 Engine" )
   chelsea:addOutfit( "Unicorp D-4 Light Plating" )
   chelsea:addOutfit( "Laser PD MK2", 2 )
   chelsea:addOutfit( "Small Shield Booster", 2 )
   chelsea:addOutfit( "Cargo Pod", 2 )

   chelsea:setHealth( 100, 100 )
   chelsea:setEnergy( 100 )
   chelsea:setTemp( 0 )
   chelsea:setFuel( true )

   chelsea:cargoAdd( "Industrial Goods", chelsea:cargoFree() )

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
   local shiptypes = { "Hyena", "Hyena", "Hyena", "Shark", "Lancelot" }
   local shiptype = shiptypes[ rnd.rnd( 1, #shiptypes ) ]
   thug = pilot.addRaw( shiptype, "baddie", param, "Comingout_thugs" )

   thug:setHostile()
   thug:rename( "Thug " .. shiptype )

   hook.pilot( thug, "death", "thug_removed" )
   hook.pilot( thug, "jump", "thug_removed" )
   hook.pilot( thug, "land", "thug_removed" )
end


function jumpNext ()
   if chelsea ~= nil and chelsea:exists() then
      chelsea:taskClear()
      chelsea:control()
      if system.cur() == missys then
         chelsea:land( misplanet, true )
      else
         chelsea:hyperspace( getNextSystem( system.cur(), missys ), true )
      end
   end
end


function takeoff ()
   spawnChelseaShip( startplanet )
   jumpNext()
   spawnThug()
end


function jumpout ()
   lastsys = system.cur()
end


function jumpin ()
   if chelsea_jumped and system.cur() == getNextSystem( lastsys, missys ) then
      spawnChelseaShip( lastsys )
      jumpNext()
      hook.timer( 5000, "thug_timer" )
   else
      fail( plflee_msg )
   end
end


function land ()
   if planet.cur() == misplanet then
      tk.msg( title[6], text[6]:format( creditstring( credits ) ) )
      player.pay( credits )
      srm_addComingOutLog( log_text )
      misn.finish( true )
   else
      tk.msg( title[5], text[5] )
      misn.finish( false )
   end
end


function thug_timer ()
   spawnThug()
   if system.cur() == missys then
      spawnThug( lastsys )
   end
end


function chelsea_death ()
   fail( chelkill_msg )
end


function chelsea_jump( p, jump_point )
   if jump_point:dest() == getNextSystem( system.cur(), missys ) then
      player.msg( cheljump_msg:format( jump_point:dest():name() ) )
      chelsea_jumped = true
   else
      fail( chelflee_msg )
   end
end


function chelsea_land( p, planet )
   if planet == misplanet then
      player.msg( chelland_msg:format( planet:name() ) )
      chelsea_jumped = true
   else
      fail( chelflee_msg )
   end
end


function chelsea_attacked ()
   if chelsea ~= nil and chelsea:exists() then
      chelsea:control( false )
      if distress_timer_hook ~= nil then hook.rm( distress_timer_hook ) end
      distress_timer_hook = hook.timer( 1000, "chelsea_distress_timer" )
   end
end


function chelsea_distress_timer ()
   jumpNext()
end


function thug_removed ()
   spawnThug()
   if distress_timer_hook ~= nil then hook.rm( distress_timer_hook ) end
   jumpNext()
end


-- Fail the mission, showing message to the player.
function fail( message )
   if message ~= nil then
      -- Pre-colourized, do nothing.
      if message:find("#") then
         player.msg( message )
      -- Colourize in red.
      else
         player.msg( "#r" .. message .. "#0" )
      end
   end
   misn.finish( false )
end
