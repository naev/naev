--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--

   This is the second "prelude" mission leading to the FLF campaign.
   stack variable flfbase_intro:
        1 - The player has turned in the FLF agent or rescued the Dvaered crew. Conditional for dv_antiflf02
        2 - The player has rescued the FLF agent. Conditional for flf_pre02
        3 - The player has found the FLF base for the Dvaered, or has betrayed the FLF after rescuing the agent. Conditional for dv_antiflf03

--]]

include "fleethelper.lua"
include "dat/missions/flf/flf_patrol.lua"

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
   title = {}
   text = {}
   DVtitle = {}
   DVtext = {}
   osd_desc = {}
   DVosd = {}
   refuelmsg = {}

   title[1] = "A chance to prove yourself"
   text[1] = [[The FLF officer doesn't seem at all surprised that you approached her. On the contrary, she looks like she expected you to do so all along.
    "Greetings," she says, nodding at you in curt greeting. "I am Corporal Benito. And you must be %s, the one who got Lt. Fletcher back here in one piece." Benito's expression becomes a little more severe. "I'm not here to exchange pleasantries, however. You probably noticed, but people here are a little uneasy about your presence. They don't know what to make of you, see. You helped us once, it is true, but that doesn't tell us much. We don't know you."]]

   text[2] = [[Indeed, you are constantly aware of the furtive glances the other people in this bar are giving you. They don't seem outright hostile, but you can tell that if you don't watch your step and choose your words carefully, things might quickly take a turn for the worse.
    Benito waves her hand to indicate you needn't pay them any heed. "That said, the upper ranks have decided that if you are truly sympathetic to our cause, you will be given an opportunity to prove yourself. Of course, if you'd rather not get involved in our struggle, that's understandable. But if you're in for greater things, if you stand for justice... Perhaps you'll consider joining with us?"]]

   title[3] = "Patrol-B-gone"
   text[3] = [["I'm happy to hear that. It's good to know we still have the support from the common man. Anyway, let me fill you in on what it is we want you to do. As you may be aware, the Dvaered have committed a lot of resources to finding us and flushing us out lately. And while our base is well hidden, those constant patrols are certainly not doing anything to make us feel more secure! I think you can see where this is going. You will go out there and eliminate one of those patrols in the %s system."
    You object, asking the Corporal if all recruits have to undertake dangerous missions like this to be accepted into the FLF ranks. Benito chuckles, and makes a pacifying gesture.
    "Calm down, it's not as bad as it sounds. You only have to take out one small patrol; I don't think you will have to fight more than 3 ships, 4 if you're really unlucky. If you think that's too much for you, you can abort the mission for now and come to me again later. Otherwise, good luck!"]]

   title[4] = "Breaking the ice"
   text[4] = [[When you left Sindbad Station, it was a cold, lonely place for you. The FLF soldiers on the station avoided you whenever they could, and basic services were harder to get than they should have been.
    But now that you have returned victorious over the Dvaered, the place has become considerably more hospitable. There are more smiles on people's faces, and some even tell you you did a fine job. Among them is Corporal Benito. She walks up to you and offers you her hand.]]

   text[5] = [["Welcome back, %s, and congratulations. I didn't expect the Dvaered to send reinforcements, much less a Vigilance. I certainly wouldn't have sent you alone if I did, and I might not have sent you at all. But then, you're still in one piece, so maybe I shouldn't worry so much, eh?"]]

   text[6] = [[Benito takes you to the station's bar, and buys you what for lack of a better word must be called a drink.
    "We will of course reward you for your service," she says once you are seated. "Though you must understand the FLF doesn't have that big a budget. Financial support is tricky, and the Frontier doesn't have that much to spare themselves to begin with. Nevertheless, we are willing to pay for good work, and your work is nothing but. What's more, you've ingratiated yourself with many of us, as you've undoubtedly noticed. Our top brass are among those you've impressed, so from today on, you can call yourself one of us! How about that, huh?"]]

   text[7] = [["Of course, our work is only just beginning. No rest for the weary; we must continue the fight against the oppressors. I'm sure the road is still long, but I'm encouraged by the fact that we gained another valuable ally today. Check the mission computer for more tasks you can help us with. I'm sure you'll play an important role in our eventual victory over the Dvaered!"
    That last part earns a cheer from the assembled FLF soldiers. You decide to raise your glass with them, making a toast to the fortune of battle in the upcoming campaign - and the sweet victory that lies beyond.]]

   refusetitle = "Some other time perhaps"
   refusetext = [["I see. That's a fair answer, I'm sure you have your reasons. But if you ever change your mind, I'll be around on Sindbad. You won't have trouble finding me, I'm sure."]]

   DVtitle[1] = "A tempting offer"
   DVtext[1] = [[Your viewscreen shows a Dvaered Colonel. He looks tense. Normally, a tense Dvaered would be bad news, but then this one bothered to hail you in the heat of battle, so perhaps there is more here than meets the eye.]]

   DVtext[2] = [["I am Colonel Urnus of the Dvaered Fleet, anti-terrorism division. I would normally never contact an enemy of House Dvaered, but my intelligence officer has looked through our records and found that you were recently a law-abiding citizen, doing honest freelance missions."]]

   DVtext[3] = [["I know your type, %s. You take jobs where profit is to be had, and you side with the highest bidder. There are many like you in the galaxy, though admittedly not so many with your talent. That's why I'm willing to make you this offer: you will provide us with information on their base of operations and their combat strength. In return, I will convince my superiors that you were working for me all along, so you won't face any repercussions for assaulting Dvaered ships. Furthermore, I will transfer a considerable amount of credits in your account, as well as put you into a position to make an ally out of House Dvaered. If you refuse, however, I guarantee you that you will never again be safe in Dvaered space. What say you? Surely this proposition beats anything that rabble can do for you?"]]

   DVchoice1 = "Accept the offer"
   DVchoice2 = "Remain loyal to the FLF"

   DVtitle[4] = "Opportunism is an art"
   DVtext[4] = [[Colonel Urnus smiles broadly. "I knew you'd make the right choice, citizen!" He addresses someone on his bridge, out of the view of the camera. "Notify the flight group. This ship is now friendly. Cease fire." Then he turns back to you. "Proceed to %s in the %s system, citizen. I will personally meet you there."]]

   DVtitle[5] = "End of negotiations"
   DVtext[5] = [[Colonel Urnus is visibly annoyed by your response. "Very well then," he bites at you. "In that case you will be destroyed along with the rest of that terrorist scum. Helm, full speed ahead! All batteries, fire at will!"]]

   DVtitle[6] = "A reward for a job well botched"
   DVtext[6] = [[Soon after docking, you are picked up by a couple of soldiers, who escort you to Colonel Urnus' office. Urnus greets you warmly, and offers you a seat and a cigar. You take the former, not the latter.
    "I am most pleased with the outcome of this situation, citizen," Urnus begins. "To be absolutely frank with you, I was beginning to get frustrated. My superiors have been breathing down my neck, demanding results on those blasted FLF, but they are as slippery as eels. Just when you think you've cornered them, poof! They're gone, lost in that nebula. Thick as soup, that thing. I don't know how they can even find their own way home!"]]

   DVtext[7] = [[Urnus takes a puff of his cigar and blows out a ring of smoke. It doesn't take a genius to figure out you're the best thing that's happened to him in a long time.
    "Anyway. I promised you money, status and opportunities, and I intend to make good on those promises. Your money is already in your account. Check your balance sheet later. As for status, I can assure you that no Dvaered will find out what you've been up to. As far as the military machine is concerned, you have nothing to do with the FLF. In fact, you're known as an important ally in the fight against them! Finally, opportunities. We're analyzing the data from your flight recorder as we speak, and you'll be asked a few questions after we're done here. Based on that, we can form a new strategy against the FLF. Unless I miss my guess by a long shot, we'll be moving against them in force very soon, and I will make sure you'll be given the chance to be part of that. I'm sure it'll be worth your while."]]

   DVtext[8] = [[Urnus stands up, a sign that this meeting is drawing to a close. "Keep your eyes open for one of our liaisons, citizen. He'll be your ticket into the upcoming battle. Now, I'm a busy man so I'm going to have to ask you to leave. But I hope we'll meet again, and if you continue to build your career like you have today, I'm sure we will. Good day to you!"
    You leave the Colonel's office. You are then taken to an interrogation room, where Dvaered petty officers question you politely yet persistently about your brief stay with the FLF. Once their curiosity is satisfied, they let you go, and you are free to return to your ship.]]

   flfcomm = {}
   flfcomm[1] = "We have your back, %s!"
   flfcomm[2] = "%s is selling us out! Eliminate the traitor!"
   flfcomm[3] = "Let's get out of here, %s! We'll meet you back at the base."

   misn_title = "FLF: Small Dvaered Patrol in %s"
   misn_desc = "To prove yourself to the FLF, you must take out one of the Dvaered security patrols."
   misn_rwrd = "A chance to make friends with the FLF."
   osd_title   = "Dvaered Patrol"
   osd_desc[1] = "Fly to the %s system"
   osd_desc[2] = "Eliminate the Dvaered patrol"
   osd_desc[3] = "Return to the FLF base"
   osd_desc["__save"] = true
   DVosd[1] = "Fly to the %s system and land on %s"
   DVosd["__save"] = true
    
   npc_name = "FLF petty officer"
   npc_desc = "There is a low-ranking officer of the Frontier Liberation Front sitting at one of the tables. She seems somewhat more receptive than most people in the bar."
end


function create ()
   missys = system.get( "Arcanis" )
   if not misn.claim( missys ) then misn.finish( false ) end

   misn.setNPC( npc_name, "flf/unique/benito" )
   misn.setDesc( npc_desc )
end


function accept ()
   tk.msg( title[1], text[1]:format( player.name() ) )
   if tk.yesno( title[1], text[2] ) then
      tk.msg( title[3], text[3]:format( missys:name() ) )

      osd_desc[1] = osd_desc[1]:format( missys:name() )

      misn.accept()
      misn.osdCreate( osd_title, osd_desc )
      misn.setDesc( misn_desc )
      misn.setTitle( misn_title:format( missys:name() ) )
      marker = misn.markerAdd( missys, "low" )
      misn.setReward( misn_rwrd )

      DVplanet = "Raelid Outpost"
      DVsys = "Raelid"

      reinforcements_arrived = false
      dv_ships_left = 0
      job_done = false

      hook.enter( "enter" )
      hook.jumpout( "leave" )
      hook.land( "leave" )
   else
      tk.msg( refusetitle, refusetext )
      misn.finish( false )
   end
end


function enter ()
   if not job_done then
      if system.cur() == missys then
         misn.osdActive( 2 )
         patrol_spawnDV( 3, nil )
      else
         misn.osdActive( 1 )
      end
   end
end


function leave ()
   if spawner ~= nil then hook.rm( spawner ) end
   if hailer ~= nil then hook.rm( hailer ) end
   if rehailer ~= nil then hook.rm( rehailer ) end
   reinforcements_arrived = false
   dv_ships_left = 0
end


function spawnDVReinforcements ()
   reinforcements_arrived = true
   local dist = 1500
   local x
   local y
   if rnd.rnd() < 0.5 then
      x = dist
   else
      x = -dist
   end
   if rnd.rnd() < 0.5 then
      y = dist
   else
      y = -dist
   end

   local pos = player.pos() + vec2.new( x, y )
   local reinforcements = pilot.add( "Dvaered Big Patrol", "dvaered_norun", pos )
   for i, j in ipairs( reinforcements ) do
      if j:ship():class() == "Destroyer" then boss = j end
      hook.pilot( j, "death", "pilot_death_dv" )
      j:setHostile()
      j:setVisible( true )
      j:setHilight( true )
      fleetDV[ #fleetDV + 1 ] = j
      dv_ships_left = dv_ships_left + 1
   end

   -- Check for defection possibility
   if faction.playerStanding( "Dvaered" ) >= -5 then
      hailer = hook.timer( 30000, "timer_hail" )
   else
      spawner = hook.timer( 30000, "timer_spawnFLF" )
   end
end


function timer_hail ()
   if hailer ~= nil then hook.rm( hailer ) end
   if boss ~= nil and boss:exists() then
      timer_rehail()
      hailer = hook.pilot( boss, "hail", "hail" )
   end
end


function timer_rehail ()
   if rehailer ~= nil then hook.rm( rehailer ) end
   if boss ~= nil and boss:exists() then
      boss:hailPlayer()
      rehailer = hook.timer( 8000, "timer_rehail" )
   end
end


function hail ()
   if hailer ~= nil then hook.rm( hailer ) end
   if rehailer ~= nil then hook.rm( rehailer ) end
   player.commClose()
   tk.msg( DVtitle[1], DVtext[1] )
   tk.msg( DVtitle[1], DVtext[2] )
   choice = tk.choice( DVtitle[1], DVtext[3]:format( player.name() ),
      DVchoice1, DVchoice2 )
   if choice == 1 then
      tk.msg( DVtitle[4], DVtext[4]:format( DVplanet, DVsys ) )

      faction.get("FLF"):setPlayerStanding( -100 )
      local standing = faction.get("Dvaered"):playerStanding()
      if standing < 0 then
         faction.get("Dvaered"):setPlayerStanding( 0 )
      end

      for i, j in ipairs( fleetDV ) do
         if j:exists() then
            j:setFriendly()
            j:changeAI( "dvaered" )
         end
      end

      job_done = true
      osd_desc[1] = DVosd[1]:format( DVsys, DVplanet )
      osd_desc[2] = nil
      misn.osdActive( 1 )
      misn.osdCreate( misn_title, osd_desc )
      misn.markerRm( marker )
      marker = misn.markerAdd( system.get(DVsys), "high" )

      spawner = hook.timer( 3000, "timer_spawnHostileFLF" )
      hook.land( "land_dv" )
   else
      tk.msg( DVtitle[5], DVtext[5] )
      timer_spawnFLF()
   end
end


function spawnFLF ()
   local dist = 1500
   local x
   local y
   if rnd.rnd() < 0.5 then
      x = dist
   else
      x = -dist
   end
   if rnd.rnd() < 0.5 then
      y = dist
   else
      y = -dist
   end

   local pos = player.pos() + vec2.new( x, y )
   fleetFLF = addShips( { "FLF Vendetta", "FLF Lancelot" }, "flf_norun", pos, 8 )
end


function timer_spawnFLF ()
   if boss ~= nil and boss:exists() then
      spawnFLF()
      for i, j in ipairs( fleetFLF ) do
         j:setFriendly()
         j:setVisplayer( true )
      end

      fleetFLF[1]:broadcast( flfcomm[1]:format( player.name() ) )
   end
end


function timer_spawnHostileFLF ()
   spawnFLF()
   for i, j in ipairs( fleetFLF ) do
      j:setHostile()
      j:control()
      j:attack( player.pilot() )
   end

   hook.pilot( player.pilot(), "death", "returnFLFControl" )
   fleetFLF[1]:broadcast( flfcomm[2]:format( player.name() ) )
end


function returnFLFControl()
   for i, j in ipairs( fleetFLF ) do
      j:control( false )
   end
end


function pilot_death_dv ()
   dv_ships_left = dv_ships_left - 1
   if dv_ships_left <= 0 then
      if spawner ~= nil then hook.rm( spawner ) end
      if hailer ~= nil then hook.rm( hailer ) end
      if rehailer ~= nil then hook.rm( rehailer ) end

      job_done = true
      local standing = faction.get("Dvaered"):playerStanding()
      if standing >= 0 then
         faction.get("Dvaered"):setPlayerStanding( -1 )
      end
      misn.osdActive( 3 )
      misn.markerRm( marker )
      marker = misn.markerAdd( system.get( var.peek( "flfbase_sysname" ) ), "high" )
      hook.land( "land_flf" )
      pilot.toggleSpawn( true )
      local hailed = false
      if fleetFLF ~= nil then
         for i, j in ipairs( fleetFLF ) do
            if j:exists() then
               j:control()
               j:hyperspace()
               if not hailed then
                  hailed = true
                  j:comm( player.pilot(), flfcomm[3]:format( player.name() ) )
               end
            end
         end
      end
   elseif dv_ships_left <= 1 and not reinforcements_arrived then
      spawnDVReinforcements()
   end
end


function land_flf ()
   leave()
   if planet.cur():faction():name() == "FLF" then
      tk.msg( title[4], text[4] )
      tk.msg( title[4], text[5]:format( player.name() ) )
      tk.msg( title[4], text[6] )
      tk.msg( title[4], text[7] )
      player.pay( 100000 )
      flf_setReputation( 20 )
      faction.get("FLF"):modPlayer( 10 )
      var.pop( "flfbase_intro" )
      misn.finish( true )
   end
end


function land_dv ()
   leave()
   if planet.cur():name() == DVplanet then
      tk.msg( DVtitle[6], DVtext[6] )
      tk.msg( DVtitle[6], DVtext[7] )
      tk.msg( DVtitle[6], DVtext[8] )
      player.pay( 70000 )
      var.push( "flfbase_intro", 3 )
      if diff.isApplied( "FLF_base" ) then diff.remove( "FLF_base" ) end
      misn.finish( true )
   end
end

