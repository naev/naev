--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Disrupt a Dvaered Patrol">
 <unique />
 <priority>2</priority>
 <chance>100</chance>
 <location>Bar</location>
 <cond>var.peek("flfbase_intro") == 2</cond>
 <spob>Sindbad</spob>
 <notes>
  <done_misn name="Deal with the FLF agent">If you return Gregar to Sindbad</done_misn>
  <provides name="The Dvaered know where Sindbad is">If you betray the FLF</provides>
  <campaign>Join the FLF</campaign>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   This is the second "prelude" mission leading to the FLF campaign.
   stack variable flfbase_intro:
        1 - The player has turned in the FLF agent or rescued the Dvaered crew. Conditional for dv_antiflf02
        2 - The player has rescued the FLF agent. Conditional for flf_pre02
        3 - The player has found the FLF base for the Dvaered, or has betrayed the FLF after rescuing the agent. Conditional for dv_antiflf03

--]]
local dv = require "common.dvaered"
local fleet = require "fleet"
local flf = require "missions.flf.flf_common"
local fmt = require "format"
require "missions.flf.flf_patrol"

-- luacheck: globals patrol_spawnDV fleetDV fleetFLF (inherited from flf_patrol, TODO remove horrible hack)

local boss -- Non-persistent state

mem.osd_desc = {}

mem.osd_desc[2] = _("Eliminate the Dvaered patrol")
mem.osd_desc[3] = _("Return to the FLF base")

function create ()
   mem.missys = system.get( "Arcanis" )
   if not misn.claim( mem.missys ) then misn.finish( false ) end

   misn.setNPC( _("FLF petty officer"), "flf/unique/benito.webp", _("There is a low-ranking officer of the Frontier Liberation Front sitting at one of the tables. She seems somewhat more receptive than most people in the bar.") )
end


function accept ()
   tk.msg( _("A chance to prove yourself"), fmt.f( _([[The FLF officer doesn't seem at all surprised that you approached her. On the contrary, she looks like she expected you to do so all along.
    "Greetings," she says, nodding at you in curt greeting. "I am Corporal Benito. And you must be {player}, the one who got Lt. Fletcher back here in one piece." Benito's expression becomes a little more severe. "I'm not here to exchange pleasantries, however. You probably noticed, but people here are a little uneasy about your presence. They don't know what to make of you, see. You helped us once, it is true, but that doesn't tell us much. We don't know you."]]), {player=player.name()} ) )
   if tk.yesno( _("A chance to prove yourself"), _([[Indeed, you are constantly aware of the furtive glances the other people in this bar are giving you. They don't seem outright hostile, but you can tell that if you don't watch your step and choose your words carefully, things might quickly take a turn for the worse.
    Benito waves her hand to indicate you needn't pay them any heed. "That said, the upper ranks have decided that if you are truly sympathetic to our cause, you will be given an opportunity to prove yourself. Of course, if you'd rather not get involved in our struggle, that's understandable. But if you're in for greater things, if you stand for justice... Perhaps you'll consider joining with us?"]]) ) then
      tk.msg( _("Patrol-B-gone"), fmt.f( _([["I'm happy to hear that. It's good to know we still have the support from the common pilot. Anyway, let me fill you in on what it is we want you to do. As you may be aware, the Dvaered have committed a lot of resources to finding us and flushing us out lately. And while our base is well hidden, those constant patrols are certainly not doing anything to make us feel more secure! I think you can see where this is going. You will go out there and eliminate one of those patrols in the {sys} system."
    You object, asking the Corporal if all recruits have to undertake dangerous missions like this to be accepted into the FLF ranks. Benito chuckles and makes a pacifying gesture.
    "Calm down, it's not as bad as it sounds. You only have to take out one small patrol; I don't think you will have to fight more than 3 ships, 4 if you're really unlucky. If you think that's too much for you, you can abort the mission for now and come to me again later. Otherwise, good luck!"]]), {sys=mem.missys} ) )

      mem.osd_desc[1] = fmt.f( _("Fly to the {sys} system"), {sys=mem.missys} )

      misn.accept()
      misn.osdCreate( _("Dvaered Patrol"), mem.osd_desc )
      misn.setDesc( _("To prove yourself to the FLF, you must take out one of the Dvaered security patrols.") )
      misn.setTitle( fmt.f( _("FLF: Small Dvaered Patrol in {sys}"), {sys=mem.missys} ) )
      mem.marker = misn.markerAdd( mem.missys, "low" )
      misn.setReward( _("A chance to make friends with the FLF.") )

      mem.DVplanet, mem.DVsys = spob.getS("Fort Raelid")

      mem.reinforcements_arrived = false
      mem.dv_ships_left = 0
      mem.job_done = false

      hook.enter( "enter" )
      hook.jumpout( "leave" )
      hook.land( "leave" )
   else
      tk.msg( _("Some other time perhaps"), _([["I see. That's a fair answer, I'm sure you have your reasons. But if you ever change your mind, I'll be around on Sindbad. You won't have trouble finding me, I'm sure."]]) )
      return
   end
end


function enter ()
   if not mem.job_done then
      if system.cur() == mem.missys then
         if mem.osd_desc[2] ~= nil then
            misn.osdActive( 2 )
            patrol_spawnDV( 3, nil )
         end
      else
         misn.osdActive( 1 )
      end
   end
end


function leave ()
   hook.rm( mem.spawner )
   hook.rm( mem.hailer )
   hook.rm( mem.rehailer )
   mem.reinforcements_arrived = false
   mem.dv_ships_left = 0
end


local function spawnDVReinforcements ()
   mem.reinforcements_arrived = true
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
   local dv_big_patrol = { "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Ancestor", "Dvaered Phalanx", "Dvaered Vigilance" }
   local reinforcements = fleet.add( 1, dv_big_patrol, "Dvaered", pos, nil, {ai="dvaered_norun"} )
   for i, j in ipairs( reinforcements ) do
      if j:ship():class() == "Destroyer" then boss = j end
      hook.pilot( j, "death", "pilot_death_dv" )
      j:setHostile()
      j:setVisible( true )
      j:setHilight( true )
      fleetDV[ #fleetDV + 1 ] = j
      mem.dv_ships_left = mem.dv_ships_left + 1
   end

   -- Check for defection possibility
   if faction.playerStanding( "Dvaered" ) >= -5 then
      mem.hailer = hook.timer( 30.0, "timer_hail" )
   else
      mem.spawner = hook.timer( 30.0, "timer_spawnFLF" )
   end
end


function timer_hail ()
   hook.rm( mem.hailer )
   if boss ~= nil and boss:exists() then
      timer_rehail()
      mem.hailer = hook.pilot( boss, "hail", "hail" )
   end
end


function timer_rehail ()
   hook.rm( mem.rehailer )
   if boss ~= nil and boss:exists() then
      boss:hailPlayer()
      mem.rehailer = hook.timer( 8.0, "timer_rehail" )
   end
end


function hail ()
   hook.rm( mem.hailer )
   hook.rm( mem.rehailer )
   player.commClose()
   tk.msg( _("A tempting offer"), _([[Your viewscreen shows a Dvaered Colonel. He looks tense. Normally, a tense Dvaered would be bad news, but then this one bothered to hail you in the heat of battle, so perhaps there is more here than meets the eye.]]) )
   tk.msg( _("A tempting offer"), _([["I am Colonel Urnus of the Dvaered Fleet, anti-terrorism division. I would normally never contact an enemy of House Dvaered, but my intelligence officer has looked through our records and found that you were recently a law-abiding citizen, doing honest freelance missions."]]) )
   local choice = tk.choice( _("A tempting offer"), fmt.f( _([["I know your type, {player}. You take jobs where profit is to be had, and you side with the highest bidder. There are many like you in the galaxy, though admittedly not so many with your talent. That's why I'm willing to make you this offer: you will provide us with information on the FLF's base of operations and their combat strength. In return, I will convince my superiors that you were working for me all along, so you won't face any repercussions for assaulting Dvaered ships. Furthermore, I will transfer a considerable amount of credits in your account, as well as put you into a position to make an ally out of House Dvaered. If you refuse, however, I guarantee you that you will never again be safe in Dvaered space. What say you? Surely this proposition beats anything that rabble can do for you?"]]), {player=player.name()} ),
      _("Accept the offer"), _("Remain loyal to the FLF") )
   if choice == 1 then
      tk.msg( _("Opportunism is an art"), fmt.f( _([[Colonel Urnus smiles broadly. "I knew you'd make the right choice, citizen!" He addresses someone on his bridge, out of the view of the camera. "Notify the flight group. This ship is now friendly. Cease fire." Then he turns back to you. "Proceed to {pnt} in the {sys} system, citizen. I will personally meet you there."]]), {pnt=mem.DVplanet, sys=mem.DVsys} ) )

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

      mem.job_done = true
      mem.osd_desc[1] = fmt.f( _("Fly to the {sys} system and land on {pnt}"), {sys=mem.DVsys, pnt=mem.DVplanet} )
      mem.osd_desc[2] = nil
      misn.osdActive( 1 )
      misn.osdCreate( _("Dvaered Patrol"), mem.osd_desc )
      misn.markerRm( mem.marker )
      mem.marker = misn.markerAdd( mem.DVplanet, "high" )

      mem.spawner = hook.timer( 3.0, "timer_spawnHostileFLF" )
      hook.land( "land_dv" )
   else
      tk.msg( _("End of negotiations"), _([[Colonel Urnus is visibly annoyed by your response. "Very well then," he bites at you. "In that case you will be destroyed along with the rest of that terrorist scum. Helm, full speed ahead! All batteries, fire at will!"]]) )
      timer_spawnFLF()
   end
end


local function spawnFLF ()
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
   fleetFLF = fleet.add( 8, {"Vendetta", "Lancelot"}, "FLF", pos, nil, {ai="flf_norun"} )
end


function timer_spawnFLF ()
   if boss ~= nil and boss:exists() then
      spawnFLF()
      for i, j in ipairs( fleetFLF ) do
         j:setFriendly()
         j:setVisplayer( true )
      end

      fleetFLF[1]:broadcast( fmt.f( _("We have your back, {player}!"), {player=player.name()} ) )
   end
end


function timer_spawnHostileFLF ()
   spawnFLF()
   local pp = player.pilot()
   for i, j in ipairs( fleetFLF ) do
      j:changeAI( "baddiepos" )
      j:setHostile()
      j:memory().guardpos = pp:pos()
   end

   fleetFLF[1]:broadcast( fmt.f( _("{player} is selling us out! Eliminate the traitor!"), {player=player.name()} ) )
end


function pilot_death_dv ()
   mem.dv_ships_left = mem.dv_ships_left - 1
   if mem.dv_ships_left <= 0 then
      hook.rm( mem.spawner )
      hook.rm( mem.hailer )
      hook.rm( mem.rehailer )

      mem.job_done = true
      local standing = faction.get("Dvaered"):playerStanding()
      if standing > -20 then
         faction.get("Dvaered"):setPlayerStanding( -20 )
      end
      misn.osdActive( 3 )
      misn.markerRm( mem.marker )
      mem.marker = misn.markerAdd( system.get( var.peek( "flfbase_sysname" ) ), "high" )
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
                  j:comm(
                     player.pilot(),
                     fmt.f( _("Let's get out of here, {player}! We'll meet you back at the base."), {player=player.name()} )
                  )
               end
            end
         end
      end
   elseif mem.dv_ships_left <= 1 and not mem.reinforcements_arrived then
      spawnDVReinforcements()
   end
end


function land_flf ()
   leave()
   if spob.cur():faction() == faction.get("FLF") then
      tk.msg( _("Breaking the ice"), _([[When you left Sindbad Station, it was a cold, lonely place for you. The FLF soldiers on the station avoided you whenever they could, and basic services were harder to get than they should have been.
    But now that you have returned victorious over the Dvaered, the place has become considerably more hospitable. There are more smiles on people's faces, and some even tell you you did a fine job. Among them is Corporal Benito. She walks up to you and offers you her hand.]]) )
      tk.msg( _("Breaking the ice"), fmt.f( _([["Welcome back, {player}, and congratulations. I didn't expect the Dvaered to send reinforcements, much less a Vigilance. I certainly wouldn't have sent you alone if I did, and I might not have sent you at all. But then, you're still in one piece, so maybe I shouldn't worry so much, eh?"]]), {player=player.name()} ) )
      tk.msg( _("Breaking the ice"), _([[Benito takes you to the station's bar and buys you what, for lack of a better word, must be called a drink.
    "We will of course reward you for your service," she says once you are seated. "Though you must understand the FLF doesn't have that big a budget. Financial support is tricky, and the Frontier doesn't have that much to spare themselves to begin with. Nevertheless, we are willing to pay for good work, and your work is nothing but. What's more, you've ingratiated yourself with many of us, as you've undoubtedly noticed. Our top brass are among those you've impressed, so from today on, you can call yourself one of us! How about that, huh?"]]) )
      tk.msg( _("Breaking the ice"), _([["Of course, our work is only just beginning. No rest for the weary; we must continue the fight against the oppressors. I'm sure the road is still long, but I'm encouraged by the fact that we gained another valuable ally today. Check the mission computer for more tasks you can help us with. Please take this Pentagram of Valor as a token of appreciation. I'm sure you'll play an important role in our eventual victory over the Dvaered!"
    That last part earns a cheer from the assembled FLF soldiers. You decide to raise your glass with them, making a toast to the fortune of battle in the upcoming campaign - and the sweet victory that lies beyond.]]) )
      player.pay( 100e3 )
      player.outfitAdd('Pentagram of Valor')
      flf.setReputation( 10 )
      faction.get("FLF"):modPlayer( 1 )
      var.pop( "flfbase_intro" )
      flf.addLog( _([[You earned the complete trust of the FLF by eliminating a Dvaered patrol and then refusing to change sides when the Dvaereds pressured you to. You can now consider yourself to be one of the FLF.]]) )
      misn.finish( true )
   end
end


function land_dv ()
   leave()
   if spob.cur() == mem.DVplanet then
      tk.msg( _("A reward for a job well botched"), _([[Soon after docking, you are picked up by a couple of soldiers, who escort you to Colonel Urnus's office. Urnus greets you warmly, and offers you a seat and a cigar. You take the former, not the latter.
    "I am most pleased with the outcome of this situation, citizen," Urnus begins. "To be absolutely frank with you, I was beginning to get frustrated. My superiors have been breathing down my neck, demanding results on those blasted FLF, but they are as slippery as eels. Just when you think you've cornered them, poof! They're gone, lost in that nebula. Thick as soup, that thing. I don't know how they can even find their own way home!"]]) )
      tk.msg( _("A reward for a job well botched"), _([[Urnus takes a puff of his cigar and blows out a ring of smoke. It doesn't take a genius to figure out you're the best thing that's happened to him in a long time.
    "Anyway. I promised you money, status, and opportunities, and I intend to make good on those promises. Your money is already in your account. Check your balance later. As for status, I can assure you that no Dvaered will find out what you've been up to. As far as the military machine is concerned, you have nothing to do with the FLF. In fact, you're known as an important ally in the fight against them! Finally, opportunities. We're analyzing the data from your flight recorder as we speak, and you'll be asked a few questions after we're done here. Based on that, we can form a new strategy against the FLF. Unless I miss my guess by a long shot, we'll be moving against them in force very soon, and I will make sure you'll be given the chance to be part of that. I'm sure it'll be worth your while."]]) )
      tk.msg( _("A reward for a job well botched"), _([[Urnus stands up, a sign that this meeting is drawing to a close. "Keep your eyes open for one of our liaisons, citizen. He'll be your ticket into the upcoming battle. Now, I'm a busy man so I'm going to have to ask you to leave. But I hope we'll meet again, and if you continue to build your career like you have today, I'm sure we will. Good day to you!"
    You leave the Colonel's office. You are then taken to an interrogation room, where Dvaered petty officers question you politely yet persistently, about your brief stay with the FLF. Once their curiosity is satisfied, they let you go, and you are free to return to your ship.]]) )
      player.pay( 70e3 )
      var.push( "flfbase_intro", 3 )
      if diff.isApplied( "FLF_base" ) then diff.remove( "FLF_base" ) end
      dv.addAntiFLFLog( _([[As you were conducting a mission to earn the trust of the FLF, Dvaered Colonel Urnus offered you a deal: you could betray the FLF and provide information on the location of the hidden FLF base in exchange for a monetary reward and immunity against any punishment. You accepted the deal, leading to an enraged wing of FLF pilots attacking you in retaliation. The FLF terrorists were repelled, however, and Urnus told you to keep an eye out for one of the Dvaered liaisons so you can join the Dvaered in the upcoming mission to destroy Sindbad.]]) )
      misn.finish( true )
   end
end
