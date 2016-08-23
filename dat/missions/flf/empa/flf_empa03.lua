--[[

   Raelid Outpost Rescue
   Copyright (C) 2014-2016 Julie Marchant <onpon4@riseup.net>

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

include "fleethelper.lua"
include "dat/missions/flf/flf_common.lua"

-- localization stuff
lang = naev.lang()
if lang == "notreal" then
else -- default English
   title = {}
   text = {}

   title[1] = "The Deal"
   text[1] = [["Ah, there you are, %s!" She motions for you to sit down and orders you a drink. "We have looked over the conditions set forth by the Empire. I have to be perfectly honest with you: not everyone is taking it well. I like the deal, personally. I think it's the way to go to actually make progress, rather than just blowing things up as we have been doing. Most of the other higher-ups agree with me, but when word got out of the deal, a lot of lower-ranking officials were outraged.
    "I'm sorry, I should explain why this is controversial. See, one of the main conditions the Empire set forth was that we have to gradually put an end to all aggression against House Dvaered, and to prove that we are willing to do this, we have to agree to remove explosives that were planted in Raelid Outpost recently."]]

   title[2] = "What explosives?"
   text[2] = [[Cheryl sees the confusion on your face. "Ah, yes, I had forgotten. You assisted in that mission, but you were not provided specific details. See, back when you were diverting attention away from Raelid, the operation at hand was to plant hidden explosives in Raelid Outpost. It was a very complex plan that took months of planning and quite a few pay-offs and was going to end in the destruction of that outpost.
    "Well, the Empire apparently noticed our actions; they don't know where we planted the explosives or what exactly we were planning on doing with them, but they detected the explosives aboard one of our ships and saw them land on the outpost."]]

   title[3] = "The Mission"
   text[3] = [["So, anyway, that's your mission, should you choose to accept it: to disarm and remove all of the explosives planted in Raelid Outpost. I know this is outside of your normal job responsibilities, but you are the only one of us who has personally earned the Empire's trust and this will also help restore trust between you and the Dvaereds, if the mission is successful."
    Cheryl takes a deep breath. "Are you ready for this mission now?"]]

   text[4] = [["Wonderful! Now, there is a chance that this could be a trap. I don't think it is, but a couple people insisted on this: you will be escorted by several FLF pilots. Your escorts will meet with you when you get to Raelid. However, they have been instructed NOT to engage the Dvaereds except as an absolute last resort. You, especially, must follow this rule. If you act aggressively against Dvaered ships, the Empire will surely not only cancel the deal, but begin earnestly assisting the Dvaereds against us. We cannot survive if that happens."]]

   text[5] = [[Cheryl hands you a chip. "This data chip contains the authorization code you will need to land on Raelid Outpost, a map showing you the locations of all of the explosives, and instructions for how to disarm them," she explains. "Before you can land, you will need to wait for Raelid Outpost to contact you. When they do, transmit the authorization code on that chip. Once you are given clearance, land and complete your mission. Report back here when you are finished."
    She stands, as do you. "Good luck, %s. I hope you are successful. We will await your return with great anticipation."
    As Cheryl leaves, you notice her walking past an old acquaintance: Corporal Benito! How odd; she is drinking alone, and she seems troubled. What could possibly be on her mind?]]

   title[6] = "Benito's Grievance"
   text[6] = [[As you approach her, she motions for you to sit down wordlessly. You do so.
    After a few moments, she begins to speak. "%s, you're not honestly going along with all of this, are you? Allying with the Empire? As if! I have seen the conditions being laid out, and it's obvious: the Empire is setting us up." She looks up at you expectantly. Unsure of what to say, you tell Benito that it is your duty to do what you can to complete your mission, but that you will remain vigilant. She frowns.]]
   
   text[7] = [["Look, you're a bright kid. When I sent you on your first mission, you took our side, the side of justice. So please be receptive to what I am about to say: the higher-ups, they think we can appease the Empire by agreeing to their ridiculous terms, and that they will in turn somehow put a stop to Dvaered advancement.
    "This is all nothing more than a farce! It's clear as day; the Empire is playing us. They will cripple our forces little by little, discover all our secrets, and then when we least expect it and are most helpless, they will attack. All of this, everything we have fought and died for, will be gone, and the Dvaereds will complete their conquest of the Frontier."]]

   text[8] = [[You sit in silence for a few moments. Benito continues, but in a low whisper. "You will be escorted by some of our greatest fighters. They have their orders, but they will follow your decision first and foremost, whatever it may be. We can always... correct any discrepancies later, so to speak, as long as everyone is on board. I know you will make the right decision, %s. Choose justice, not appeasement."
    Benito picks up her things and walks off. What should you do? Should you continue with the mission as planned, or should you put the brakes on the idea before it's too late? You will have to decide by the time you get to Raelid.]]

   title[9] = "Sabotage"
   text[9] = [[You are hailed by Raelid Outpost. When you respond, a man appears on your view screen. "Please transmit your auth-" the transmission stops. Your communication systems are jammed!]]

   title[10] = "The Choice"
   text[10] = [[To your astonishment, everyone, Dvaered and FLF alike, stops dead in their tracks when an Empire Pacifier intervenes. The pilot aboard it shows a strong air of authority you have never seen in anyone else. When the sense of wonder passes, you realize that the pilot is none other than Commander Petridis! You never knew he had this in him. Then again, you have never seen him in a combat ship before.
    Petridis immediately starts lecturing the Dvaered fleet. "What the hell are you doing?! Going against protocol like that! You KNOW that %s has been tasked with a mission for the Empire!" He continues to go on saying that the Dvaered pilots are only breeding more terrorists with their aggressive action, while the Dvaered pilots argue back that you cannot be trusted due to your association with the FLF.]]

   text[11] = [[Someone from the FLF fleet sends you an encrypted message. "%s, this is the perfect opportunity to put an end to these Dvaered scum! We should attack them now while they are distracted. What do you say?"
    At the same time, you notice that you are being hailed by Raelid Outpost again. The Dvaered fleet's communications jamming has stopped. What do you do?]]

   choice1 = "Attack the Dvaered fleet"
   choice2 = "Answer the hail"

   title[12] = "Defect"
   text[12] = [[You announce to all of the FLF pilots that you have decided to engage the Dvaered forces. This is met with cheers. "Well met, soldier," one of the pilots says. "Down with the Dvaered scum!"
    Another pilot says to you, "We can take everything from here. Go back to base and report. Here is the story: you were mercilessly attacked by Dvaereds aided by the Empire before you could complete your mission. The whole thing was a trap. We had no choice but to defend you, so we did." The rest of the pilots agree with the story. So now, you must return to base and present this story to Cheryl.]]

   title[13] = "Beginning of the End"
   text[13] = [[When you approach Cheryl, she seems surprised that you do not have the disarmed explosives. "What happened?" she asks. You explain the story, exactly as you and the other FLF pilots agreed. Is it just you, or does Cheryl seem skeptical? "That is very unfortunate," she says after a short pause. "In that case, we will have to abandon the whole idea of getting help from the Empire.
    "I honestly don't know what we're going to do now. I'm certain the Empire will be aiding the Dvaereds against us from now on, and we were having trouble fighting just the Dvaereds. But what's done is done. Here is your payment for attempting to complete the mission." She hands you a credit chip. "I hope we can turn around this dire situation."]]

   title[14] = "A Good Call"
   text[14] = [[The same man appears on your view screen. "Sorry about that," he says. "I don't know what those pilots were thinking. Anyway, please transit your authorization code." You do so. "Thank you," he says. "You may land when ready."
    As you close the transmission with Raelid Outpost, you see that the FLF fleet is beginning to leave the system. Their duty in this mission is over, so it is perfectly reasonable, but you can't help but wonder why they aren't sending you any transmissions.]]

   title[15] = "Disarmament"
   text[15] = [[Following the map you have been given, you locate all of the explosives on the outpost and disarm them. You are accompanied by a Dvaered officer along the way. When you are done, you take them into the cockpit of your ship. Now all you have to do is return to the base.]]

   title[16] = "Mission Success"
   text[16] = [[When you return to Sindbad, you get a strange feeling of coldness from many people which makes you wonder if you might have made a mistake. However, when Cheryl sees you, she appears to be overjoyed. "Congratulations on your success, %s! You have no idea how vital that mission was. I heard a small fleet of Dvaereds gave you a little bit of trouble, but you succeeded in the end."
    She hands you the credit chip with your payment. "I need to go discuss things with the other leaders. We will have another mission ready for you shortly." Well, at least Cheryl appreciates your work, but will the others come to appreciate your work as well? Only time will tell.]]

   flfcomm = "I knew they couldn't be trusted! Everyone, fire at will!"
   empcomm = "EVERYBODY STOP, in the name of the Empire!"

   misn_title = "Raelid Outpost Rescue"
   misn_desc = "Go to Raelid Outpost and remove the hidden explosives planted there."
   misn_reward = "Sealing the deal with the Empire"

   npc_name = "Cheryl"
   npc_desc = "Cheryl is walking around the room, seemingly looking for someone."

   def_name = "Benito"
   def_desc = "Corporal Benito is sitting alone. She appears troubled."

   osd_title   = "Raelid Outpost Rescue"
   osd_desc    = {}
   osd_desc[1] = "Go to the %s system"
   osd_desc[2] = "Approach %s and wait for contact"
   osd_desc[3] = "Land on %s and remove all explosives"
   osd_desc[4] = "Return to FLF base and report back to Cheryl"
   osd_desc["__save"] = true
end


function create ()
   -- Note: this mission does not make any system claims.
   missys = system.get( "Raelid" )
   misplanet = planet.get( "Raelid Outpost" )

   credits = 100000
   reputation = 15
   emp_reputation = 5
   dv_reputation = 5

   alt_credits = 50000

   misn.setNPC( npc_name, "neutral/miner2" )
   misn.setDesc( npc_desc )
end


function accept ()
   tk.msg( title[1], text[1]:format( player.name() ) )
   tk.msg( title[2], text[2] )
   if tk.yesno( title[3], text[3] ) then
      tk.msg( title[3], text[4] )
      tk.msg( title[3], text[5]:format( player.name() ) )

      misn.accept()

      osd_desc[1] = osd_desc[1]:format( missys:name() )
      osd_desc[2] = osd_desc[2]:format( misplanet:name() )
      osd_desc[3] = osd_desc[3]:format( misplanet:name() )
      misn.osdCreate( osd_title, osd_desc )
      misn.setTitle( misn_title )
      misn.setDesc( misn_desc )
      marker = misn.markerAdd( missys, "high" )
      misn.setReward( misn_reward )

      job_done = false
      job_aborted = false

      npc = misn.npcAdd( "approach", def_name, "flf/unique/benito", def_desc )

      hook.enter( "enter" )
      hook.jumpout( "jumpout" )
      hook.land( "land" )
   else
   end
end


function approach ()
   tk.msg( title[6], text[6]:format( player.name() ) )
   tk.msg( title[6], text[7] )
   tk.msg( title[6], text[8]:format( player.name() ) )
   misn.npcRm( npc )
end


function enter ()
   if not job_done and not job_aborted then
      if system.cur() == missys then
         misn.osdActive( 2 )

         pilot.clear()
         pilot.toggleSpawn( false )
         player.pilot():setVisible( true )
         player.allowLand( false )

         fleetFLF = addShips(
            { "FLF Lancelot", "FLF Vendetta" }, "flf_norun", last_system, 8 )
         for i, j in ipairs( fleetFLF ) do
            j:setFriendly()
            j:setVisible( true )
            j:control()
            j:follow( player.pilot() )
         end

         timer_hook = hook.timer( 10000, "timer_raelidContact" )
      else
         misn.osdActive( 1 )
      end
   end
end


function jumpout ()
   last_system = system.cur()
   player.allowLand()
end


function timer_raelidContact ()
   if timer_hook ~= nil then hook.rm( timer_hook ) end

   local player_pos = player.pilot():pos()
   local outpost_pos = misplanet:pos()
   if player_pos:dist( outpost_pos ) < 5000 then
      tk.msg( title[9], text[9] )

      fleetDV = addShips(
         { "Dvaered Ancestor", "Dvaered Vendetta" }, nil, misplanet, 3 )
      for i, j in ipairs( fleetDV ) do
         j:setHostile()
         j:setVisible( true )
      end

      timer_hook = hook.timer( 3000, "timer_flfAttack" )
   else
      timer_hook = hook.timer( 2000, "timer_raelidContact" )
   end
end


function timer_flfAttack ()
   if timer_hook ~= nil then hook.rm( timer_hook ) end

   local comm_done = false
   hookFLF = {}
   hookDV = {}
   for i, j in ipairs( fleetFLF ) do
      if j:exists() then
         j:control( false )
         hookFLF[i] = hook.pilot( j, "death", "pilot_death" )

         if not comm_done then
            j:comm( flfcomm )
            comm_done = true
         end
      end
   end
   for i, j in ipairs( fleetDV ) do
      if j:exists() then
         hookDV[i] = hook.pilot( j, "death", "pilot_death" )
      end
   end
end


function pilot_death ()
   local pos = player.pos() + vec2.new( -3000, 3000 )
   petridis = pilot.add( "Empire Pacifier", nil, pos )[1]
   petridis:control()
   petridis:setVisible( true )
   petridis:comm( empcomm )

   local firstdv
   for i, j in ipairs( fleetFLF ) do
      if j:exists() then
         hook.rm( hookFLF[i] )
         j:control()
         j:brake()
         j:face( petridis, true )
      end
   end
   for i, j in ipairs( fleetDV ) do
      if j:exists() then
         hook.rm( hookDV[i] )
         j:control()
         j:brake()
         j:face( petridis, true )
         if firstdv == nil then firstdv = j end
      end
   end

   petridis:face( firstdv )
   timer_hook = hook.timer( 5000, "timer_lecture" )
end


function timer_lecture ()
   if timer_hook ~= nil then hook.rm( timer_hook ) end

   player.allowLand()

   tk.msg( title[10], text[10]:format( player.name() ) )
   choice = tk.choice( title[10], text[11]:format( player.name() ), choice1, choice2 )
   if choice == 1 then
      tk.msg( title[12], text[12] )

      misn.osdActive( 4 )
      if marker ~= nil then misn.markerRm( marker ) end
      job_aborted = true
      flf_setReputation( 75 )
      faction.get("FLF"):modPlayer( 15 )
      pilot.toggleSpawn( true )

      -- Make the Empire and FLF enemies
      diff.apply( "flf_vs_empire" )
      local standing = faction.get("Empire"):playerStanding()
      if standing >= 0 then
         faction.get("Empire"):setPlayerStanding( -1 )
      end

      if hookPetridis ~= nil then hook.rm( hookPetridis ) end
      if petridis ~= nil and petridis:exists() then
         petridis:control( false )
         petridis:setHostile()
      end
      for i, j in ipairs( fleetDV ) do
         if j:exists() then
            j:control( false )
         end
      end
      for i, j in ipairs( fleetFLF ) do
         if j:exists() then
            j:control( false )
         end
      end
   else
      tk.msg( title[14], text[14] )

      misn.osdActive( 3 )
      misplanet:landOverride( true )

      for i, j in ipairs( fleetFLF ) do
         if j:exists() then
            j:hyperspace( last_system )
         end
      end
   end
end


function land ()
   if planet.cur():faction():name() == "FLF" then
      if job_done then
         tk.msg( title[16], text[16]:format( player.name() ) )
         player.pay( credits )
         misn.finish( true )
      elseif job_aborted then
         tk.msg( title[13], text[13] )
         player.pay( alt_credits )
         misn.finish( true )
      end
   elseif planet.cur() == misplanet then
      if not job_done and not job_aborted then
         tk.msg( title[15], text[15] )
         misn.osdActive( 4 )
         if marker ~= nil then misn.markerRm( marker ) end
         job_done = true
         faction.get("Empire"):modPlayerSingle( 5 )
         faction.get("Dvaered"):modPlayerSingle( 5 )
         var.push( "flf_raelid_disarmed", true )
      end
   end
end


function abort ()
   if job_done or job_aborted then
      misn.finish( true )
   end
end

