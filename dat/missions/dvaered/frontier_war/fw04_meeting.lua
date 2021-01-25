--[[
<?xml version='1.0' encoding='utf8'?>
 <mission name="Dvaered Meeting">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>2</priority>
   <chance>25</chance>
   <done>Dvaered Diplomacy</done>
   <location>Bar</location>
   <faction>Dvaered</faction>
  </avail>
  <notes>
   <campaign>Frontier Invasion</campaign>
  </notes>
 </mission>
 --]]
--[[
-- Dvaered Meeting
-- This is the 5th mission of the Frontier War Dvaered campaign.
-- The player has to secure a Dvaered Warlord Meeting.
-- Hamelsen lures out the player, while Strafer gets killed by a spy who runs away.
-- There are two parts in the mission : part 1 is mainly contemplative, and part 2 is tricky, with fights and so.

   Stages :
   0) Way to Dvaer Prime
   1) Briefing at Dvaer Prime and patrol around DHC
   2) Following Hamelsen towards Laars (main purpose of this is to let the player save the game because second part is tricky)
   3) Taking off from Laars
   4) Land on Laars and get rewarded
--]]

require "missions/dvaered/frontier_war/fw_common"
require "proximity"
require "portrait"
require "numstring"

-- TODO: hooks to penalize attacking people

npc_name = _("Lieutenant Strafer")
npc_desc1 = _("Judging by how he looks at you, Strafer needs you for another mission along with the Dvaered Space Force.")
npc_desc2 = _("Harsh voice, frank gaze and easy trigger. The lieutenant Strafer is a Dvaered pilot.")

propose_title = _("We need you once more")
propose_text = _([[As you sit at his table, the clearly anxious Dvaered pilot stops biting his nails and explains why he is here.
   "The High Command summoned, under request of general Klank, an extraordinary meeting of the high council of Warlords, and all of them have agreed to come..." You frown, and before you have a chance to ask where is the problem, he continues: "... but we received an intelligence report according to which the ex-colonel Hamelsen (who has already tried to murder Major Tam several times) is going to take advantage of this meeting to take action against us."
   "Do you want to help us against that threat?"]])

accept_title = _("Here is the situation")
accept_text = _([["General Klank has summoned the Warlords in order to present them the Frontier invasion plan. When a meeting of the high council of warlords occurs, a short truce takes place and they all come on the DHC station with their Goddards. This fact alone is already enough to put the station's security service under pressure, as the warlords constantly provoke each other and start brawls. But this time, we believe that Hamelsen will try either to assassinate warlords, or to record our invasion plan in order to sell it to hostile foreign powers.
   "This is why Major Tam wants our squadron from the Special Operations Forces to support the regular units of the station. Fly to Dvaer Prime and meet me in the bar there."]])

refuse_title = _("Too bad")
refuse_text = _([[Mm. I see. you probably have many much more interesting things to do that being loyal to the Dvaered Nation...]])


lore_title = _("Lieutenant Strafer")
lore_text0 = _("What do you want to ask to the lieutenant before taking off?")

lore_already_told1 = _("Hem... You asked the exact same question just before.")
lore_already_told2 = _("You are obsessed with this question. Just move on, please.")
lore_already_told3 = _("Is it a kind of joke?")

quitstraf = _("Take off when you're ready for action!")
question = {}
lore_text = {}

question[1] = _("Ask for a briefing")
lore_text[1] = _([["Both squadrons of the DHC station's space security force will be deployed with full range ships from Vendettas to Goddards. Those squadrons are the 'Beta-Storks' and the 'Beta-Hammer' and their missions will be to control medium and heavy ships and to provide anti-heavy firepower in case of need. Our squadron, named 'Alpha-NightClaws', is in charge of fast ships (Yachts and Fighters). We will be flying Hyenas.
   "The procedure is the following: any ship approaching the station will be assigned to a squad by the fleet leader, and then to a pilot by the squad leader (Captain Leblanc). When a ship is attributed to you, you will have to approach the ship within 1000 m. Their security clearance code will be automatically requested and processed by the system we install right now on your core unit. Afterwards, the ship will be allowed to land, or ordered to fly away. The same thing happens for ships that leave the station.
   "Finally, in case something unexpected happens, you will of course have to obey to orders. Watch your messages closely. A few pilots will be kept in reserve close to the station.
   "Oh, and there is another point I must warn you about: it's the warlord's humour. When they see a small ship close to their Goddard, they may get the idea to shoot a small railgun-volley in your direction. Some of them tend to enjoy seeing pilots dodge for their lives. Dvaered laws authorize warlords to do so provided they can assure the High Command that there was no hostile intention. That can be a bit annoying, sometimes."]])

question[2] = _("Ask about Colonel Hamelsen")
lore_text[2] = _([[When you mention the colonel Hamelsen, Strafer cuts you: "Ex-colonel Hamelsen! She is a traitor and has lost all her commendations. Now, she is nothing to the Dvaered, and things are better like that." You ask him if things may have turned differently for her and he answers:
   "Watch out, citizen: this kind of question leads to compassion. Compassion leads to weakness and weakness leads to death. Death for yourself and for those who trusted you. Death for your leaders and for your subordinates. Death for people you love and people you respect. Remember: if you want to be strong, don't listen to compassion. Don't even give compassion any chance to reach your heart."]])

question[3] = _("Ask why you were hired for this mission")
lore_text[3] = _([[You ask Strafer why Major Tam requested you to be part of the mission. "Actually, we did not need a private pilot. I just managed to convince Captain Leblanc to hire you." As you wonder why he did that, Strafer thinks a bit and smiles: "Well, I get the feeling we are doing good job, together. Aren't we?"]])

question[4] = _("Ask how one becomes a warlord")
lore_text[4] = _([["You wish to become one of them?" Before you have a chance to deny, he continues: "Anybody can become a warlord. One just has to have received the '9th grade commendation', and to conquer a planet (or a station). In the army, every rank gives you a commendation grade, for example, I have the 3rd grade. Civilians also obtain commendation for their high deeds; you obtained the first grade commendation for your involvement in the FLF destruction, if I am right. The 9th grade commendation, that is associated to the rank of first class General in the army, gives the right to own a military base, and by extension, to be granted the regal power over a region.
   "In the Dvaered army, everybody starts as a raw soldier, no matter if you're an infantryman, a pilot, a medic or even a General's child. And only Valor decides how quick you rise in the hierarchy. Warlord is the ultimate rank for the military (and private combat pilots, like yourself)"]])

quit = _("I am ready for action!")

briefing_title = _("Read your messages")
briefing_text = _("Don't forget to read the messages Captain Leblanc will send to you. It contains valuable information.")

briefing0 = _("I hope everyone is listening carefully.")
briefing1 = _("Every incoming pilot must be visually controlled by one of us.")
briefing2 = _("I will notify each of you when you have a pilot to control.")
briefing3 = _("And you will hear the following audio signal.")
briefing4 = _("Last thing: I remind you to beware the Warlords: some of them may want to shoot at you because they love to watch lighter ships dodge for their lives.")


closer_title = _("An unidentified ship came close to the station")
closer_text = _("A ship managed to approach the station, and you failed to control it. Fortunately, it was identified by the station's sensors and is not hostile. However, your failure to intercept it could have led to problems in the opposite case. As a consequence, your reward has been decreased.")

lander_title = _("An unidentified ship landed on the station")
lander_text = _("A ship managed to land on the station, and you failed to control it. Unidentified and potentially hostile individuals have entered the Dvaered High Command station: The mission is a failure.")

noanswer_title = _("Incoming ship refuses control")
noanswer_text = _([[When getting in range, you receive an alarm. This ship does not have an invitation. Suddenly, you see it accelerating as if the pilot wanted to force the blockade around the station. You hear an order from Captain Leblanc: "A-NightClaws Leader to %s: intercept and destroy %s".]])

hamtitle1 = _("Hi there!")
hamtext1 = _([[The fleeing ship suddenly hails you. You answer and the face of the Colonel Hamelsen emerges from your holoscreen. "No, you won't best me, %s. Not this time. Not anymore." Aware that she is now too far away for you to hope to catch her, you ask her what interest she finds in constantly harassing Major Tam. "This is all that I have left," she answers.
   "My hate for Tam and Klank is all that remains now that my Lord is dead. I dedicated my entire life to the greatness of House Dvaered, I practiced and sharpened all my skills to serve the Army at best. When I got recruited by Lord Battleaddict, I became faithful to him because he was giving me the opportunity to serve House Dvaered through him. And then...
   "Since the day when Klank assassinated my Lord, I have been rejected by the High Command. Rejected by the Warlords. Rejected by that nation that claims to reward Valor and Righteousness. Tell me when I have given up Valor! Tell me when I have given up Righteousness! Never! The Dvaered social contract is broken as far as I am concerned.
   "All that remains of me is a vassal without suzerain, a colonel without regiment, a corpse without grave. I will haunt you until your demise. I will be in the way of any of your wishes, big ones as well as small ones. There will be no forgiveness, no remission, no relief, neither for you nor for me."
   After this very encouraging speech, Hamelsen cuts off the communication channel and lands.]])

hamtitle2 = _("Follow her!")
hamtext2 = _([[A new message comes from Captain Leblanc. "This is obviously a diversion! Everyone, back to your positions! %s, go and investigate on %s. Bring me the head of the ex-colonel Hamelsen!"]])


dv_name = _("Officer")
dv_desc = _("This Dvaered officer is probably waiting for you and might have information about the ex-colonel Hamelsen's escape.")

offtitle = _("No trace of Hamelsen")
offtext = _([[You approach the soldier, who seems to recognize you. Probably your portrait has been transmitted by Captain Leblanc. "After we received the message from your captain, we seized the Hyena you were pursuing, but the pilot has managed to escape unnoticed."
   Having no way to track Hamelsen on land, you decide it's better to take off again, and to wait for further instructions from Leblanc.]])

spytitle = _("Something is happening at the station")
spytext = _([[You start heading to the station, but you hear many messages coming from the NightClaws squadron. A Schroedinger has managed to take off unnoticed from the High Command station, presumably carrying classified information, and to sneak through the blockade. The squadrons have been taken by surprise, but Strafer is catching up.]])

spychatter0 = _("A-NightClaws Leader to all pilots: Engage and destroy this Schroedinger at all costs!")
spychatter1 = _("A-NightClaws Second to Leader: I'm on it!")
spychatter2 = _("Bye, suckers!")
spychatter3 = _("Just wait for my shredders to...")
spychatter4 = _("Come on!")

killertitle = _("Now, it's your turn")
killertext = _([[Your sensors suddenly pick up three hostile signals coming from Laars, just when Captain Leblanc sends her message: "To all A-NightClaw pilots: follow and intercept the hostiles." You report about the three hostiles coming at you and she answers: "%s, take care of those three. Don't let any of them escape."]])

killerchatter0 = _("You fell in our trap, %s!")
killerchatter1 = _("You're so dead!")
killerchatter2 = _("Without any support!")
killerchatter3 = _("Folks, please do less chatting and more killing.")
killerchatter4 = _("Aye-aye, boss!")
killerchatter5 = _("Copy that!")
killerchatter6 = _("I'm as silent as a carp.")
killerchatter7 = _("Same for me!")

kiltitle = _("Rid of them!")
kiltext = _("As the last enemy ship explodes, you watch to your sensorscreen, and notice that the alpha squadron has left the system. The fleet leader orders you to land on Dvaer Prime.")


debrief_title = _("Time for a gorgeous reward?")
debrief_text1 = _([[At the very moment you step out of your ship, you see an officer, alone on the dock, obviously waiting for you. After having moved forward, you recognize the Major Tam. The cold wind unfolds the lapels of his coat, and make it whip his sad face.
   "We had better days, isn't it, citizen? A spy managed to run away with what seems to be a copy of our invasion plan, they killed one of my best pilots and Hamelsen escaped... Once more." Tam looks at the sky that starts to rain "... and it's winter on the spacedock of Dvaer Prime. Shall we enter the building? I was told that the chemical plant works twice more in winter, and the rain often turns acid."
   You enter and head to the military bar. Tam looks at you: "I've grown up on Nanek in Allous. For 13 years, the only part of the universe I used to know was my village on Nanek, and the only people I used to know were its inhabitants. And now, I've seen hundreds of planets, and thousands of people all around the galaxy. But most of them have been killed at some point, and they corpses are drifting here and there in space, along with the pitiful remains of their defeated ships. The night sky is constellated with the souls of dead pilots. Our control of space gave us access to experiences our forefathers could not even dream of, but you know what? No matter how cold the graves of my ancestors on Nanek are, they are warmer than the emptiness of infinite space."]])
debrief_text2 = _([[You start wondering if the major will remember to pay you, but his voice suddenly changes: "We definitely had better days, but you know, the true valor of a warrior reveals itself in times of adversity. The dark clouds that drift above the horizon, pushed by the cruel wings of despair, are here to challenge the strength of our Dvaered souls. And it is up to us to accept this challenge.
   "I did not anticipate that the traitor Hamelsen could reconstruct her group of mercenaries so fast, but you already killed some of them, and Leblanc's squadron will kill even more in the near future. We will then hunt the ex-colonel Hamelsen down, and finally we will carry this invasion on.
   "Anyway, for now, we will transfer %s to your account, as a reward for this mission, and be certain that we will need you again soon!"]])


lords_chatter = { _("Ahoy, suckers! Here comes the master!"),
                  _("Look down, you weaklings."),
                  _("Only submission will save you from my anger!"),
                  _("Kneel, for I am destined to rule you all!"),
                  _("Worship my strength, or burn by my railguns."),
                  _("Here comes Daddy!"),
                  _("I am an artist of Pain and Destruction. Who wants to be part of my next artwork?"),
                  _("Make way for the supplier of Hell!"),
                  _("Death is the destination of everyone's trip. Pissing me off means taking a shortcut."), }

order_control = _("A-NightClaws Leader to %s: intercept %s and control their security clearance code")

flee_title = _("Look who is running away!")
flee_text = _("You were supposed to secure a meeting, not to run away like that!")


control_title = _("Controlling incoming ship")
-- This might be hard to translate, but that is not a problem IMHO as it is actually supposed to be an automatically generated message.
control_text = _([[As you approach the ship, your targeting array focuses on it, and its clearance code gets processed. You can soon read on your control pad: "This citizen is an honorable %s whose presence is required for the meeting: let the ship land on the station."]])

occupations = { _("caterer"),
                _("butcher"),
                _("brewer"),
                _("waiter"),
                _("vigil"),
                _("gladiator"),
                _("rat exterminator"),
                _("emergency doctor"),
                _("weapons dealer"),
                _("drug dealer"),
                _("torturer"), }


misn_desc = _("You are part of the space security service for an extraordinary meeting of the high council of Warlords, where the invasion plan of the frontier will be discussed.")
misn_reward = _("The greatness of House Dvaered.")
log_text = _("You have taken part to the security of a meeting of Dvaered warlords which goal was to set the invasion plan of the Frontier. Unfortunately, the ex-colonel Hamelsen and her wingmen managed to seize data and escape the system. The Lieutenant Strafer has been killed in action during this incident.")

osd_title = _("The Meeting")
osd_text1 = _("Land on %s")
osd_text2 = _("Stay close to %s and wait for orders")
osd_text3 = _("Intercept suspect ship for visual identification")
osd_text4 = _("Engage %s")
osd_text5 = _("Eliminate the hostile fighters")


function create()
   destpla, destsys = planet.get("Dvaer Prime")
   targpla = planet.get("Dvaered High Command")
   haltpla = planet.get("Laarss")

   if planet.cur() == destpla then
      misn.finish(false)
   end

   if not misn.claim(destsys) then
       misn.finish(false)
   end

   misn.setNPC(npc_name, portrait_strafer)
   misn.setDesc(npc_desc1)
end

function accept()
   if not tk.yesno( propose_title, propose_text ) then
      tk.msg(refuse_title, refuse_text)
      misn.finish(false)
   end
   tk.msg(accept_title, accept_text)

   misn.accept()
   misn.osdCreate( osd_title, {osd_text1:format(destpla:name())} )
   misn.setDesc(misn_desc)
   misn.setReward(misn_reward)
   misn.markerAdd(destsys, "low")

   stage = 0
   hook.land("land")
   loadhook = hook.load("loading")
   reward = credits_04
end

function loading()
   if (stage == 1 and planet.cur() == destpla) then
      strNpc() -- Put Strafer back at loading TODO: test whether it works
   end
end

function land()
   if (stage == 0 and planet.cur() == destpla) then
      strNpc()
      takhook = hook.takeoff("takeoff")
      stage = 1
      misn.osdDestroy()
      misn.osdCreate( osd_title, {osd_text2:format(targpla:name()), osd_text3 } )

   -- Player is running away
   elseif (stage == 1 or stage == 3 or (stage == 2 and (not planet.cur() == haltpla) )) then
      flee()

   -- Landing on Laars after Hamelsen
   elseif stage == 2 then
      npc = misn.npcAdd("discussOff", dv_name, getMilPortrait( "Dvaered" ), dv_desc)
      stage = 3

   -- Player killed attackers, and can finally land for reward
   elseif (stage == 4 and planet.cur() == destpla) then
      tk.msg( debrief_title, debrief_text1, ("portraits/"..portrait_tam) )
      tk.msg( debrief_title, debrief_text2:format(creditstring(reward)), ("portraits/"..portrait_tam) )
      player.pay(reward)

      shiplog.createLog( "frontier_war", _("Frontier War"), _("Dvaered") )
      shiplog.appendLog( "frontier_war", log_text )
      misn.finish(true)
   end
end

-- Encounter with Strafer on Dvaer Prime
function strNpc()
   toldya = {0,0,0,0}
   strafer = misn.npcAdd("discussStr", npc_name, portrait_strafer, npc_desc2)
end

function takeoff()
   -- Taking off from Dvaer Prime for first part of mission
   if stage == 1 then
      --hook.rm(takhook)
      hook.jumpout("flee")

      pilot.toggleSpawn(false)
      pilot.clear()
      targpos = targpla:pos()

      spawnBeta()
      spawnAlpha()
      scheduleIncoming()

   -- Taking off from Laars for the death of Strafer and the attack from henchmen
   elseif stage == 3 then
      pilot.toggleSpawn(false)
      pilot.clear()
      targpos = targpla:pos()

      spawnBeta()
      spawnAlpha()
      StraferNspy()

      -- Chatter
      hook.timer( 700, "message", {pilot = alpha[1], msg = spychatter0} )
      hook.timer( 1400, "message", {pilot = alpha[2], msg = spychatter1} )
      hook.timer( 2100, "message", {pilot = spy, msg = spychatter2} )
      hook.timer( 2800, "message", {pilot = alpha[2], msg = spychatter3} )
      hook.timer( 5000, "message", {pilot = alpha[2], msg = spychatter4} )

      hook.timer( 4000, "deathOfStrafer" )

      misn.osdDestroy()
      misn.osdCreate( osd_title, {osd_text2:format(targpla:name()), osd_text3 } )
   end
end

-- Player flees from the system
function flee()
   tk.msg(flee_title, flee_text)
   misn.finish(false)
end

-- Player discusses with Lieutenant Strafer
function discussStr()
   local c = tk.choice( lore_title, lore_text0, question[1], question[2], question[3], question[4], quit )
   if c <= 4 then
      if toldya[c] >= 3 then -- Strafer gets annoyed if one asks several times the same question
         tk.msg( lore_title, lore_already_told3 )
         toldya[c] = 0
      elseif toldya[c] == 2 then
         tk.msg( lore_title, lore_already_told2 )
         toldya[c] = 3
      elseif toldya[c] == 1 then
         tk.msg( lore_title, lore_already_told1 )
         toldya[c] = 2
      else
         tk.msg( lore_title, lore_text[c] )
         toldya[c] = 1
      end
   end
end

-- Spawn the Beta squadrons
function spawnBeta()
   beta = {}
   beta[1] = pilot.addFleet( "Dvaered Vendetta", targpla )[1]
   beta[1]:rename(_("B-Storks-4"))
   beta[2] = pilot.addFleet( "Dvaered Vendetta", targpla )[1]
   beta[2]:rename(_("B-Storks-3"))
   beta[3] = pilot.addFleet( "Dvaered Vendetta", targpla )[1]
   beta[3]:rename(_("B-Storks-2"))
   beta[4] = pilot.addFleet( "Dvaered Vendetta", targpla )[1]
   beta[4]:rename(_("B-Storks-Lead"))
   beta[5] = pilot.addFleet( "Dvaered Ancestor", targpla )[1]
   beta[5]:rename(_("B-Storks-7"))
   beta[6] = pilot.addFleet( "Dvaered Ancestor", targpla )[1]
   beta[6]:rename(_("B-Storks-6"))
   beta[7] = pilot.addFleet( "Dvaered Ancestor", targpla )[1]
   beta[7]:rename(_("B-Storks-5"))
   beta[8] = pilot.addFleet( "Dvaered Phalanx", targpla )[1]
   beta[8]:rename(_("B-Hammer-4"))
   beta[9] = pilot.addFleet( "Dvaered Phalanx", targpla )[1]
   beta[9]:rename(_("B-Hammer-4"))
   beta[10] = pilot.addFleet( "Dvaered Phalanx", targpla )[1]
   beta[10]:rename(_("B-Hammer-3"))
   beta[11] = pilot.addFleet( "Dvaered Vigilance", targpla )[1]
   beta[11]:rename(_("B-Hammer-2"))
   beta[12] = pilot.addFleet( "Dvaered Goddard", targpla )[1]
   beta[12]:rename(_("B-Hammer-Lead"))

   Bidlehooks = {}
   for i, p in ipairs(beta) do
      p:control()
      imDoingNothing( p )
      Bidlehooks[i] = hook.pilot( p, "idle", "imDoingNothing", p )
      p:setVisible()
   end
end

-- Spawn the alpha squadron
function spawnAlpha()
   alpha = {}
   alpha[1] = pilot.add( "Hyena", "Dvaered", targpla, _("Captain Leblanc"), "baddie" )
   alpha[2] = pilot.add( "Hyena", "Dvaered", destpla, _("Lieutenant Strafer"), "baddie" )
   alpha[3] = pilot.add( "Hyena", "Dvaered", targpla, _("A-NightClaws-3"), "baddie" )
   alpha[4] = pilot.add( "Hyena", "Dvaered", targpla, _("A-NightClaws-4"), "baddie" )
   alpha[5] = pilot.add( "Hyena", "Dvaered", destpla, _("A-NightClaws-5"), "baddie" )

   Aidlehooks = {}
   for i, p in ipairs(alpha) do
      p:control()
      imDoingNothing( p )
      Aidlehooks[i] = hook.pilot( p, "idle", "imDoingNothing", p )
      p:setVisible()
   end
end

-- Schedule the appearance of incoming ships
function scheduleIncoming()
   -- TODO: some other civilian ships should arrive (for decoration)

   -- First the Warlords
   noWrlrd = 1
   wrlrds = {}

   spawnWrlrd()
   hook.timer( 50000, "spawnWrlrd" )
   hook.timer( 100000, "spawnWrlrd" )
   hook.timer( 150000, "spawnWrlrd" )
   hook.timer( 200000, "spawnWrlrd" )

   -- Then annoying people the player has to control
   controls = { nil, nil, nil, nil, nil }
   canland = { false, false, false, false, false } -- Marks wether the ship has been controlled by the player
   noCtrl = 1
   spawnControl()
   hook.timer( 40000, "spawnControl" )
   hook.timer( 80000, "spawnControl" )
   hook.timer( 120000, "spawnControl" )
   hook.timer( 160000, "spawnControl" )
   n2control = 0 -- This stores the number of ships the play has to control

   hook.timer( 200000, "spawnHam" ) -- Hamelsen comes in

   -- Briefing by Leblanc.
   tk.msg( briefing_title, briefing_text )
   hook.timer( 4000, "message", {pilot = alpha[1], msg = briefing0} )
   hook.timer( 8000, "message", {pilot = alpha[1], msg = briefing1} )
   hook.timer( 12000, "message", {pilot = alpha[1], msg = briefing2} )
   hook.timer( 16000, "message", {pilot = alpha[1], msg = briefing3} )
   hook.timer( 17000, "beepMe" )
   hook.timer( 20000, "message", {pilot = alpha[1], msg = briefing4} )
end

-- Play a beep
function beepMe()
   audio.soundPlay( "jump" )
end

-- Spawn Warlords, who will annoy the player
function spawnWrlrd()
   spawn1Wrlrd( system.get("Gremlin") )
   noWrlrd = noWrlrd + 1
   spawn1Wrlrd( system.get("Radix") )
   noWrlrd = noWrlrd + 1
   spawn1Wrlrd( system.get("Urbanus") )
   noWrlrd = noWrlrd + 1
end

-- Spawn one warlord
function spawn1Wrlrd( origin )
   wrlrds[noWrlrd] = pilot.add( "Dvaered Goddard", "Warlords", origin, lords[noWrlrd] )
   wrlrds[noWrlrd]:control()

   -- Decide if the Warlord will play ar shooting at the player
   shootOnMe = false
   if rnd.rnd() <= .3 then
      shootOnMe = true
   end
   wrlrds[noWrlrd]:land( targpla, shootOnMe )

   hook.pilot( wrlrds[noWrlrd], "land", "warlordTaunt", wrlrds[noWrlrd] )
end

-- Broadcast a stupid taunt upon arriving
function warlordTaunt( self )
   ind = rnd.rnd( 1, #lords_chatter )
   self:broadcast( lords_chatter[ind] )
end

-- Spawn a pilot the player has to control
function spawnControl()
   syst = {"Gremlin", "Radix", "Urbanus"}
   theSys = syst[ rnd.rnd(1,#syst) ]
   origin = system.get(theSys)

   types = { "Civilian Gawain", "Civilian Llama", "Civilian Schroedinger", "Civilian Hyena" }
   theTyp = types[ rnd.rnd(1,#types) ]

   controls[noCtrl] = pilot.addFleet( theTyp, origin )[1]
   controls[noCtrl]:control()
   controls[noCtrl]:land(targpla)

   hook.timer(500, "proximity", {location = targpos, radius = 10000, funcname = ("incomingControl"..tostring(noCtrl)), focus = controls[noCtrl]}) -- First one for detection
   hook.timer(500, "proximity", {location = targpos, radius = 1500, funcname = ("toocloseControl"..tostring(noCtrl)), focus = controls[noCtrl]}) -- Second one for loosing

   noCtrl = noCtrl + 1
end

-- Some patrol pilot is idle.
function imDoingNothing( self )
   rad = rnd.rnd() * 1000 + 1000
   ang = rnd.rnd() * 2 * math.pi
   self:moveto( targpos + vec2.new(math.cos(ang) * rad, math.sin(ang) * rad) )
end

-- A controlled ship is too close from station
function toocloseControl( ind )
   if (not canland[ind]) then -- Player failed to control a ship: penalty
      tk.msg( closer_title, closer_text )
      reward = reward - 10000
   end
end
function toocloseControl1()
   toocloseControl(1)
end
function toocloseControl2()
   toocloseControl(2)
end
function toocloseControl3()
   toocloseControl(3)
end
function toocloseControl4()
   toocloseControl(4)
end
function toocloseControl5()
   toocloseControl(5)
end

-- A ship approaches from DHC: assign it to player
function incomingControl( self )
   audio.soundPlay( "jump" )
   alpha[1]:comm( order_control:format( player.name(), self:name() ) )
   self:setHilight()
   self:setVisible()
   n2control = n2control + 1
   misn.osdActive(2)
end
function incomingControl1()
   incomingControl( controls[1] )
   hook.timer(500, "proximity", {anchor = controls[1], radius = 1000, funcname = ("checkClearance1")}) -- Just because I cannot pass an argument to proximity hooks :(
end
function incomingControl2()
   incomingControl( controls[2] )
   hook.timer(500, "proximity", {anchor = controls[2], radius = 1000, funcname = ("checkClearance2")})
end
function incomingControl3()
   incomingControl( controls[3] )
   hook.timer(500, "proximity", {anchor = controls[3], radius = 1000, funcname = ("checkClearance3")})
end
function incomingControl4()
   incomingControl( controls[4] )
   hook.timer(500, "proximity", {anchor = controls[4], radius = 1000, funcname = ("checkClearance4")})
end
function incomingControl5()
   incomingControl( controls[5] )
   hook.timer(500, "proximity", {anchor = controls[5], radius = 1000, funcname = ("checkClearance5")})
end

-- Player checks security clearance of a ship
function checkClearance( self )
   myjob = occupations[ rnd.rnd(1,#occupations) ]
   tk.msg( control_title, control_text:format(myjob) )
   self:setHilight( false )

   -- Change osd if needed
   n2control = n2control - 1
   if n2control <= 0 then
      n2control = 0 -- Sanity
      misn.osdActive(1)
   end
end
function checkClearance1()
   checkClearance( controls[1] )
   canland[1] = true
end
function checkClearance2()
   checkClearance( controls[2] )
   canland[2] = true
end
function checkClearance3()
   checkClearance( controls[3] )
   canland[3] = true
end
function checkClearance4()
   checkClearance( controls[4] )
   canland[4] = true
end
function checkClearance5()
   checkClearance( controls[5] )
   canland[5] = true
end

-- Spawn Hamelsen in a hyena
function spawnHam()
   hamelsen = pilot.addFleet( "Civilian Hyena", system.get("Beeklo") )[1]
   equipHyena( hamelsen )
   hamelsen:control()
   hamelsen:land(targpla)

   hook.pilot( hamelsen, "land", "hamelsenLanded" )
   hook.pilot( hamelsen, "hail", "controlHailed" )
   hook.timer(500, "proximity", {location = targpos, radius = 10000, funcname = ("incomingHamelsen"), focus = hamelsen})

   -- Hamelsen's partner, whose purpose is to make a fight occur
   jules = pilot.addFleet( "Civilian Hyena", system.get("Beeklo") )[1]
   equipHyena( jules )
   jules:control()
   jules:follow( hamelsen )
end

-- Equips a quick and strong Hyena
function equipHyena( p )
   p:rmOutfit("cores")
   p:rmOutfit("all")
   p:addOutfit("Tricon Zephyr Engine")
   p:addOutfit("Milspec Orion 2301 Core System")
   p:addOutfit("S&K Ultralight Combat Plating")
   p:addOutfit("Shredder",3)
   p:addOutfit("Improved Stabilizer")
   p:addOutfit("Hellburner")
   p:setHealth(100,100)
   p:setEnergy(100)

   -- Remove all cargo (to make them lighter)
   for i, v in ipairs(p:cargoList()) do
      p:cargoRm( v.name, v.q )
   end
end

function hamelsenLanded()
   if stage == 1 then -- Hamelsen managed to land. That is badbadbad.
      tk.msg( lander_title, lander_text )
      misn.finish(false)
   else -- Landing on Laarss: indicate the player he has to follow her
      tk.msg( hamtitle1, hamtext1:format( player.name() ) )
      tk.msg( hamtitle2, hamtext2:format( player.name(), haltpla:name() ) )
      misn.osdDestroy()
      misn.osdCreate( osd_title, {osd_text1:format(haltpla:name()) } )
   end
end

-- Hamelsen is in range: do as usual
function incomingHamelsen()
   audio.soundPlay( "jump" )
   alpha[1]:comm( order_control:format( player.name(), hamelsen:name() ) )
   hamelsen:setHilight()
   hamelsen:setVisible()
   hook.timer(500, "proximity", {anchor = hamelsen, radius = 1000, funcname = ("checkHamelsen")})
   misn.osdActive(2)
end

-- Player checks security clearance of Hamelsen: let the fun begin
function checkHamelsen()
   tk.msg( noanswer_title, noanswer_text:format( player.name(), hamelsen:name() ) )
   hamelsen:setHostile()
   --hamelsen:rename( _("Suspect Hyena") )

   hamelsen:taskClear()
   hamelsen:runaway( player.pilot(), true ) -- First run away (for hellburner) then land
   hook.timer( 5000, "hamelsenTowards" )
   hamelsen:setNoDeath()
   hamelsen:setNoDisable()

   -- Let's rock a bit !
   jules:taskClear()
   jules:attack( player.pilot() )

   stage = 2
   misn.osdDestroy()
   misn.osdCreate( osd_title, {osd_text4:format(hamelsen:name()) } )
end

-- Hamelsen heads towards Laars
function hamelsenTowards()
   hamelsen:taskClear()
   hamelsen:land( haltpla )
end

-- Discuss with an officer on Laarss
function discussOff()
   tk.msg( offtitle, offtext )
   misn.npcRm( npc )
end

-- Add a spy close to Gremlin, make Strafer head to it, and a group of Hyenas kill Strafer to secure the spy's exit
function StraferNspy()

   -- Compute the line on which to place the spy and Strafer
   jppos = jump.pos( jump.get( system.cur(), "Gremlin" ) )
   unitline = (targpos - jppos) * (1/vec2.dist(targpos-jppos))
   strpos = jppos + unitline * 15000
   spypos = jppos + unitline * 12000

   -- First, teleport Strafer far away from any backup
   alpha[2]:rm()
   alpha[2] = pilot.add( "Hyena", "DHC", strpos, _("Lieutenant Strafer") )
   alpha[2]:setVisplayer()
   alpha[2]:control()

   -- Then put the fleeing spy
   spy = pilot.add( "Schroedinger", "Warlords", spypos )
   spy:setVisplayer()
   spy:control()
   spy:hyperspace( system.get("Gremlin") )
   spy:setNoDeath()
   spy:setNoDisable()
   alpha[2]:attack( spy )

   -- Remove all cargo (to control their speed)
   for i, v in ipairs(alpha[2]:cargoList()) do
      alpha[2]:cargoRm( v.name, v.q )
   end
   for i, v in ipairs(spy:cargoList()) do
      spy:cargoRm( v.name, v.q )
   end
end

-- Many enemies jump and kill Strafer
function deathOfStrafer()
   player.pilot():control()
   player.pilot():brake()
   player.cinematics( true, { gui = true } )
   camera.set( alpha[2], true, 20000 )

   tk.msg( spytitle, spytext )

   attackers = {}
   for i = 1, 10 do
      attackers[i] = pilot.add( "Hyena", "Warlords", system.get("Gremlin") )
      attackers[i]:control()
      attackers[i]:attack( alpha[2] )
   end

   hook.pilot( alpha[2], "exploded", "straferDied" )
end

-- Strafer just died: now, there will be action for the player
function straferDied()
   camera.set( )
   player.pilot():control( false )
   player.cinematics( false )
   hook.timer( 1000, "spawnKillers" )

   for i = 1, #attackers do
      attackers[i]:taskClear()
      attackers[i]:hyperspace(system.get("Gremlin"))
   end
   for i = 1, #alpha do
      if alpha[i]:exists() then
         alpha[i]:taskClear()
         alpha[i]:hyperspace(system.get("Gremlin"))
      end
   end
end

-- Killers go after the player around Laarss
function spawnKillers()
   misn.osdDestroy()
   misn.osdCreate( osd_title, {osd_text5} )

   killers = {}
   killers[1] = pilot.add( "Hyena", "Warlords", haltpla, _("Curiatius"), "baddie_norun" )
   killers[2] = pilot.add( "Shark", "Warlords", haltpla, _("Curiatius"), "baddie_norun" )
   killers[3] = pilot.add( "Lancelot", "Warlords", haltpla, _("Curiatius"), "baddie_norun" )

   deadkillers = 0
   for i = 1, #killers do
      hook.pilot( killers[i], "exploded", "killerDied" )
   end

   tk.msg( killertitle, killertext:format(player.name()) )

   hook.timer( 1000, "message", {pilot = killers[1], msg = killerchatter0:format(player.name())} )
   hook.timer( 3000, "message", {pilot = killers[2], msg = killerchatter1} )
   hook.timer( 5000, "message", {pilot = killers[1], msg = killerchatter2} )
   hook.timer( 7000, "message", {pilot = killers[3], msg = killerchatter3} )
   hook.timer( 9000, "message", {pilot = killers[1], msg = killerchatter4} )
   hook.timer( 11000, "message", {pilot = killers[2], msg = killerchatter5} )
   hook.timer( 13000, "message", {pilot = killers[1], msg = killerchatter6} )
   hook.timer( 15000, "message", {pilot = killers[2], msg = killerchatter7} )
end

-- A killer died
function killerDied()
   deadkillers = deadkillers + 1
   if deadkillers >= 3 then -- Watch out that 3 matches #killers
      stage = 4
      tk.msg( kiltitle, kiltext )
      misn.osdDestroy()
      misn.osdCreate( osd_title, {osd_text1:format(destpla:name())} )
   end
end
