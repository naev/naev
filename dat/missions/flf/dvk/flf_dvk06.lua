--[[

   Assault on Haleb
   Copyright (C) 2018 Julie Marchant <onpon4@riseup.net>

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
include "fleethelper.lua"
include "dat/missions/flf/flf_common.lua"

-- Localization stuff
title = {}
text = {}

title[1] = _("Here we go again")
text[1] = _([["%s, we were just saying you should join in on this one! It's another great victory against the Dvaered oppressors, and we'd like you to lead the way to victory once again! Are you in?"]])

title[2] = _("Not This Time")
text[2] = _([["Okay. Just let me know if you change your mind."]])

title[3] = _("Another Decisive Strike")
text[3] = _([["Excellent!" You take a seat. "So once again, our mission today is the destruction of a loathed Dvaered base: Raglan Outpost! The plan is pretty much the same as before: we have tasked a group of pirates with creating a disturbance nearby, and we have planted a bomb within the outpost to aid in its destruction. You just need to decide when to strike and let your teammates know.
    The one thing that will be different, though, is that you're likely to find more Dvaered ships guarding Raglan Outpost compared to Raelid Outpost, and it might be a little harder to destroy. So be extra careful!" Time to get your ship ready for battle, then.]])

title[4] = _("Another Day, Another Victory")
text[4] = _([[If your comrades were happy about your victory at Raelid, they are ecstatic about your victory at Haleb. As you enter the station, you are met with cheers from what seems to be everyone. It takes you longer than usual to make it to Benito as a result. "Congratulations," she says. "That was an astounding victory, sure to set back the Dvaered oppressors substantially! This is the first time we've pushed them out of Frontier space, and for that, we all thank you. If you haven't noticed, you've made yourself into a bit of a hero!
    "Here is your pay, %s. May we have another great victory later on! Down with the oppressors!" You exchange some more words with Benito, party with the others for a few hours, and then make your way back to your ship for some much-needed rest.]])
    

misn_title = _("Assault on Haleb")
misn_desc = _("Join with the other FLF pilots for the assault on Raglan Outpost.")
misn_reward = _("Another great victory against the Dvaereds")

npc_name = _("Benito")
npc_desc = _("Benito is seated at a table with several other FLF soldiers. She motions for you to come over.")

osd_title   = _("Assault on Haleb")
osd_desc    = {}
osd_desc[1] = _("Fly to the %s system and meet with the group of FLF ships")
osd_desc[2] = _("Wait until the coast is clear, then hail one of your wingmates")
osd_desc[3] = _("Attack Raglan Outpost until it is destroyed")
osd_desc[4] = _("Return to FLF base")
osd_desc["__save"] = true

flfcomm = {}
flfcomm[1] = _("You're just in time, %s! The chaos is just about to unfold.")
flfcomm[2] = _("You heard the boss! Let's grind that station to dust!")

civcomm = _("Help! SOS! We are under attack! In need of immediate assistance!")


function create ()
   missys = system.get( "Haleb" )
   if not misn.claim( missys ) then
      misn.finish( false )
   end

   misn.setNPC( npc_name, "flf/unique/benito" )
   misn.setDesc( npc_desc )
end


function accept ()
   if tk.yesno( title[1], text[1]:format( player.name() ) ) then
      tk.msg( title[3], text[3] )

      misn.accept()

      osd_desc[1] = osd_desc[1]:format( missys:name() )
      misn.osdCreate( osd_title, osd_desc )
      misn.setTitle( misn_title )
      misn.setDesc( misn_desc:format( missys:name() ) )
      marker = misn.markerAdd( missys, "plot" )
      misn.setReward( misn_reward )

      credits = 750000
      reputation = 10

      started = false
      attacked_station = false
      completed = false

      hook.enter( "enter" )
   else
      tk.msg( title[2], text[2] )
      misn.finish( false )
   end
end


function enter ()
   if not completed then
      started = false
      attacked_station = false
      misn.osdActive( 1 )
      if timer_start_hook ~= nil then hook.rm( timer_start_hook ) end
      if timer_pirates_hook ~= nil then hook.rm( timer_pirates_hook ) end

      if diff.isApplied( "raglan_outpost_death" ) then
         diff.remove( "raglan_outpost_death" )
      end

      if system.cur() == missys then
         pilot.clear()
         pilot.toggleSpawn( false )

         local ro, ms, s, nf

         ro, s = planet.get( "Raglan Outpost" )

         -- Spawn Raglan Outpost ship
         nf = pilot.add( "Raglan Outpost", nil, ro:pos() )
         dv_base = nf[1]
         dv_base:rmOutfit( "all" )
         dv_base:rmOutfit( "cores" )
         dv_base:addOutfit( "Dummy Systems" )
         dv_base:addOutfit( "Dummy Plating" )
         dv_base:addOutfit( "Dummy Engine" )
         dv_base:control()
         dv_base:setNodisable()
         dv_base:setNoboard()
         dv_base:setNoLand()
         dv_base:setVisible()
         dv_base:setHilight()
         hook.pilot( dv_base, "attacked", "pilot_attacked_station" )
         hook.pilot( dv_base, "death", "pilot_death_station" )

         -- Spawn Dvaered ships
         dv_fleet = {}

         local dv_ships = {
            "Dvaered Goddard", "Dvaered Big Patrol", "Dvaered Big Patrol",
            "Dvaered Home Guard", "Dvaered Ancestor", "Dvaered Ancestor"}
         dv_fleet = addShips( dv_ships, "dvaered_norun", ro:pos(), 1 )

         for i, j in ipairs( dv_fleet ) do
            j:control()
            j:setVisible()
            hook.pilot( j, "attacked", "pilot_attacked" )
         end

         -- Spawn FLF ships
         local jmp, jmp2
         jmp, jpm2 = jump.get( "Haleb", "Theras" )
         flf_fleet = addShips(
            {"FLF Vendetta", "FLF Vendetta", "FLF Lancelot"}, nil, jmp:pos(), 4 )

         for i, j in ipairs( flf_fleet ) do
            j:control()
            j:brake()
            j:face( dv_base:pos(), true )
            j:setVisplayer()
            j:setHilight()
            hook.pilot( j, "attacked", "pilot_attacked" )
         end

         timer_start_hook = hook.timer( 4000, "timer_start" )
         diff.apply( "raglan_outpost_death" )
      end
   end
end


function timer_start ()
   if timer_start_hook ~= nil then hook.rm( timer_start_hook ) end

   local player_pos = player.pilot():pos()
   local proximity = false
   for i, j in ipairs( flf_fleet ) do
      local dist = player_pos:dist( j:pos() )
      if dist < 500 then proximity = true end
   end

   if proximity then
      started = true
      flf_fleet[1]:comm( flfcomm[1]:format( player.name() ) )
      timer_pirates_hook = hook.timer( 4000, "timer_pirates" )
      misn.osdActive( 2 )

      for i, j in ipairs( flf_fleet ) do
         j:setHilight( false )
         hook.hail( "hail" )
      end

      civ_fleet = {}
      local choices = {
         "Civilian Llama", "Civilian Gawain", "Trader Llama",
         "Trader Koala", "Trader Mule" }
      local src = system.get( "Triap" )
      for i = 1, 16 do
         local choice = choices[ rnd.rnd( 1, #choices ) ]
         local nf = pilot.add( choice, nil, src )
         civ_fleet[ #civ_fleet + 1 ] = nf[1]
      end

      local dest = system.get( "Slaccid" )
      for i, j in ipairs( civ_fleet ) do
         j:control()
         j:hyperspace( dest )
         j:setVisible()
         hook.pilot( j, "attacked", "pilot_attacked_civilian" )
      end
   else
      timer_start_hook = hook.timer( 50, "timer_start" )
   end
end


function timer_pirates ()
   civ_fleet[1]:comm( civcomm )

   local src = system.get( "Triap" )

   pir_fleet = pilot.add( "Pirate Kestrel", nil, src )
   pir_boss = pir_fleet[1]
   hook.pilot( pir_boss, "death", "pilot_death_kestrel" )

   local choices = {
      "Pirate Hyena", "Pirate Shark", "Pirate Admonisher",
      "Pirate Vendetta", "Pirate Ancestor" }
   for i = 1, 9 do
      local choice = choices[ rnd.rnd( 1, #choices ) ]
      local nf = pilot.add( choice, nil, src )
      pir_fleet[ #pir_fleet + 1 ] = nf[1]
   end

   for i, j in ipairs( pir_fleet ) do
      j:control()
      j:setVisible()
      j:setFriendly()
      j:attack()
      hook.pilot( j, "attacked", "pilot_attacked" )
   end

   for i, j in ipairs( dv_fleet ) do
      if j:exists() then
         j:attack( pir_boss )
      end
   end
end


function hail ()
   player.commClose()
   if not attacked_station then
      local comm_done = false
      for i, j in ipairs( flf_fleet ) do
         if j:exists() then
            j:attack( dv_base )
            if not comm_done then
               j:comm( flfcomm[2] )
               comm_done = true
            end
         end
      end
      attacked_station = true
      misn.osdActive( 3 )
   end
end


function pilot_attacked( pilot, attacker, arg )
   pilot:control( false )
end


function pilot_attacked_civilian( pilot, attacker, arg )
   pilot:control( false )
   attacker:control( false )
end


function pilot_attacked_station( pilot, attacker, arg )
   for i, j in ipairs( dv_fleet ) do
      if j:exists() then
         j:control( false )
         j:setHostile()
      end
   end
   for i, j in ipairs( flf_fleet ) do
      if j:exists() then
         j:setVisible()
      end
   end
end


function pilot_death_civilian( pilot, attacker, arg )
   for i, j in ipairs( pir_fleet ) do
      if j:exists() then
         j:control( false )
      end
   end
end


function pilot_death_kestrel( pilot, attacker, arg )
   for i, j in ipairs( dv_fleet ) do
      if j:exists() then
         j:control( false )
      end
   end
end


function pilot_death_station( pilot, attacker, arg )
   for i, j in ipairs( flf_fleet ) do
      if j:exists() then
         j:control( false )
         j:changeAI( "flf" )
      end
   end
   for i, j in ipairs( pir_fleet ) do
      if j:exists() then
         j:control( false )
      end
   end

   completed = true
   pilot.toggleSpawn( true )
   misn.osdActive( 4 )
   if marker ~= nil then misn.markerRm( marker ) end
   hook.land( "land" )
end


function land ()
   if planet.cur():faction():name() == "FLF" then
      tk.msg( title[4], text[4]:format( player.name() ) )
      finish()
   end
end


function finish ()
   player.pay( credits )
   flf_setReputation( 92 )
   faction.get("FLF"):modPlayer( reputation )
   misn.finish( true )
end


function abort ()
   if completed then
      finish()
   else
      if diff.isApplied( "raglan_outpost_death" ) then
         diff.remove( "raglan_outpost_death" )
      end
      if dv_base ~= nil and dv_base:exists() then
         dv_base:rm()
      end
      misn.finish( false )
   end
end

