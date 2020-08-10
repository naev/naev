--[[
-- Dvaered Escort
-- This is the second mission of the Frontier War Dvaered campaign.
-- The player has to sabotage a Warlord's Goddard in prevision of a duel.
-- The frontier invasion is still not mentionned

   Stages :
   0) Goto find Hamfresser
   1) First try
   2) Fleeing first time
   3) Second try
   4) Fight with the Phalanx
   5) Way back
   6) Watch the duel
   7) Final landing
--]]

require "dat/scripts/nextjump.lua"
require "proximity.lua"
require "selectiveclear.lua"

--TODO: Set the priority and conditions of this mission (should not start at first planet)

portrait_name = _("Major Tam")
portrait_desc = _("Major Tam may be in need of a pilot.")

npc_name = _("Captain Hamfresser")
npc_desc = _("A tall and very large cyborg soldier sits against a wall, right next to the emergency exit. He loudly drinks an orange juice through a pink straw and suspiciously examines the other customers. By the power of his eyes he cleared a large aera around him as people seem to prefer to move away instead of supporting his half-robotic gaze. Unfortunately, he matches the description of your contact, which means you will have to overcome your fear and talk to him.")

tam_name = _("Major Tam and Captain Leblanc")
tam_desc = _("Major Tam and Captain Leblanc seem to be waiting for you.")

team_name = _("Your employers")
team_desc = _("Tam and Leblanc are congratulating their general.")

propose_title = _("Ready for an other mission?")
propose_text = _([[You sit at Tam's table and wait for him to speak. "Hello, citizen %s. You remember Lord Battleaddict, the old warlord who tried to kill us twice? I have good news: with a few other people of the Space Force, we found a way to make him regret what he did, and we need a civilian pilot, like you. Are you in?"]])

accept_title = _("The plan")
accept_text1 = _([["I knew you would accept!" Says Major Tam. "Here is the situation:
   "The general I am working for, General Klank, is in charge of... hem... in charge of a crucial operation the High Command wants to carry on. This operation will involve troops of the High Command, but also Warlords, including Battleaddict. The problem is that General Klank and Lord Battleaddict disagree on everything about this plan. As a consequence, they are going to have a Goddard duel, which is usually what happends when two important Dvaered generals have deep disagrements."]])
accept_text2 = _([["The problem is that Battleaddict's plan is way too stupid. It would weaken the Dvaered Nation durably and let us at the mercy of all the other nations around us, that are waiting for us to show a sign of weakness in order to attack us and impose their iniquitus and obsolete political system to our citizen." Tam takes a deep breath and looks at you in the eyes. "You don't know, citizen, all the dreadful enemies who are waiting in shadow, their hearth filled with hatred against House Dvaered. Sometimes I look at the constelladet sky and I wonder. I wonder why the Dvaered Nation has to be the only threatened islet of justice and compassion in this... in this Sea of Darkness.
   "Hey, citizen! But I have good news! We won't fall to the Barbarian hordes! Because me myself, Major Archibald Tom, I have a plan. We will make sure that Lord Battleaddict looses his duel. Please note however that if the very existence of House Dvaered was not threatened, we would never allow ourselves to interfere in a honorful duel between two respectable gentlemen. Go to %s in %s and meet the Captain Hamfresser (his portrait is attached in the data I will give to you). He will explain the details. It is very important that you use a civilian ship that can transport at least %d tons of cargo."]])

refuse_title = _("Refusal")
refuse_text = _([["Allright, citizen, see you later, then."]])

not_title = _("Not enough free space")
not_text = _("Your ship does not have enough free space. Come back with %d tons free.")

ham_title = _("New passengers")
ham_text1 = _([["H... hi", you say, softly shaking your hand "Are you the captain Hamfresser?". The soldier answers "Of course, as it is written on my nametag." Showing a small label on his chest, where stay the letters "Hamfresser", beside his Capitain gallons and the logo of the Dvaered Space Infantry (a mace of arms with wings). Hamfresser looks at you from top to bottom "You're the private pilot, right? Tell me the number of the dock you ship is at, I'll meet you there. Oh, and please make room for %d tons of cargo."
   The captain then gets up, delicately puts his empty glass on the counter, and goes away. While his hair scratch the ceiling, you wonder if %d tons are enough for him. When you arrive at the dock, you see Hamfresser, with five other soldiers and two androids that load a huge and strange machine into your ship. "Hey" you say "What are you doing with your... your death machine?" Hamfresser approaches and answers at low voice "But, mate, this is not a death machine, It's just a bomb. Or even a bomblet."]])
ham_text2 = _([["Very well" you abdicate. "Do what you have to do". Once the cargo is loaded and the team has taken place in the cabin, you start to talk with the captain. "And I suppose this bomblet is destined to Battleaddict's Goddard. How are we supposed to put it there? Are we going to pretend it's a gift from the High Command to his granddaughter?". Hamfresser looks at you surprised. "No... that's not what the Major... do you think it could work?" You realize it would take too long to explain that it was a sarcastic comment (assuming that this guy knew what is a sarcastic comment) and simply ask him to explain the major's plan.
   "Last period, we intercepted a message from Battleaddict to the plumber. His cruiser has issues with sewage disposal and he requested an intervention. So we abducted the plumber and we disguised an EMP bomb into a sewage emptier. We will dock with him, put the bomb, repair the breakdown (so that he won't suspect us) and go away. Private Ling here is a Goddard-plumber so she will lead us." A young and smiling soldier raises her hand, saying "Hi".]])
ham_text3 = _([[While you wonder wether the plan is awsomely brillant or dead stupid, Hamfresser starts to do the presentations. "This is Sergeant Nikolov, she is my second in command, this is Private Tronk, from my squad, and Caporal Therus, our medical support. Oh, and the guy in his corner over there is Lieutenant Strafer. He is a pilot from the Special Operations. He is here in case we need to switch to plan B." As you ask what plan B is, Hamfresser simply answers "You don't want to switch to plan B.
   "As usual, Lord Battleaddict's cruiser should be in orbit around %s in %s".]])

enter1_title = _("Not far from goal")
enter1_text = _([[As you finish your jump, Lieutenant Strafer approaches to your radar screen "Battleaddict's Goddard should be around %s. I guess he should be at adding nanobond plating and repeating railguns everywhere possible by now. There should be a few patrol ships around him that will control our security clearance." Hamfresser gives instructions to the team: "Everyone put your plumber suit on, Nikolov, switch the decoder on, so that we will monitor the transmissions of the escort ships. It could tell us if we're detected."]])

check_title = _("You're controlled")
check_text = _([[As you approach to the lighter ships that protect the cruiser, a Vendetta hails you. Hamfresser answers "We're Johnson and Jhonson, associate plumbers. We've an appointment with Mr. Battleaddict, it's about a sewage disposal problem." The fighter pilot answers: "It's allright citizen, your transponder code is right. You may pass." You make your ship move forwards while Hamfresser greets the pilot with an obsequious "Thank you, mister officer."
   "By the way," says the Lieutenant Strafer once the communication has been closed, "This cruiser probably has no turreted weapons in prevision to the duel, so I would recommend to approach it from the back, just in case."]])

kill_title = _("That could have worked")
kill_text = _([[While approaching, you start to distinguish better the surface of the cruiser. You see a dozen of shuttles that transport material and tools form the planet to the ship, and when getting closer, you remark that the cruiser looks like an huge construction site with workers in spacesuit who are welding nanobond rienforcement plates on the hull. You hear behind you the chatter of the escort ships, that Hamfresser and his team are anxiously listening to: "Hey, Zog, I'm getting concerned about my daughter, her teacher told me she was non-violent with her classmates, do you think I should see a specialist?" "Meh, I don't know, honestly. The new holomovies are to blame, there is always less violence and more love in there. The government should take measures."
   Suddently, a message makes everyone come to a halt: "So, Colonel, when do we blow those fake plumbers out? I look forward to using my shredders a bit!" "Shut up, Lieutenant!" "Oah, come on, I'm on the encoded channel. Plumbers are not able to break our code" "But they're NOT plumbers, stupid!"
   Hamfresser looks at you, and simply declares "We abort the mission. get us out of that system, %s!" Strangely enough, none of the soldiers seem to show up signs of panic.]])

combat_title = _("We told you not to use a combat ship!")
combat_text = _([[As you approach, the Lieutenant Strafer looks at your radar screen. "We are in a combat ship. We told you not to use a combat ship. Now, they are going to attack us! Why did you have to use a combat ship? We'll have to abort the mission now. All because of your bloody combat ship!"]])

capt_title = _("What are you doing here?")
capt_text = _("This planet belongs to Lord Battleaddict. You will be captured if you land here. The mission failed.")

escape_title = _("We're safe now")
escape_text = _([[Once the ship recovers its normal speed after jumping, Hamfresser says: "Strange, I wouldn't have believed that we survive this one. Would you, Strafer?" The lieutenant answers "I agree, captain. I guess we've got a good pilot." As you ask them why they were not scared then, Hamfresser answers: "Of course we were scared, who would not be? But we are trained not to show our fear, it tends to distract the pilots."
   A few seconds later, you recieve an encoded inter-system message from Major Tam: "The plan A has leaked. Please do switch to plan C. Do not jump in %s by any means. For information, the leak is under control and the source has been dealed with." The voice makes a pause and continues: "I really hope this messages catches you before you enter %s. Otherwise, may Dvaerius, the patron saint of mace rockets have mercy for your souls..." "Good old Tamtam," Hamfresser says smiling, "he always worries too much for us."]])

next_title = _("The plan C")
next_text = _([["Allright, everyone, we're now heading to %s in %s. According to our intelligence, there should be a Phalanx from Battleaddict's fleet that will take off from there soon. It's name is 'Gorgon'. They are on their way back in a transport mission. According to the analysts, there should be enough free space in this ship for our bomb. We will disable the ship, neutralize the pilot and load our material. After that, %s will report back to the Major on %s and the rest of the team will execute the rest of the plan. I or Sergeant Nikolov will brief you once in the Phalanx."]])

board_title = _("Boarding")
board_text = _([[All the members of the commando have put on their battle suits. Hamfresser gives the last orders. "Nikolov, Tronk and I will enter first and clean the aera. Remember that we don't have our usual Dudley combat androids, we're stuck with the two useless plumber bots and the few security droids of %s's ship. As much to say, we'll have to work by hand. I recall for everyone that corvettes are typically protected by a few 629 Spitfires and an occasionnal 711 Grillmeister. That's not very much, but still enough to send the inattentive soldier ad patres."
   When the corvette's airlock falls under Nikolov's circular saw, the captain waves and the small team enters the ship. You then hear shots and explosions coming from further and further in the enemy ship. Finally, you hear a laconic message coming from the disabled corvette "Here is Strafer, everything went well. We transfer now the cargo into the Phalanx... Allright, the maneuver is finished, you may leave." Happy to have survived the operation so far, you start your engines and respond "Good luck, folks!". The lieutenant answers "Thanks, citizen, I'm glad I've meet you."]])

pdie_title = _("Mission Failed: target destroyed")
pdie_text = _("You were supposed to disable that ship, not to destroy it. How are you supposed to transport the bomb now?")

pesc_title = _("Mission Failed: target escaped")
pesc_text = _("You were supposed to disable that ship, not to let it escape. How are you supposed to transport the bomb now?")

preduel_title = _("Ready to attend to the show?")
preduel_text = _([[As you sit at the table, Tam starts to speak: "I got a message from Capitain Hamfresser. Apparently, everything went according to the plan this time. They should have docked with the Goddard, allegedly to add their mission log to the central database. Then they have put the bomb in the plumbing, close to the central unit, and they have faked an accident while landing on %s. I guess they should be hiking somewhere at the planet's surface by now, looking for the opportunuity to steal an unfortunate civilian's Llama in order to make their trip back.
   "Our boss, the General Klank is ready for the duel. The Captain and I are his duel witnesses, so we should be joining our pageantry ships by now. Oh, and the duel commissioner is someone you already know, the Colonel Urnus. In about a period, Lord Battleaddict should arrive, so if you take off soon, you will see the duel."]])

duel1_title = _("Here we go")
duel1_text = _([[Colonel Urnus's ship broadcasts the message: "I, Colonel Urnus, have been requested by both parts of this duel to be today's commissioner. I hereby solemnly swear to be a right person, officer of the Dvaered Army, respectful of our laws and our customs, and I have never worked under the command nor as a commander of any of the generals who are having this duel. I have controlled the pedigree of the four witnesses and I can attest they are respectable officers of the Dvaered Army. Lord Battleaddict, General Klank, before proceeding to the combat, I have to ask you one last time: Are you sure your disagreement can not be solved by any other means?"
   A formal silence follows the words of the colonel, but soon Battleaddict and Klank respond: "It can not, mister commissioner." And Urnus continues: "I am guarantor of the fact that this duel is conform to the rules established by our ancestors. I have controlled both ships and I attest that no irregularity is to be noticed. Let the fight begin, may the most virtuous of you both survive."]])

epilogue_title = _("A good thing done")
epilogue_text = _([[As you approach, the major tells the General Klank that you are the private pilot they had hired recently. "I see," says the general "so you are part of the people I have to thank for still being alive now." You answer that he apparently would not have needed help if Battleaddict had not cheated as well, and he responds: "Damn fake electricians I should have suspected something. Anyway, citizen, be ensured that we will need your services again." A group of generals then approaches and congratulate Klank. He stands up and goes away with them, loudly exchanging dubious pleasantries with them.
   Major Tam then speaks to you: "Apparently, Battleaddict had got a commando to dress like electricians and to hide an EMP bomb in the General's Goddard, that exploded during the fight, right like our own bomb. And now, both ships have their systems ruined. Well, anyway, thank you for your help, here are %d credits for you!"]])


phalanx_comm_fight = _("You made a very big mistake!")
phalanx_comm_flee = _("Just try to catch me, you pirate!")

duel_broadcast = _("Let the fight begin!")

duel_comm1 = _("Come on, boss!")
duel_comm2 = _("DESTROY HIM!")
duel_comm3 = _("You're the best, boss!")
duel_comm4 = _("Yeah!")

disable_comm1 = _("Damn!")
disable_comm2 = _("Oooooo...")
disable_comm3 = _("What the?")
disable_comm4 = _("Come on!")

disable_comm5  = _("Hey, they have put a bomb in the general's ship as well!")
disable_comm6  = _("The electricians! We've called electricians recently! They did put the bomb!")
disable_comm12 = _("Cheaters!")
disable_comm13 = _("Cheaters youselves!")

disable_comm7  = _("Hey, Battleaddict, it seems we are both down...")
disable_comm8  = _("I still want to kill you!")
disable_comm9  = _("So do I.")
disable_comm10 = _("Luckly enough, I've my Vendetta in the fighter bay.")
disable_comm11 = _("So do I.")

takoff_comm1 = _("Shall we continue?")
takoff_comm2 = _("Of course, we shall!")

kill_comm1 = _("Aha! In your freaking monkey face!")
kill_comm2 = _("You're the best, general!")
kill_comm3 = _("Oooooo...")
kill_comm4 = _("Nooooo!")
kill_comm5 = _("The General Klank won the duel!")

osd_title = _("Dvaered Sabotage")
osd_msg1  = _("Pick up Hamfresser in %s in %s. Use a civilian ship with at least %d tons of free cargo")
osd_msg2  = _("Meet Battleaddict around %s in %s")
osd_msg3  = _("Deposit Hamfresser on %s in %s")
osd_msg4  = _("Jump out. Do NOT land in this system")
osd_msg5  = _("Go to %s, apporach %s and wait for the Phalanx")
osd_msg6  = _("Wait for the Phalanx to take off")
osd_msg7  = _("Disable and board the Phalanx")
osd_msg8  = _("Report back on %s in %s")
osd_msg9  = _("Attend to the duel")
osd_msg10 = _("Land on %s")

misn_desc = _("You have to sabotage Lord Battleaddict's cruiser in order to ensure General Klank's victory at a duel.")
misn_reward = _("Focus on the mission, pilot.")

log_text = _("Major Tam's superior, General Klank, made a Goddard duel with Lord Battleaddict. You took part to an operation to sabotage Battleaddict's cruiser. Lord Battleaddict made sabotage Klank's cruiser as well, but at the end of the day, General Klank won the duel.")

bombMass = 100
credits = 50000


function create()
   hampla, hamsys     = planet.get("Morgan Station")
   sabotpla, sabotsys = planet.get("Buritt")
   duelpla, duelsys   = planet.get("Dvaer Prime")
   intpla, intsys     = planet.get("Timu")

   if not misn.claim ( {sabotsys, duelsys, intsys} ) then
      misn.finish(false)
   end

   misn.setNPC(portrait_name, "dvaered/dv_military_m3")
   misn.setDesc(portrait_desc)
end

function accept()
   if not tk.yesno( propose_title, propose_text:format(player.name()) ) then
      tk.msg(refuse_title, refuse_text)
      misn.finish(false)
   end
   tk.msg(accept_title, accept_text1)
   tk.msg(accept_title, accept_text2:format(hampla:name(), hamsys:name(), bombMass))

   misn.accept()
   misn.setDesc(misn_desc)
   misn.setReward(misn_reward)

   stage = 0
   hook.land("land")
   misn.osdCreate( osd_title, {osd_msg1:format(hampla:name(),hamsys:name(),bombMass), osd_msg2:format(sabotpla:name(),sabotsys:name()), osd_msg3:format(duelpla:name(),duelsys:name())} )
   mark = misn.markerAdd(hamsys, "low")
end

function land()
   if stage == 0 and planet.cur() == hampla then -- Meet captain Hamfresser
      captain = misn.npcAdd("hamfresser", npc_name, "dvaered/dv_military_m7", npc_desc)

   elseif stage == 2 then -- The player landed somewhere on Battleaddict's system
      tk.msg( capt_title, capt_text )
      misn.finish(false)

   elseif stage == 5 and planet.cur() == duelpla then -- Report back
      tam = misn.npcAdd("majorTam", tam_name, "dvaered/dv_military_m3", tam_desc)
      leb = misn.npcAdd("majorTam", tam_name, "dvaered/dv_military_f8", tam_desc)

   elseif stage == 7 and planet.cur() == duelpla then -- Epilogue
      tam = misn.npcAdd("endMisn", team_name, "dvaered/dv_military_m3", team_desc)
      leb = misn.npcAdd("endMisn", team_name, "dvaered/dv_military_f8", team_desc)
      kla = misn.npcAdd("endMisn", team_name, "dvaered/dv_general1", team_desc)
   end
end

function hamfresser()
   if (player.pilot():cargoFree() >= bombMass) then
      tk.msg( ham_title, ham_text1:format(bombMass,bombMass) )
      tk.msg( ham_title, ham_text2 )
      tk.msg( ham_title, ham_text3:format(sabotpla:name(),sabotsys:name()) )
      stage = 1
      hook.enter("enter")
      bomblet = misn.cargoAdd("Bomb", bombMass)

      misn.markerRm(mark)
      misn.osdActive(2)
      mark = misn.markerAdd(sabotsys, "low")
      player.takeoff() -- TODO: takeoff at loading as well
   else
      tk.msg(not_title, not_text:format(bombMass))
   end
end

function enter()
   -- Spawn Battleaddict and his team
   if stage == 1 and system.cur() == sabotsys then
      pilot.toggleSpawn("FLF") -- TODO : It's only for testing. It can be removed once FLF is dead
      pilot.clearSelect("FLF") --
      pilot.toggleSpawn("Pirate")
      pilot.clearSelect("Pirate")

      warlord = pilot.add("Dvaered Goddard", nil, sabotpla)[1]
      warlord:rename( "Lord Battleaddict" )
      warlord:control(true)
      warlord:goto( sabotpla:pos() + vec2.newP(rnd.rnd(1000), rnd.rnd(360)) )
      warlord:memory().formation = "circleLarge"
      warlord:setHilight()
      equipGoddard( warlord, false )

      p = {}
      for i = 1, 4 do
         p[i] = pilot.add("Dvaered Vendetta", nil, sabotpla)[1]
         p[i]:setLeader(warlord)
      end
      for i = 1, 2 do
         p[i+4] = pilot.add("Dvaered Ancestor", nil, sabotpla)[1]
         p[i+4]:setLeader(warlord)
      end
      p[7] = pilot.add("Dvaered Phalanx", nil, sabotpla)[1]
      p[7]:setLeader(warlord)
      p[8] = pilot.add("Dvaered Vigilance", nil, sabotpla)[1]
      p[8]:setLeader(warlord)
      p[8]:rename("Colonel Hamelsen")
      p[8]:setNoDeath()

      hook.timer(4000, "enter1_message")
      hook.timer(500, "proximity", {anchor = warlord, radius = 2000, funcname = "meeting", focus = player.pilot()})
      hook.timer(500, "proximity", {anchor = warlord, radius = 300, funcname = "killing", focus = player.pilot()})

   elseif stage == 2 then
      hook.timer(2000, "enter2_message")
      stage = 3
      misn.osdDestroy()
      misn.osdCreate( osd_title, {osd_msg5:format(intsys:name(),intpla:name()), osd_msg7, osd_msg8:format(duelpla:name(),duelsys:name())} )
      mark = misn.markerAdd(intsys, "low")

   elseif stage == 3 and system.cur() == intsys then
      hook.timer(500, "proximity", {location = intpla:pos(), radius = 1000, funcname = "spawn_phalanx", focus = player.pilot()})

   elseif stage == 6 and system.cur() == duelsys then
      pilot.toggleSpawn(false)
      pilot.clear()

      mypos = duelpla:pos()
      step = 150

      klank = pilot.add("Dvaered Goddard", nil, mypos + vec2.new(-step, step/2))[1]
      klank:rename( "General Klank" )
      klank:control(true)
      klank:setFaction("DHC")
      equipGoddard( klank, true ) -- Klank's superior equipment should ensure victory

      battleaddict = pilot.add("Dvaered Goddard", nil, mypos + vec2.new(step, step/2))[1]
      battleaddict:rename( "Lord Battleaddict" )
      battleaddict:control(true)
      battleaddict:setFaction("Warlords")
      equipGoddard( battleaddict, false )

      klank:face(battleaddict)
      battleaddict:face(klank)

      urnus = pilot.add("Dvaered Vigilance", nil, mypos + vec2.new(0, 3*step/2))[1]
      urnus:rename( "Colonel Urnus" )
      urnus:control(true)
      urnus:face( mypos + vec2.new(0, step/2) )

      tam = pilot.add("Dvaered Vigilance", nil, mypos + vec2.new(-2*step, 3*step/2))[1]
      tam:rename( "Major Tam" )
      tam:control(true)
      tam:face(battleaddict)

      leblanc = pilot.add("Dvaered Phalanx", nil, mypos + vec2.new(-2*step, -step/2))[1]
      leblanc:rename( "Captain Leblanc" )
      leblanc:control(true)
      leblanc:face(battleaddict)

      hamelsen = pilot.add("Dvaered Vigilance", nil, mypos + vec2.new(2*step, 3*step/2))[1]
      hamelsen:rename( "Colonel Hamelsen" )
      hamelsen:control(true)
      hamelsen:face(klank)

      randguy = pilot.add("Dvaered Vigilance", nil, mypos + vec2.new(2*step, -step/2))[1]
      randguy:control(true)
      randguy:face(klank)

      player.pilot():control()
      player.pilot():goto( mypos + vec2.new(0, -step/2) ) -- To avoid being in the range
      player.cinematics( true, { gui = true } )

      camera.set( mypos + vec2.new(0, step/2) )

      hook.timer(5000, "beginDuel")
      hook.timer(15000, "disableDuel")
      hook.timer(55000, "fighterDuel")
   end
end

-- Equips a Goddard for a duel, with or without repeating railguns
function equipGoddard( pilot, repeating )
   pilot:rmOutfit("all")
   pilot:rmOutfit("cores")
   pilot:addOutfit("S&K Superheavy Combat Plating")
   pilot:addOutfit("Melendez Mammoth XL Engine")
   pilot:addOutfit("Nanobond Plating", 6)

   if repeating then
      pilot:addOutfit("Milspec Hermes 9802 Core System")
      pilot:addOutfit("Repeating Railgun", 7)
      pilot:addOutfit("Reactor Class III", 5)
   else
      pilot:addOutfit("Milspec Prometheus 9803 Core System")
      pilot:addOutfit("Railgun", 7)
      pilot:addOutfit("Reactor Class II", 5)
   end
   pilot:setHealth(100,100)
   pilot:setEnergy(100)
   pilot:setFuel(true)
end

function equipVendetta( pilot )
   pilot:rmOutfit("all")
   pilot:rmOutfit("cores")
   pilot:addOutfit("S&K Light Combat Plating")
   pilot:addOutfit("Tricon Zephyr II Engine")
   pilot:addOutfit("Shield Capacitor")
   pilot:addOutfit("Milspec Aegis 3601 Core System")
   pilot:addOutfit("Small Shield Booster")
   pilot:addOutfit("Unicorp Mace Launcher", 6)

   pilot:setHealth(100,100)
   pilot:setEnergy(100)
   pilot:setFuel(true)
end

function enter1_message()
   tk.msg(enter1_title, enter1_text:format(sabotpla:name()))
end

function enter2_message()
   tk.msg(escape_title, escape_text:format(sabotsys:name(), sabotsys:name()))
   tk.msg(next_title, next_text:format(intpla:name(), intsys:name(), player.name(), duelpla:name()))
end

-- Battleaddict agrees for the player to approach
function meeting()
   if player_civilian() then
      tk.msg(check_title, check_text)
   else
      tk.msg(combat_title, combat_text)
      release_baddies()
      mission.finish(false)
   end
end

-- Battleaddict sees that the player is not a plumber
function killing()
   tk.msg(kill_title, kill_text:format(player.name()))
   release_baddies()
   stage = 2

   misn.osdDestroy()
   misn.osdCreate( osd_title, {osd_msg4} )
   misn.markerRm(mark)
end

function release_baddies()
   warlord:setFaction("Warlords")
   warlord:control(false)
   for i, j in ipairs(p) do
      j:setFaction("Warlords")
   end
end

-- Test civilian ships
function player_civilian()
   local playerclass = player.pilot():ship():class()
   return (playerclass == "Luxury Yacht" or playerclass == "Yacht" or playerclass == "Courier" or playerclass == "Freighter" or playerclass == "Armoured Transport")
end

-- Spawn the Phalanx to disable
function spawn_phalanx()
   p = pilot.add("Dvaered Phalanx", nil, intpla)[1]
   p:setFaction("Warlords")
   p:setHilight()
   p:rename("Gorgon")
   p:control()

   nextsys = getNextSystem(system.cur(), sabotsys)
   p:hyperspace( nextsys ) -- Go towards Battleaddict's place

   p:rmOutfit("all")
   p:rmOutfit("cores")
   p:addOutfit("S&K Medium Combat Plating")
   p:addOutfit("Milspec Prometheus 4703 Core System")
   p:addOutfit("Tricon Cyclone Engine")
   p:addOutfit("Turreted Vulcan Gun", 2)
   p:addOutfit("Mass Driver MK3")
   p:addOutfit("Vulcan Gun", 2)
   p:addOutfit("Reactor Class I")
   p:addOutfit("Medium Cargo Pod", 2)
   p:setHealth(100,100)
   p:setEnergy(100)
   p:setFuel(true)

   pattacked = hook.pilot( p, "attacked", "phalanx_attacked" )
   pboarded = hook.pilot( p, "board", "phalanx_boarded" )
   hook.pilot( p, "death", "phalanx_died" )
   hook.pilot( p, "jump", "phalanx_safe" )
   hook.pilot( p, "land", "phalanx_safe" )

   stage = 4
   misn.osdActive(2)
   -- TODO: not possible to jump out nor land
end

-- Decide if the Phalanx flees or fight
function phalanx_attacked()
   hook.rm(pattacked)
   --if weakerThanPlayer(p) then
   if true then
      p:taskClear()
      p:comm( phalanx_comm_flee )
      p:runaway(player.pilot())
   else
      p:control(false)
      p:comm( phalanx_comm_fight )
   end
end

function phalanx_boarded()
   hook.rm(pboarded)
   tk.msg( board_title, board_text:format(player.name()) )
   stage = 5
   misn.cargoRm(bomblet)

   player.unboard() -- Prevent the player form actually boarding the ship
   p:setFaction("DHC")
   p:control(true)
   p:taskClear()
   p:hyperspace( nextsys )

   misn.osdActive(3)
   misn.markerRm(mark)
   mark = misn.markerAdd(duelsys, "low")
end

-- Mission failed: phalanx died
function phalanx_died()
   tk.msg( pdie_title, pdie_text )
   misn.finish(false)
end

-- Mission failed: phalanx escaped
function phalanx_safe()
   tk.msg( pesc_title, pesc_text )
   misn.finish(false)
end

-- TODO: base it on ship class otherwise, it incitates the player to sabotage their own firepower
function weakerThanPlayer( target_ship )
   pstat = player.pilot():stats()
   tstat = target_ship:stats()
   return ( ((pstat.armour + pstat.shield)) > (tstat.armour + tstat.shield) ) 
end

function majorTam()
   tk.msg( preduel_title, preduel_text:format(sabotpla:name()) )
   stage = 6

   misn.osdDestroy()
   misn.osdCreate( osd_title, {osd_msg9, osd_msg10:format(duelpla:name())} )
   misn.markerRm(mark)
end

-- Starts the duel
function beginDuel()
   tk.msg( duel1_title, duel1_text )
   klank:taskClear()
   klank:attack(battleaddict)
   klank:setNoDeath() -- Actually it should not be necessary, but...
   battleaddict:taskClear()
   battleaddict:attack(klank)

   urnus:broadcast( duel_broadcast )
   hook.timer( 1000, "message", {pilot = tam, msg = duel_comm1} )
   hook.timer( 2000, "message", {pilot = leblanc, msg = duel_comm2} )
   hook.timer( 3000, "message", {pilot = hamelsen, msg = duel_comm3} )
   hook.timer( 4000, "message", {pilot = randguy, msg = duel_comm4} )

   hook.pilot( battleaddict, "exploded", "battleaddict_killed" )
end

-- Disables the ships
function disableDuel()
   klank:disable()
   battleaddict:disable()

   -- Explosion and such
   spfx.soundPlay( "empexplode" )
   camera.shake()
   hook.timer(1000, "moreSound1")
   hook.timer(2000, "moreSound2")

   hook.timer( 2000, "message", {pilot = tam, msg = disable_comm1} )
   hook.timer( 4000, "message", {pilot = leblanc, msg = disable_comm2} )
   hook.timer( 6000, "message", {pilot = hamelsen, msg = disable_comm3} )
   hook.timer( 8000, "message", {pilot = randguy, msg = disable_comm4} )

   hook.timer( 11000, "message", {pilot = tam, msg = disable_comm5} )
   hook.timer( 15000, "message", {pilot = leblanc, msg = disable_comm6} )
   hook.timer( 19000, "message", {pilot = tam, msg = disable_comm12} )
   hook.timer( 23000, "message", {pilot = hamelsen, msg = disable_comm13} )

   hook.timer( 28000, "message", {pilot = klank, msg = disable_comm7} )
   hook.timer( 30000, "message", {pilot = battleaddict, msg = disable_comm8} )
   hook.timer( 32000, "message", {pilot = klank, msg = disable_comm9} )
   hook.timer( 34000, "message", {pilot = klank, msg = disable_comm10} )
   hook.timer( 36000, "message", {pilot = battleaddict, msg = disable_comm11} )
end

function moreSound1()
   spfx.soundPlay( "beam_off0" )
end
function moreSound2()
   spfx.soundPlay( "hyperspace_powerdown" )
end

-- Gets someone to make a message
function message( arg )
   local pilot = arg.pilot
   local msg = arg.msg
   pilot:broadcast( msg )
end

-- Fighter duel
function fighterDuel()
   klank2 = pilot.add("Dvaered Vendetta", nil, klank:pos())[1]
   klank2:rename( "General Klank" )
   klank2:control(true)
   klank2:setFaction("DHC")
   equipVendetta( klank2 ) -- Klank's superior equipment should ensure victory once more

   battleaddict2 = pilot.add("Dvaered Vendetta", nil, battleaddict:pos())[1]
   battleaddict2:rename( "Lord Battleaddict" )
   battleaddict2:control(true)
   battleaddict2:setFaction("Warlords")

   battleaddict2:broadcast( takoff_comm2 )
   hook.timer( 1000, "message", {pilot = klank2, msg = takoff_comm1} )
   hook.timer( 1500, "message", {pilot = tam, msg = duel_comm1} )
   hook.timer( 2000, "message", {pilot = leblanc, msg = duel_comm2} )
   hook.timer( 2500, "message", {pilot = hamelsen, msg = duel_comm3} )
   hook.timer( 3000, "message", {pilot = randguy, msg = duel_comm4} )

   klank2:setNoDeath() -- Actually it should not be necessary, but...
   klank2:setNoDisable()

   klank2:memory().atk = atk_drone  -- Set the AI so that they don't get stuck
   battleaddict2:memory().atk = atk_drone

   battleaddict2:control()
   battleaddict2:goto( mypos + vec2.new(step,step/4), false, false ) -- Prevent them from staying on the top of their ships
   battleaddict2:attack(klank2)
   klank2:control()
   klank2:goto( mypos + vec2.new(-step,step/4), false, false )
   klank2:attack(battleaddict2)
   hook.pilot( battleaddict2, "exploded", "battleaddict_killed" )

   --camera.set(klank2)
end

function battleaddict_killed()
   tam:broadcast( kill_comm1 )
   leblanc:broadcast( kill_comm2 )
   hamelsen:broadcast( kill_comm3 )
   randguy:broadcast( kill_comm4 )
   urnus:broadcast( kill_comm5 )

   hook.timer( 2000, "everyoneLands" )
   camera.set()
   player.pilot():control( false )
   player.cinematics( false )

   stage = 7
   misn.osdActive(2)
end

function everyoneLands()
   everyone = { klank, klank2, urnus, tam, leblanc, hamelsen, randguy }
   for i, p in ipairs(everyone) do
      p:taskClear()
      p:land(duelpla)
   end
end

-- Epilogue
function endMisn()
   tk.msg( epilogue_title, epilogue_text:format(credits) )
   player.pay(credits)
   misn.finish(true)
   shiplog.createLog( "frontier_war", _("Dvaered Military Coordination"), _("Dvaered") ) --TODO: create a common file for this TODO: see if it's possible to rename it afterwards
   shiplog.appendLog( "frontier_war", log_text )
end
