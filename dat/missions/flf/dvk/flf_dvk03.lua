--[[

   Assault on Raelid
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

title[1] = _("The Next Level")
text[1] = _([["Hello there, %s! You're just in time. We were just discussing our next operation against the Dvaered oppressors! Do you want in?"]])

title[2] = _("Not This Time")
text[2] = _([["Okay. Just let me know if you change your mind."]])

title[3] = _("A Decisive Strike")
text[3] = _([[You take a seat among the group. "Fantastic!" says Benito. "Let me get you caught up, then. Do you remember that mission in Raelid you helped with a while back?" You nod. You were wondering what you were actually creating a diversion from. "Yes, well, I never got around to telling you what we actually did there. See, we've been wanting to destroy Raelid Outpost for some time, mostly because it's often used as a front for trying to scout us out. So while you were getting the attention of those idiot Dvaered oppressors, we rigged a special bomb and covertly installed it onto the outpost!"]])

text[4] = _([["Now, the bomb is not perfect. Given how hastily we had to install the thing, we could not make it so that it could be detonated remotely. Instead, it has to be detonated manually, by blasting the station repeatedly. Shooting it down, in other words.
    "So here's the plan. We have hired a large group of pirates to help us out by creating a massive disturbance far away from our target. You are to wait until the coast is clear, then swarm in and attack the outpost with all you've got. You, %s, will lead the charge. You have to determine the optimal time, when the Dvaereds are far enough away for you to initiate the attack, but before the pirates are inevitably overwhelmed. Simply hail one of the others when it's time to attack, then make a beeline for Raelid Outpost and shoot at it with all you've got!"]])

text[5] = _([["You guys are some of our best pilots, so try not to get killed, eh? A moment of triumph is upon us! Down with the oppressors!" The last line earns Benito a cheer from the crowd. Well, time to get your ship ready for the battle.]])

title[6] = _("Ominous Foretelling")
text[6] = _([[As Raelid Outpost erupts into a fantastic explosion before your very eyes, your comrades cheer. But then, you see something odd. Someone is hailing you... an Empire ship? Surely this can't be. Cautiously, you answer. The man whose face comes up on your view screen wastes no time.
    "So, you actually showed your face. I half expected you to run away and hide. But no matter." You try not to show any reaction to his icy stare. He continues.]])

text[7] = _([["Terrorist, I'd bet you think this latest act of yours is a victory for you. Perhaps, for now, it is. But I assure you that the Empire will not ignore your activities any longer. I have already sent word to the Emperor, and he has authorized a declaraction of your organization, the FLF, as an enemy of the Empire. Count the minutes on your fingers, terrorist. Your days are numbered."
    The Empire officer then immediately ceases communication, and you suddenly feel a chill down your spine. But one of your wingmates snaps you out of it. "Pay the Empire no mind," he says. "More importantly, we have to get out of here! We'll meet you at Sindbad."]])

title[8] = _("Victory on the Horizon")
text[8] = _([[As you return to the base, you are welcomed with all manner of cheers and enthusiasm. You can understand why, too; this is a huge victory for the FLF, and surely just one of many victories to come. But still...
    You manage to make your way over to Benito, who is clearly pleased with the outcome. "Outstanding job!" she says. "That base has been a burden on us for so long. Now it is gone, 100% gone! I don't think I need to tell you how fantastic of a victory this is. Victory is within our grasp!" That's when all doubt is erased from your mind. She's right; so what if the Empire is against you now? You exchange some more words with Benito, after which she hands you your pay for a job well done and excuses herself. You, on the other hand, stay behind to celebrate for a few more hours before finally excusing yourself.]])

misn_title = _("Assault on Raelid")
misn_desc = _("Join with the other FLF pilots for the assault on Raelid Outpost.")
misn_reward = _("A great victory against the Dvaereds")

npc_name = _("Benito")
npc_desc = _("Benito is seated at a table with several other FLF soldiers. She motions for you to come over.")

osd_title   = _("Assault on Raelid")
osd_desc    = {}
osd_desc[1] = _("Fly to the %s system and meet with the group of FLF ships")
osd_desc[2] = _("Wait until the coast is clear, then hail one of your wingmates")
osd_desc[3] = _("Attack Raelid Outpost until it is destroyed")
osd_desc[4] = _("Return to FLF base")
osd_desc["__save"] = true

flfcomm = {}
flfcomm[1] = _("You're just in time, %s! The chaos is just about to unfold.")
flfcomm[2] = _("You heard the boss! Let's grind that station to dust!")

civcomm = _("Help! SOS! We are under attack! In need of immediate assistance!")


function create ()
   missys = system.get( "Raelid" )
   if not misn.claim( missys ) then
      misn.finish( false )
   end

   misn.setNPC( npc_name, "flf/unique/benito" )
   misn.setDesc( npc_desc )
end


function accept ()
   if tk.yesno( title[1], text[1]:format( player.name() ) ) then
      tk.msg( title[3], text[3] )
      tk.msg( title[3], text[4]:format( player.name() ) )
      tk.msg( title[3], text[5] )

      misn.accept()

      osd_desc[1] = osd_desc[1]:format( missys:name() )
      misn.osdCreate( osd_title, osd_desc )
      misn.setTitle( misn_title )
      misn.setDesc( misn_desc:format( missys:name() ) )
      marker = misn.markerAdd( missys, "plot" )
      misn.setReward( misn_reward )

      credits = 300000
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

      if diff.isApplied( "raelid_outpost_death" ) then
         diff.remove( "raelid_outpost_death" )
      end

      if system.cur() == missys then
         pilot.clear()
         pilot.toggleSpawn( false )

         local ro, ms, s, nf

         ro, s = planet.get( "Raelid Outpost" )
         ms, s = planet.get( "Marius Station" )

         -- Spawn Raelid Outpost ship
         nf = pilot.add( "Raelid Outpost", nil, ro:pos() )
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

         -- Spawn Dvaered and Empire ships
         dv_fleet = {}

         nf = pilot.add( "Empire Lge Attack", "empire_norun", ms:pos() )
         for i, j in ipairs( nf ) do
            dv_fleet[ #dv_fleet + 1 ] = j
         end

         nf = pilot.add( "Dvaered Big Patrol", "dvaered_norun", ro:pos() )
         for i, j in ipairs( nf ) do
            dv_fleet[ #dv_fleet + 1 ] = j
         end

         for i, j in ipairs( dv_fleet ) do
            j:control()
            j:setVisible()
            hook.pilot( j, "attacked", "pilot_attacked" )
         end

         -- Spawn FLF ships
         local jmp, jmp2
         jmp, jpm2 = jump.get( "Raelid", "Arcanis" )
         flf_fleet = addShips( "FLF Vendetta", nil, jmp:pos(), 14 )

         for i, j in ipairs( flf_fleet ) do
            j:control()
            j:brake()
            j:face( dv_base:pos(), true )
            j:setVisplayer()
            j:setHilight()
            hook.pilot( j, "attacked", "pilot_attacked" )
         end

         timer_start_hook = hook.timer( 4000, "timer_start" )
         diff.apply( "raelid_outpost_death" )
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
      local src = system.get( "Zacron" )
      for i = 1, 12 do
         local choice = choices[ rnd.rnd( 1, #choices ) ]
         local nf = pilot.add( choice, nil, src )
         civ_fleet[ #civ_fleet + 1 ] = nf[1]
      end

      local dest = system.get( "Tau Prime" )
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

   local src = system.get( "Zacron" )

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
   hook.timer( 3000, "timer_station" )
end


function timer_station ()
   tk.msg( title[6], text[6] )
   tk.msg( title[6], text[7] )

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
   faction.get("Empire"):setPlayerStanding( -100 )
   diff.apply( "flf_vs_empire" )
   misn.osdActive( 4 )
   if marker ~= nil then misn.markerRm( marker ) end
   hook.land( "land" )
end


function land ()
   if planet.cur():faction():name() == "FLF" then
      tk.msg( title[8], text[8] )
      finish()
   end
end


function finish ()
   player.pay( credits )
   flf_setReputation( 75 )
   faction.get("FLF"):modPlayer( reputation )
   misn.finish( true )
end


function abort ()
   if completed then
      finish()
   else
      if diff.isApplied( "raelid_outpost_death" ) then
         diff.remove( "raelid_outpost_death" )
      end
      if diff.isApplied( "flf_vs_empire" ) then
         diff.remove( "flf_vs_empire" )
      end
      if dv_base ~= nil and dv_base:exists() then
         dv_base:rm()
      end
      misn.finish( false )
   end
end

