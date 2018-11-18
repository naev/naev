--[[

   Pirate Alliance
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

-- Localization
title = {}
text = {}

title[1] = _("The enemy of my enemy...")
text[1] = _([[Benito motions for you to sit. She doesn't seem quite as calm and relaxed as she usually is.
    "Hello again, %s. Look: we have a pretty bad situation here. As you may have guessed, we rely on... unconventional sources for supplies. Unfortunately, we seem to have hit a bit of a snag. See, one of our important sources has stopped supplying us, and I fear we may be cut off and no longer able to carry out our operations before long if we don't do something.
    "But that being said, I think I may have found a solution. See, we have reason to believe that we are actually neighboring a pirate stronghold. We're not entirely sure, but we have detected some evidence of occasional pirate activity in the nearby %s system."]])

text[2] = _([[You raise an eyebrow. It seems rather odd that pirates would be in such a remote system. Perhaps it could be a gateway of some sort?
    "You must be thinking the same thing," Benito pipes up. "Yes, that is a very strange system to see pirates in, even occasionally. That's why we think there is a secret pirate stronghold nearby. It may even be the one associated with piracy in the Frontier.
    "We must establish trading relations with that stronghold at once. This could give us just the edge we need against the Dvaereds. I honestly don't know how you can go about doing it, but my recommendation would be to go to the %s system and see if you find any pirates. Tell them you're on official FLF business, and that we're seeking to become trade partners with them. Are you in?"]])

title[3] = _("...is my friend.")
text[3] = _([["Excellent! I knew you would do it." Benito becomes visibly more relaxed, almost her usual self. "Now, %s, I'm sure you're well aware of this, but please remember that pirates are extremely dangerous. They will probably attack you, and they may have demands. I'm counting on you to overcome any... obstacles you may encounter and secure a deal." You nod in understanding. "Good," she says. "Report back here with your results." Benito then excuses herself, presumably to take care of other things.]])

title[4] = _("...is still my enemy.")
text[4] = _([["That's too bad. I understand where you're coming from, though. Please feel free to return if you are willing to take on this mission at a later date."]])

title[5] = _("Who are you calling a weakling?")
text[5] = _([[A scraggly-looking pirate appears on your viewscreen. You realize this must be the leader of the group. "Bwah ha ha!" he laughs. "That has to be the most pathetic excuse for a ship I've ever seen!" You try to ignore his rude remark and start to explain to him that you just want to talk. "Talk?" he responds. "Why, that's the stupidest thing I've ever heard! Why would I want to talk to a weakling like you? Why, I'd bet my mates right here could blow you out of the sky even without my help!"
    With that, the pirate immediately cuts his connection. Well, if these pirates won't talk to "weaklings", maybe it's time to show him who the real weakling is. Destroying just one or two of his escorts should do the trick.]])

title[6] = _("Mission Failure")
text[6] = _([[As the Pirate Kestrel is blown out of the sky, it occurs to you that you have made a terrible mistake. Having killed off the leader of the pirate group, you have lost your opportunity to negotiate a trade deal with the pirates. You shamefully transmit your result to Benito via a coded message and abort the mission. Perhaps you will be given another opportunity later.]])

title[7] = _("Still Not Impressed")
text[7] = _([[The pirate leader comes on your screen once again. "Lucky shot, but you're still a pathetic weakling!" he says before promptly terminating the connection once again. Perhaps you need to destroy some more of his escorts so he can see who the real weakling is.]])

title[8] = _("Not So Weak After All")
text[8] = _([[The pirate comes on your view screen once again, but his expression has changed this time. You come to the realization that he is finally willing to talk and suppress a sigh of relief.
    "Perhaps you're not so bad after all," he says. Funny how destroying his "mates" impresses him. Not the slightest hint of devotion to his comrades. Still, you hold back the urge to tell him off. He continues. "I've misjudged you lot. I guess FLF pilots can fight after all."]])

text[9] = _([[You begin to talk to the pirate about what you and the FLF are after. "Supplies, eh? Yeah, we've got supplies, alright. Heh, heh, heh... but it'll cost you!" You inquire as to what the cost might be. "Simple, really. We want to build another base in the %s system. We can do it ourselves, of course, but if we can get you to pay for it, even better! Specifically, we need %s more tons of ore to build the base. So you bring it back to the Anger system, and we'll call it a deal!
    "Oh yeah, I almost forgot; you don't know how to get to the Anger system, now, do you? Well, since you've proven yourself worthy, I suppose I'll let you in on our little secret." He transfers a file to your ship's computer. When you look at it, you see that it's a map showing a single hidden jump point. "Now, away with you! Meet me in the %s system when you have the loot."]])

title[10] = _("I knew we could work something out")
text[10] = _([["Ha, you came back after all! Wonderful. I'll just take that ore, then." You hesitate for a moment, but considering the number of pirates around, they'll probably take it from you by force if you refuse at this point. You jettison the cargo into space, which the Kestrel promptly picks up with a tractor beam. "Excellent! Well, it's been a pleasure doing business with you. Send your mates over to the new station whenever you're ready. It should be up and running in just a couple periods or so. And in the meantime, you can consider yourselves one of us! Bwa ha ha!"
    You exchange what must for lack of a better word be called pleasantries with the pirate, with him telling a story about a pitifully armed Mule he recently plundered and you sharing stories of your victories against Dvaered scum. You seem to get along well. You then part ways. Now to report to Benito....]])

title[11] = _("Just The Edge We Need")
text[11] = _([[You greet Benito in a friendly manner as always, sharing your story and telling her the good news before handing her a chip with the map data on it. She seems pleased. "Excellent," she says. "We'll begin sending our trading convoys out right away. We'll need lots of supplies for our next mission! Thank you for your service, %s. Your pay has been deposited into your account. It will be a while before we'll be ready for your next big mission, so you can do some missions on the mission computer in the meantime. And don't forget to visit the Pirate worlds yourself and bring your own ship up to par!
    "Oh, one last thing. Make sure you stay on good terms with the pirates, yeah? The next thing you should probably do is buy a Skull and Bones ship; pirates tend to respect those who use their ships more than those who don't. And make sure to destroy Dvaered scum with the pirates around! That should keep your reputation up." You make a mental note to do what she suggests as she excuses herself and heads off.]])

comm_pirate = _("Har, har, har! You're hailing the wrong ship, buddy. Latest word from the boss is you're a weakling just waiting to be plundered!")
comm_pirate_friendly = _("I guess you're not so bad after all!")
comm_boss_insults = {}
comm_boss_insults[1] = _("You call those weapons? They look more like babies' toys to me!")
comm_boss_insults[2] = _("What a hopeless weakling!")
comm_boss_insults[3] = _("What, did you really think I would be impressed that easily?")
comm_boss_insults[4] = _("Keep hailing all you want, but I don't listen to weaklings!")
comm_boss_insults[5] = _("We'll have your ship plundered in no time at all!")
comm_boss_incomplete = _("Don't be bothering me without the loot, you hear?")

misn_title = _("Pirate Alliance")
misn_desc = _("You are to seek out pirates in the %s system and try to convince them to become trading partners with the FLF.")
misn_reward = _("Supplies for the FLF")

npc_name = _("Benito")
npc_desc = _("It seems Benito wants something from you again. Something about her looks a little off this time around.")

osd_title   = _("Pirate Alliance")
osd_desc    = {}
osd_desc[1] = _("Fly to the %s system")
osd_desc[2] = _("Find pirates and try to talk to (hail) them")
osd_desc["__save"] = true

osd_apnd    = {}
osd_apnd[3] = _("Destroy some of the weaker pirate ships, then try to hail the Kestrel again")
osd_apnd[4] = _("Bring %s tons of Ore to the Pirate Kestrel in the %s system")

osd_final   = _("Return to FLF base")
osd_desc[3] = osd_final


function create ()
   missys = system.get( "Tormulex" )
   missys2 = system.get( "Anger" )
   if not misn.claim( missys ) then
      misn.finish( false )
   end

   misn.setNPC( npc_name, "flf/unique/benito" )
   misn.setDesc( npc_desc )
end


function accept ()
   tk.msg( title[1], text[1]:format( player.name(), missys:name() ) )
   if tk.yesno( title[1], text[2]:format( missys:name() ) ) then
      tk.msg( title[3], text[3]:format( player.name() ) )

      misn.accept()

      osd_desc[1] = osd_desc[1]:format( missys:name() )
      misn.osdCreate( osd_title, osd_desc )
      misn.setTitle( misn_title )
      misn.setDesc( misn_desc:format( missys:name() ) )
      marker = misn.markerAdd( missys, "plot" )
      misn.setReward( misn_reward )

      stage = 0
      pirates_left = 0
      boss_hailed = false
      boss_impressed = false
      boss = nil
      pirates = nil
      boss_hook = nil

      ore_needed = 300
      credits = 300000
      reputation = 10
      pir_reputation = 10
      pir_starting_reputation = faction.get("Pirate"):playerStanding()

      hook.enter( "enter" )
   else
      tk.msg( title[4], text[4] )
   end
end


function pilot_hail_pirate ()
   player.commClose()
   if stage <= 1 then
      player.msg( comm_pirate )
   else
      player.msg( comm_pirate_friendly )
   end
end


function pilot_hail_boss ()
   player.commClose()
   if stage <= 1 then
      if boss_impressed then
         stage = 2
         local standing = faction.get("Pirate"):playerStanding()
         if standing < 25 then
            faction.get("Pirate"):setPlayerStanding( 25 )
         end

         if boss ~= nil then
            boss:changeAI( "pirate" )
            boss:setHostile( false )
            boss:setFriendly()
         end
         if pirates ~= nil then
            for i, j in ipairs( pirates ) do
               if j:exists() then
                  j:changeAI( "pirate" )
                  j:setHostile( false )
                  j:setFriendly()
               end
            end
         end

         tk.msg( title[8], text[8] )
         tk.msg( title[8], text[9]:format(
            missys2:name(), numstring( ore_needed ), missys2:name() ) )

         player.addOutfit( "Map: FLF-Pirate Route" )
         if marker ~= nil then misn.markerRm( marker ) end
         marker = misn.markerAdd( missys2, "plot" )

         osd_desc[4] = osd_apnd[4]:format( numstring( ore_needed ), missys2:name() )
         osd_desc[5] = osd_final
         misn.osdCreate( osd_title, osd_desc )
         misn.osdActive( 4 )
      else
         if boss_hailed then
            player.msg( comm_boss_insults[ rnd.rnd( 1, #comm_boss_insults ) ] )
         else
            boss_hailed = true
            if stage <= 0 then
               tk.msg( title[5], text[5] )
               osd_desc[3] = osd_apnd[3]
               osd_desc[4] = osd_final
               misn.osdCreate( osd_title, osd_desc )
               misn.osdActive( 3 )
            else
               tk.msg( title[7], text[7] )
            end
         end
      end
   elseif player.pilot():cargoHas( "Ore" ) >= ore_needed then
      tk.msg( title[10], text[10] )
      stage = 3
      player.pilot():cargoRm( "Ore", ore_needed )
      hook.rm( boss_hook )
      hook.land( "land" )
      misn.osdActive( 5 )
      if marker ~= nil then misn.markerRm( marker ) end
   else
      player.msg( comm_boss_incomplete )
   end
end


function pilot_death_pirate ()
   if stage <= 1 then
      pirates_left = pirates_left - 1
      stage = 1
      boss_hailed = false
      if pirates_left <= 0 or rnd.rnd() < 0.25 then
         boss_impressed = true
      end
   end
end


function pilot_death_boss ()
   tk.msg( title[6], text[6] )
   misn.finish( false )
end


function enter ()
   if stage <= 1 then
      stage = 0
      if system.cur() == missys then
         pilot.clear()
         pilot.toggleSpawn( false )
         local r = system.cur():radius()
         local vec = vec2.new( rnd.rnd( -r, r ), rnd.rnd( -r, r ) )

         local bstk = pilot.add( "Pirate Kestrel", "pirate_norun", vec )
         boss = bstk[1]
         hook.pilot( boss, "death", "pilot_death_boss" )
         hook.pilot( boss, "hail", "pilot_hail_boss" )
         boss:setHostile()
         boss:setHilight( true )

         pirates_left = 4
         pirates = addShips( "Pirate Hyena", "pirate_norun", vec, pirates_left )
         for i, j in ipairs( pirates ) do
            hook.pilot( j, "death", "pilot_death_pirate" )
            hook.pilot( j, "hail", "pilot_hail_pirate" )
            j:setHostile()
         end

         misn.osdActive( 2 )
      else
         osd_desc[3] = osd_final
         osd_desc[4] = nil
         misn.osdCreate( osd_title, osd_desc )
         misn.osdActive( 1 )
      end
   elseif stage <= 2 then
      if system.cur() == missys2 then
         local r = system.cur():radius()
         local vec = vec2.new( rnd.rnd( -r, r ), rnd.rnd( -r, r ) )

         local bstk = pilot.add( "Pirate Kestrel", "pirate_norun", vec )
         boss = bstk[1]
         hook.pilot( boss, "death", "pilot_death_boss" )
         boss_hook = hook.pilot( boss, "hail", "pilot_hail_boss" )
         boss:setFriendly()
         boss:setHilight( true )
      end
   end
end


function land ()
   if stage >= 3 and planet.cur():faction() == faction.get( "FLF" ) then
      tk.msg( title[11], text[11]:format( player.name() ) )
      diff.apply( "Fury_Station" )
      diff.apply( "flf_pirate_ally" )
      player.pay( credits )
      flf_setReputation( 55 )
      faction.get("FLF"):modPlayer( reputation )
      faction.get("Pirate"):modPlayerSingle( pir_reputation )
      misn.finish( true )
   end
end


function abort ()
   faction.get("Pirate"):setPlayerStanding( pir_starting_reputation )
   local hj1 = nil
   local hj2 = nil
   hj1, hj2 = jump.get( "Tormulex", "Anger" )
   hj1:setKnown( false )
   hj2:setKnown( false )
   misn.finish( false )
end

