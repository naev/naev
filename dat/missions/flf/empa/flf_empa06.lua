--[[

   A Common Enemy

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

include "fleethelper.lua"
include "dat/missions/flf/flf_pirates.lua"

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
   title = {}
   text = {}

   title[1] = "A New Common Enemy"
   text[1] = [[Cheryl seems pleased to see you. She motions for you to sit. "I have some good news, %s", she says. "As you know, having personally had to deal with this lately, we have had some... dissidents. Of course, this is a serious problem.
    "However, we have come up with a solution. See, a large group of pirates has been terrorizing the %s system lately. It's come to the point where no one is willing to put a stop to them. So that's where we come in. This is a perfect opportunity to change our operations to suit the Empire's wishes. So I need you to lead the first of one of many missions protecting the Frontier from pirate terror. Are you ready?"]]

   text[2] = [[Perfect! After this mission, we will immediately cease assigning missions to attack the Dvaered forces. I am confident that the Dvaereds will begin to open up to us as we clean Frontier-Dvaered border systems of dangerous pirates. Good luck!]]

   title[3] = "A New Horizon"
   text[3] = [[As you enter Sindbad after successfully completing your mission, you are warmly greeted by not only Cheryl, but several FLF soldiers. A few FLF personnel still seem dissatisfied, but you quickly brush them off. Considering how positive the reception to this mission has been, you are certain they will come around before long.
    "Congratulations on your victory, %s!" Cheryl says, with the firm approval of many of your comrades. "This has definitely been the most important mission you will ever complete. Not only have you made the %s system safe from piracy again, I have received word that the Dvaereds have taken notice."]]

   text[4] = [["This is just the beginning! From now on, missions to eliminate pirate scum from all of FLF space will be offered in our mission computers. Everyone, fight the good fight! We will show everyone, including the Dvaereds, that we are not, as they say, a terrorist organization. We are the keepers of security for the Frontier, and we will not allow criminals to run rampant in our space! For the frontier!" That last sentence earns Cheryl a cheer from the audience. She hands you your pay, then excuses herself while your comrades continue to congratulate you for your success and express hope about the future of the FLF and the Frontier. Things are finally starting to look up!]]

   misn_rwrd = "A new beginning for the FLF."
    
   npc_name = "Cheryl"
   npc_desc = "Cheryl appears to want to speak with you again. Another mission, perhaps?"
end


function create ()
   missys = system.get( "Theras" )
   if not misn.claim( missys ) then misn.finish( false ) end

   level = 6
   has_ancestor = false
   ships = 8
   has_kestrel = true
   flfships = 16
   reputation = 20
   other_reputation = 2

   credits = 1000000

   late_arrival = false
   late_arrival_delay = rnd.rnd( 10000, 120000 )

   misn.setNPC( npc_name, "neutral/miner2" )
   misn.setDesc( npc_desc )
end


function accept ()
   if tk.yesno( title[1], text[1]:format( player.name(), missys:name() ) ) then
      tk.msg( title[1], text[2] )

      misn.accept()
      osd_desc[1] = osd_desc[1]:format( missys:name() )
      misn.osdCreate( osd_title, osd_desc )

      local desc
      if ships == 1 then
         desc = misn_desc[2]:format( missys:name() )
      else
         desc = misn_desc[1]:format( ships, missys:name() )
      end
      if has_ancestor then desc = desc .. misn_desc[3] end
      if has_kestrel then desc = desc .. misn_desc[4] end
      if flfships > 0 then
         desc = desc .. misn_desc[5]:format( flfships )
      end
      misn.setDesc( desc )

      misn.setTitle( misn_title:format( misn_level[level], missys:name() ) )
      marker = misn.markerAdd( missys, "high" )
      misn.setReward( misn_rwrd )

      pirate_ships_left = 0
      job_done = false

      hook.enter( "enter" )
      hook.jumpout( "leave" )
      hook.land( "leave" )

      diff.apply( "flf_vs_pirates" )
   else
   end
end


function land_flf ()
   leave()
   if planet.cur():faction():name() == "FLF" then
      tk.msg( title[3], text[3]:format( player.name(), missys:name() ) )
      tk.msg( title[3], text[4] )
      player.pay( credits )
      faction.get("FLF"):modPlayerSingle( reputation )
      faction.get("Frontier"):modPlayerSingle( other_reputation )
      if missys:presences()[ "Dvaered" ] then
         faction.get("Dvaered"):modPlayerSingle( other_reputation )
      end
      if missys:presences()[ "Sirius" ] then
         faction.get("Sirius"):modPlayerSingle( other_reputation )
      end
      if missys:presences()[ "Empire" ] then
         faction.get("Empire"):modPlayerSingle( other_reputation )
      end
      misn.finish( true )
   end
end


function abort ()
   diff.remove( "flf_vs_pirates" )
end

