--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Sabotage">
 <unique />
 <priority>2</priority>
 <chance>20</chance>
 <done>Dvaered Escort</done>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <notes>
  <campaign>Frontier Invasion</campaign>
 </notes>
</mission>
--]]
--[[
-- Dvaered Sabotage
-- This is the second mission of the Frontier War Dvaered campaign.
-- The player has to sabotage a Warlord's Goddard in prevision of a duel.
-- The frontier invasion is still not mentioned

   Stages :
   0) Goto find Hamfresser
   1) First try. TODO: tell the player it's preferable to have refueled just before jumping in Ginger
   2) Fleeing first time. TODO: see if it's possible to jettison the bomb (and make it not possible)
   3) Second try
   4) Fight with the Phalanx
   5) Way back
   6) Watch the duel
   7) Final landing
--]]

local atk_generic = require "ai.core.attack.generic"
local lmisn = require "lmisn"
require "proximity"
local fw = require "common.frontier_war"
local fmt = require "format"
local pir = require "common.pirate"
local cinema = require "cinema"
local ai_setup = require "ai.core.setup"

-- Mission constants
local bombMass = 100
local hampla, hamsys     = spob.getS("Stutee") --Morgan Citadel
local sabotpla, sabotsys = spob.getS(fw.wlrd_planet)
local duelpla, duelsys   = spob.getS("Dvaer Prime")
local intpla, intsys     = spob.getS("Timu")

-- Non-persistent state
local p, ps -- active pilot/fleet
local battleaddict, battleaddict2, hamelsen, klank, klank2, leblanc, randguy, tam, urnus, warlord -- pilots in the plot
local mypos, step -- location and spacing of the duel, initialized with the above pilots

local equipGoddard, player_civilian, release_baddies -- Forward-declared functions

-- common hooks
message = fw.message

function create()
   if spob.cur() == hampla then
      misn.finish(false)
   end

   if not misn.claim ( {sabotsys, duelsys, intsys} ) then
      misn.finish(false)
   end

   misn.setNPC(_("Major Tam"), fw.portrait_tam, _("Major Tam may be in need of a pilot."))
end

function accept()
   if not tk.yesno( _("Ready for another mission?"), fmt.f(_([[You sit at Tam's table and wait for him to speak. "Hello, citizen {player}. You remember Lord Battleaddict, the old warlord who tried to kill us twice? I have good news: with a few other members of the Space Force, we've devised a way to make him regret what he did, and we need a civilian pilot, like you. Are you in?"]]), {player=player.name()}) ) then
      tk.msg(_("Refusal"), _([["Alight, citizen, see you later, then."]]))
      return
   end
   tk.msg(_("The plan"), _([["I knew you would accept!" Says Major Tam. "Here is the situation:
   "The general I am working for, General Klank, is in charge of... hem... in charge of a crucial operation the High Command wants to carry out. This operation will involve troops of the High Command, but also Warlords, including Battleaddict. The problem is that General Klank and Lord Battleaddict disagree on everything about this plan. As a consequence, they are going to have a Goddard duel, which is usually how two important Dvaered generals settle deep disagreements."]]))
   tk.msg(_("The plan"), fmt.f(_([["The problem is that Battleaddict's plan is far too stupid. It would weaken the Dvaered Nation in the long run and leave us at the mercy of all the other nations around us. We can't afford to show any signs of weakness or they will attack us and impose their iniquitous and obsolete political systems on our citizenry." Tam takes a deep breath and looks you in the eyes. "You don't know, citizen, all the dreadful enemies who are waiting in the shadows, their hearts filled with hatred against House Dvaered. Sometimes I look at the star-filled night sky and I wonder. I wonder why the Dvaered Nation has to be the only threatened islet of justice and compassion in this... in this Sea of Darkness.
   "Hey, citizen! But I have good news! We won't fall to the Barbarian hordes! Because I myself, Major Archibald Tam, I have a plan. We will make sure that Lord Battleaddict loses his duel. Please note, however, that if the very existence of House Dvaered was not threatened, we would never allow ourselves to interfere in a honourable duel between two respectable gentlemen. Go to {pnt} in {sys} and meet Captain Hamfresser. His portrait is attached in the data I will give you. He will explain the details. It is very important that you use a civilian ship that can transport at least {tonnes} of cargo."]]), {pnt=hampla, sys=hamsys, tonnes=fmt.tonnes(bombMass)}))

   misn.accept()
   misn.setDesc(_("You have to sabotage Lord Battleaddict's cruiser in order to ensure General Klank's victory at a duel."))
   misn.setReward(_("Focus on the mission, pilot."))

   mem.stage = 0
   hook.land("land")
   misn.osdCreate( _("Dvaered Sabotage"), {
      fmt.f(_("Pick up Hamfresser in {pnt} in {sys}. Use a civilian ship with at least {tonnes} of free cargo"), {pnt=hampla, sys=hamsys, tonnes=fmt.tonnes(bombMass)}),
      fmt.f(_("Meet Battleaddict around {pnt} in {sys}"), {pnt=sabotpla, sys=sabotsys}),
      fmt.f(_("Deposit Hamfresser on {pnt} in {sys}"), {pnt=duelpla, sys=duelsys}),
   } )
   mem.mark = misn.markerAdd(hampla, "low")
end

function land()
   if mem.stage == 0 and spob.cur() == hampla then -- Meet Captain Hamfresser
      misn.npcAdd("hamfresser", _("Captain Hamfresser"), fw.portrait_hamfresser, _("A tall, and very large, cyborg soldier sits against a wall, right next to the emergency exit. He loudly drinks an orange juice through a pink straw and suspiciously examines the other customers. By the power of his glare he cleared a large area around him as people seem to prefer to move away instead of meeting his half-robotic gaze. Unfortunately, he matches the description of your contact, which means you will have to overcome your fear and talk to him."))

   elseif mem.stage == 2 then -- The player landed somewhere on Battleaddict's system
      tk.msg( _("What are you doing here?"), _("This planet belongs to Lord Battleaddict. You will be captured if you land here. The mission failed.") )
      misn.finish(false)

   elseif mem.stage == 5 and spob.cur() == duelpla then -- Report back
      misn.npcAdd("majorTam", _("Major Tam and Captain Leblanc"), fw.portrait_tam, _("Major Tam and Captain Leblanc seem to be waiting for you."))
      misn.npcAdd("majorTam", _("Major Tam and Captain Leblanc"), fw.portrait_leblanc, _("Major Tam and Captain Leblanc seem to be waiting for you."))

   elseif mem.stage == 7 and spob.cur() == duelpla then -- Epilogue
      misn.npcAdd("endMisn", _("Your employers"), fw.portrait_tam, _("Tam and Leblanc are congratulating their general."))
      misn.npcAdd("endMisn", _("Your employers"), fw.portrait_leblanc, _("Tam and Leblanc are congratulating their general."))
      misn.npcAdd("endMisn", _("Your employers"), fw.portrait_klank, _("Tam and Leblanc are congratulating their general."))
   end
end

function hamfresser()
   if (player.pilot():cargoFree() >= bombMass) then
      tk.msg( _("New passengers"), fmt.f(_([["H... hi", you say, waving timidly. "Are you Captain Hamfresser?". The soldier answers "Of course, as it is written on my name tag." Next to his Captain's insignia, and the logo of the Dvaered Space Infantry (a mace with wings), he points to a small label on his chest that reads "Hamfresser". Hamfresser looks at you from top to bottom "You're the private pilot, right? Tell me your ship's dock number, and I'll meet you there. Oh, and please make room for {tonnes} of cargo."
   The captain then gets up, delicately puts his empty glass on the counter, and leaves. While his hair brushes the ceiling, you wonder if {tonnes} are enough to accomodate him. When you arrive at the dock, you see Hamfresser, with five other soldiers and two androids that load a huge and strange machine into your ship. "Hey," you say, "what are you doing with your... your death machine?" Hamfresser approaches and answers at low voice "But, mate, this is not a death machine, It's just a bomb. Or even just a bomblet."]]), {tonnes=fmt.tonnes(bombMass)}) )
      tk.msg( _("New passengers"), _([["Very well," you acquiesce, "do what you have to do." Once the cargo is loaded and the team has taken their places in the cabin, you start to talk with the captain. "And I suppose this bomblet is destined for Battleaddict's Goddard. How are we supposed to put it there? Are we going to pretend it's a gift from the High Command to his granddaughter?". Hamfresser looks at you surprised. "No... that's not what the Major... do you think it could work?" You realize it would take too long to explain that it was a sarcastic comment (assuming that this guy knew what sarcasm is) and simply ask him to explain the major's plan.
   "Last period, we intercepted a message from Battleaddict to a plumber. His cruiser has issues with sewage disposal and he requested an intervention. So, we abducted the plumber and we disguised an EMP bomb as a replacement sewage disposal. We will dock with his ship, plant the bomb, repair the breakdown (so he won't suspect us) and leave. Private Ling here is a Goddard-plumber, so she will lead us." A young and smiling soldier raises her hand, and says "Hi".]]) )
      tk.msg( _("New passengers"), fmt.f(_([[While you wonder whether the plan is insanely brilliant or dead stupid, Hamfresser begins the introductions. "This is Sergeant Nikolov, she is my second in command, this is Private Tronk, from my squad, and Corporal Therus, our medical support. Oh, and the guy in the corner over there is Lieutenant Strafer. He is a pilot from Special Operations. He is here in case we need to switch to plan B." As you ask what plan B is, Hamfresser simply answers, "you don't want to switch to plan B.
   "As usual, Lord Battleaddict's cruiser should be in orbit around {pnt} in {sys}".]]), {pnt=sabotpla, sys=sabotsys}) )
      mem.stage = 1
      hook.enter("enter")
      local c = commodity.new( N_("Bomb"), N_("A gift from the High Command to Lord Battleaddict.") )
      mem.bomblet = misn.cargoAdd( c, bombMass )

      misn.markerRm(mem.mark)
      misn.osdActive(2)
      mem.mark = misn.markerAdd(sabotsys, "low")
      player.takeoff()
   else
      tk.msg(_("Not enough free space"), fmt.f(_("Your ship does not have enough free space. Come back with {tonnes} free."), {tonnes=fmt.tonnes(bombMass)}))
   end
end

function enter()
   -- Spawn Battleaddict and his team
   if mem.stage == 1 and system.cur() == sabotsys then
      pilot.toggleSpawn("FLF", false) -- This helps when testing the mission using the Lua console. Normally, the FLF should be dead.
      pilot.clearSelect("FLF")
      pir.clearPirates()

      warlord = pilot.add( "Dvaered Goddard", "Dvaered", sabotpla, _("Lord Battleaddict"), {naked=true} )
      warlord:control(true)
      warlord:moveto( sabotpla:pos() + vec2.newP(rnd.rnd(1000), rnd.angle()) )
      warlord:memory().formation = "circleLarge"
      warlord:setHilight()
      equipGoddard( warlord, false )

      ps = {}
      for i = 1, 4 do
         ps[i] = pilot.add( "Dvaered Vendetta", "Dvaered", sabotpla )
         ps[i]:setLeader(warlord)
      end
      for i = 1, 2 do
         ps[i+4] = pilot.add( "Dvaered Ancestor", "Dvaered", sabotpla )
         ps[i+4]:setLeader(warlord)
      end
      ps[7] = pilot.add( "Dvaered Phalanx", "Dvaered", sabotpla )
      ps[7]:setLeader(warlord)
      ps[8] = pilot.add( "Dvaered Vigilance", "Dvaered", sabotpla, _("Colonel Hamelsen") )
      ps[8]:setLeader(warlord)
      ps[8]:setNoDeath()

      hook.timer(4.0, "enter1_message")
      hook.timer(0.5, "proximity", {anchor = warlord, radius = 2000, funcname = "meeting", focus = player.pilot()})
      hook.timer(0.5, "proximity", {anchor = warlord, radius = 300, funcname = "killing", focus = player.pilot()})

   elseif mem.stage == 2 then
      hook.timer(2.0, "enter2_message")
      mem.stage = 3
      misn.osdDestroy()
      misn.osdCreate( _("Dvaered Sabotage"), {
         fmt.f(_("Go to {sys}, approach {pnt}, and wait for the Phalanx"), {sys=intsys, pnt=intpla}),
         _("Disable and board the Phalanx"),
         fmt.f(_("Report back on {pnt} in {sys}"), {pnt=duelpla, sys=duelsys})} )
      mem.mark = misn.markerAdd(intsys, "low")

   elseif mem.stage == 3 and system.cur() == intsys then
      hook.timer(0.5, "proximity", {location = intpla:pos(), radius = 1000, funcname = "spawn_phalanx", focus = player.pilot()})

   elseif mem.stage == 6 and system.cur() == duelsys then
      pilot.toggleSpawn(false)
      pilot.clear()

      mypos = duelpla:pos()
      step = 150

      klank = pilot.add( "Dvaered Goddard", "Dvaered", mypos + vec2.new(-step, step/2), _("General Klank"), {naked=true} )
      klank:control(true)
      klank:setFaction( fw.fct_dhc() )
      equipGoddard( klank, true ) -- Klank's superior equipment should ensure victory

      battleaddict = pilot.add( "Dvaered Goddard", "Dvaered", mypos + vec2.new(step, step/2), _("Lord Battleaddict"), {naked=true} )
      battleaddict:control(true)
      battleaddict:setFaction( fw.fct_warlords() )
      equipGoddard( battleaddict, false )

      klank:face(battleaddict)
      battleaddict:face(klank)

      urnus = pilot.add( "Dvaered Vigilance", "Dvaered", mypos + vec2.new(0, 3*step/2), _("Colonel Urnus"), {naked=true} )
      urnus:control(true)
      urnus:face( mypos + vec2.new(0, step/2) )

      tam = pilot.add( "Dvaered Vigilance", "Dvaered", mypos + vec2.new(-2*step, 3*step/2), _("Major Tam") )
      tam:control(true)
      tam:face(battleaddict)

      leblanc = pilot.add( "Dvaered Phalanx", "Dvaered", mypos + vec2.new(-2*step, -step/2), _("Captain Leblanc") )
      leblanc:control(true)
      leblanc:face(battleaddict)

      hamelsen = pilot.add( "Dvaered Vigilance", "Dvaered", mypos + vec2.new(2*step, 3*step/2), _("Colonel Hamelsen") )
      hamelsen:control(true)
      hamelsen:face(klank)

      randguy = pilot.add( "Dvaered Vigilance", "Dvaered", mypos + vec2.new(2*step, -step/2) )
      randguy:control(true)
      randguy:face(klank)

      local pp = player.pilot()
      cinema.on{ gui = true }
      pp:taskClear()
      pp:moveto( mypos + vec2.new(0, -step/2) ) -- To avoid being in the range

      camera.set( mypos + vec2.new(0, step/2), true )

      hook.timer(5.0, "beginDuel")
      hook.timer(15.0, "disableDuel")
      hook.timer(65.0, "fighterDuel")
   end
end

-- Equips a Goddard for a duel, with or without repeating railguns
function equipGoddard( plt, repeating )
   -- TODO switch to equipopt
   plt:outfitAdd("S&K Superheavy Combat Plating")
   plt:outfitAdd("Melendez Mammoth XL Engine")
   plt:outfitAdd("Milspec Orion 9901 Core System")
   plt:outfitAdd("Nanobond Plating", 6)
   plt:outfitAdd("Milspec Impacto-Plastic Coating")
   plt:outfitAdd("Droid Repair Crew",4)
   if repeating then
      plt:outfitAdd("Repeating Railgun", 7)
   else
      plt:outfitAdd("Railgun", 7)
   end
   plt:setHealth(100,100)
   plt:setEnergy(100)
   plt:setFuel(true)
   ai_setup.setup( plt )
end

function enter1_message()
   tk.msg(_("Not far from the goal"), fmt.f(_([[As you finish your jump, Lieutenant Strafer approaches your radar screen "Battleaddict's Goddard should be around {pnt}. I guess he should be adding nanobond plating and repeating railguns everywhere he can by now. There should be a few patrol ships around him that will control our security clearance." Hamfresser gives instructions to the team: "Everyone put your plumber suits on. Nikolov, switch the decoder on so that we can monitor the transmissions of the escort ships. It could tell us if we're detected."]]), {pnt=sabotpla}))
end

function enter2_message()
   tk.msg(_("We're safe now"), fmt.f(_([[Once the ship returns to its normal speed after jumping, Hamfresser says: "Strange, I wouldn't have believed we'd survive this one. Would you, Strafer?" The lieutenant answers "I agree, captain. I guess we've got a good pilot." As you ask them why, then, they were not scared, Hamfresser answers: "Of course we were scared! Who would not be? But we are trained not to show our fear. It tends to distract the pilots."
   A few seconds later, you receive an encoded inter-system message from Major Tam: "Plan A has leaked. Please switch to plan C. Do not jump in {sys} by any means. For your information, the leak is under control and the source has been dealt with." The voice makes a pause and continues: "I really hope this messages catches you before you enter {sys}. Otherwise, may Dvaerius, the patron saint of mace rockets, have mercy on your souls..." "Good old Tamtam," Hamfresser says smiling, "he always worries too much about us."]]), {sys=sabotsys}))
   tk.msg(_("Plan C"), fmt.f(_([["All right, everyone, we're now heading to {pnt} in {sys}. According to our intelligence, there should be a Phalanx from Battleaddict's fleet that will take off from there soon. Its name is 'Gorgon'. It is on its way back from a transport mission. According to the analysts, there should be enough free space in this ship for our bomb. We will disable the ship, neutralize the pilot, and load our material. After that, {player} will report back to the Major on {duel_pnt} and the rest of the team will execute the remainder of the plan. I, or Sergeant Nikolov, will brief you once we're in the Phalanx."]]), {pnt=intpla, sys=intsys, player=player.name(), duel_pnt=duelpla}))
end

-- Battleaddict agrees for the player to approach
function meeting()
   if player_civilian() then
      tk.msg(_("You're controlled"), _([[As you approach the lighter ships that protect the cruiser, a Vendetta hails you. Hamfresser answers "We're Johnson and Jhonson, associate plumbers. We've an appointment with Mr. Battleaddict. It's about a sewage disposal problem." The fighter pilot answers: "It's all right citizen, your transponder code is correct. You may pass." You ease your ship forwards while Hamfresser greets the pilot with an obsequious "Thank you, mister officer."
   "By the way," says Lieutenant Strafer once the communication has been closed, "This cruiser probably has no turreted weapons, in anticipation of the duel, so I would recommend to approach it from the back, just in case."]]))
   else
      tk.msg(_("We told you not to use a combat ship!"), _([[As you approach, Lieutenant Strafer looks at your radar screen. "We are in a combat ship. We told you not to use a combat ship. Now, they are going to attack us! Why did you have to use a combat ship? We'll have to abort the mission now. All because of your bloody combat ship!"]]))
      release_baddies()
      misn.finish(false)
   end
end

-- Battleaddict sees that the player is not a plumber
function killing()
   tk.msg(_("That could have worked"), fmt.f(_([[While on approach, you get a better look at the surface of the cruiser. You see a dozen shuttles transporting material and tools from the planet to the ship. As you get closer, you remark that the cruiser looks like an huge construction site with workers in spacesuits welding nanobond reinforcement plates on the hull. Behind you you hear the chatter of the escort ships, which Hamfresser and his team are anxiously listening to: "Hey, Zog, I'm getting concerned about my daughter. Her teacher told me she was non-violent with her classmates. Do you think I should see a specialist?" "Meh, I don't know, honestly. The new holomovies are to blame. There is always less violence and more love in there. The government should take measures."
   Suddenly, a message makes everyone come to a halt: "So, Colonel, when do we take those fake plumbers out? I look forward to using my shredders a bit!" "Shut up, Corporal!" "Oah, come on, I'm on the encoded channel. The plumbers aren't able to break our code." "But they're NOT plumbers, stupid!"
   Hamfresser looks at you and declares "We're aborting the mission. Get us out of this system, {player}!" Strangely enough, none of the soldiers seem to show any sign of panic.]]), {player=player.name()}))
   release_baddies()
   mem.stage = 2

   misn.osdDestroy()
   misn.osdCreate( _("Dvaered Sabotage"), {_("Jump out. Do NOT land in this system")} )
   misn.markerRm(mem.mark)
end

function release_baddies()
   local fwarlords = fw.fct_warlords()
   warlord:setFaction( fwarlords )
   warlord:control(false)
   for i, j in ipairs(ps) do
      j:setFaction( fwarlords )
   end
end

-- Test civilian ships
function player_civilian()
   local pps = player.pilot():ship()
   local tags = pps:tags()
   local playerclass = pps:class()
   return (playerclass == "Yacht" or playerclass == "Courier" or tags.transport or playerclass == "Armoured Transport")
end

-- Spawn the Phalanx to disable
function spawn_phalanx()
   p = pilot.add( "Dvaered Phalanx", "Dvaered", intpla, _("Gorgon"), {naked=true} )
   p:setFaction(fw.fct_warlords())
   p:setHilight()
   p:control()

   mem.nextsys = lmisn.getNextSystem(system.cur(), sabotsys)
   p:hyperspace( mem.nextsys, true ) -- Go towards Battleaddict's place

   -- TODO switch to equipopt
   p:outfitAdd("S&K Medium Combat Plating")
   p:outfitAdd("Milspec Orion 4801 Core System")
   p:outfitAdd("Tricon Cyclone Engine")
   p:outfitAdd("Turreted Vulcan Gun", 2)
   p:outfitAdd("Mass Driver")
   p:outfitAdd("Vulcan Gun", 2)
   p:outfitAdd("Reactor Class I")
   p:outfitAdd("Medium Cargo Pod", 2)
   p:setHealth(100,100)
   p:setEnergy(100)
   p:setFuel(true)
   ai_setup.setup(p)

   mem.pattacked = hook.pilot( p, "attacked", "phalanx_attacked" )
   mem.pboarded = hook.pilot( p, "board", "phalanx_boarded" )
   hook.pilot( p, "death", "phalanx_died" )
   hook.pilot( p, "jump", "phalanx_safe" )
   hook.pilot( p, "land", "phalanx_safe" )

   mem.stage = 4
   misn.osdActive(2)
   -- TODO: not possible to jump out nor land
end

-- Decide if the Phalanx flees or fight
function phalanx_attacked()
   hook.rm(mem.pattacked)
   if player.pilot():ship():size() > 3 then
      p:taskClear()
      p:comm( _("Just try to catch me, you pirate!") )
      p:runaway(player.pilot())
   else
      p:control(false)
      p:comm( _("You made a very big mistake!") )
   end
end

function phalanx_boarded()
   hook.rm(mem.pboarded)
   tk.msg( _("Boarding"), fmt.f(_([[All the members of the commando unit have put on their battle suits. Hamfresser gives the final orders. "Nikolov, Tronk, and I will enter first and clear the area. Remember, we don't have our usual Dudley combat androids. We're stuck with the two useless plumber bots and the few security droids of {player}'s ship so we'll have to get our hands dirty. Corvettes are typically protected by a few 629 Spitfires and an occasional 711 Grillmeister. That's not very much, but still enough to send the inattentive soldier ad patres."
   When the corvette's airlock falls under Nikolov's circular saw, the captain waves and the small team enters the ship. You hear shots and explosions coming from further and further into the enemy ship. Finally, you hear a laconic message coming from the disabled corvette: "Strafer here, everything went well. We'll now transfer the cargo into the Phalanx... Now that the maneuver is finished, you may leave." Happy to have survived the operation so far, you start your engines and respond "Good luck, folks!" The lieutenant answers "Thanks, citizen, I'm glad to have met you."]]), {player=player.name()}) )
   mem.stage = 5
   misn.cargoRm(mem.bomblet)

   player.unboard() -- Prevent the player form actually boarding the ship
   p:setFaction( fw.fct_dhc() )
   p:control(true)
   p:taskClear()
   p:hyperspace( mem.nextsys )
   p:setFriendly(true) -- It's ours now!

   misn.osdActive(3)
   misn.markerRm(mem.mark)
   mem.mark = misn.markerAdd(duelpla, "low")
end

-- Mission failed: phalanx died
function phalanx_died()
   tk.msg( _("Mission Failed: target destroyed"), _("You were supposed to disable that ship, not to destroy it. How are you supposed to transport the bomb now?") )
   misn.finish(false)
end

-- Mission failed: phalanx escaped
function phalanx_safe()
   tk.msg( _("Mission Failed: target escaped"), _("You were supposed to disable that ship, not to let it escape. How are you supposed to transport the bomb now?") )
   misn.finish(false)
end

function majorTam()
   tk.msg( _("Ready to attend to the show?"), fmt.f(_([[As you sit at the table, Tam starts to speak: "I got a message from Captain Hamfresser. Apparently, everything went according to plan this time. They should have docked with the Goddard, allegedly to add their mission log to the central database. Then they planted the bomb in the plumbing, close to the central unit, and they faked an accident while landing on {pnt}. I guess they should be hiking somewhere on the planet's surface by now, looking for the opportunity to steal an unfortunate civilian's Llama in order to make their trip back.
   "Our boss, General Klank, is ready for the duel. The Captain and I are his duel witnesses, so we should be joining our pageantry ships by now. Oh, and the duel commissioner is someone you already know, Colonel Urnus. In about a period, Lord Battleaddict should arrive, so if you take off soon, you will see the duel."]]), {pnt=sabotpla}) )
   mem.stage = 6

   misn.osdDestroy()
   misn.osdCreate( _("Dvaered Sabotage"), {_("Attend to the duel"), fmt.f(_("Land on {pnt}"), {pnt=duelpla})} )
   misn.markerRm(mem.mark)
end

-- Starts the duel
function beginDuel()
   tk.msg( _("Here we go"), _([[Colonel Urnus's ship broadcasts the message: "I, Colonel Urnus, have been requested by both parties of this duel to be today's commissioner. I hereby solemnly swear, as an officer of the Dvaered Army, to be respectful of our laws and our customs, and I have never worked under the command nor as a commander of either of the generals involved in this duel. I have verified the pedigree of the four witnesses and I can attest they are respectable officers of the Dvaered Army. Lord Battleaddict, General Klank, before proceeding with combat, I must ask you one last time: Are you sure your disagreement cannot be solved by any other means?"
   A formal silence follows the words of the colonel, but soon Battleaddict and Klank respond: "It cannot, Mister Commissioner." Urnus continues: "I am witness to the fact that this duel conforms to the rules established by our ancestors. I have inspected both ships and I attest that I observed no irregularities. Let the fight begin. May the most virtuous one of you survive."]]) )
   klank:taskClear()
   klank:attack(battleaddict)
   klank:setNoDeath() -- Actually it should not be necessary, but...
   battleaddict:taskClear()
   battleaddict:attack(klank)

   urnus:broadcast( _("Let the fight begin!") )
   hook.timer( 1.0, "message", {pilot = tam, msg = _("Come on, boss!")} )
   hook.timer( 2.0, "message", {pilot = leblanc, msg = _("DESTROY HIM!")} )
   hook.timer( 3.0, "message", {pilot = hamelsen, msg = _("You're the best, boss!")} )
   hook.timer( 4.0, "message", {pilot = randguy, msg = _("Yeah!")} )

   hook.pilot( battleaddict, "exploded", "battleaddict_killed" )
end

-- Disables the ships
function disableDuel()
   klank:disable()
   battleaddict:disable()

   -- Explosion and such
   audio.soundPlay( "empexplode" )
   camera.shake()
   hook.timer(1.0, "moreSound1")
   hook.timer(2.0, "moreSound2")

   hook.timer( 2.0, "message", {pilot = tam, msg = _("Damn!")} )
   hook.timer( 4.0, "message", {pilot = leblanc, msg = _("Oooooo...")} )
   hook.timer( 6.0, "message", {pilot = hamelsen, msg = _("What the?")} )
   hook.timer( 8.0, "message", {pilot = randguy, msg = p_("fw01", "Come on!")} )

   hook.timer( 11.0, "message", {pilot = tam, msg = _("Hey, they have put a bomb in the general's ship as well!")} )
   hook.timer( 15.0, "message", {pilot = leblanc, msg = _("The electricians! We've called electricians recently! They planted the bomb!")} )
   hook.timer( 19.0, "message", {pilot = tam, msg = _("Cheaters!")} )
   hook.timer( 23.0, "message", {pilot = hamelsen, msg = _("Cheaters yourselves!")} )

   hook.timer( 28.0, "message", {pilot = klank, msg = _("Hey, Battleaddict, it seems we are both down...")} )
   hook.timer( 32.0, "message", {pilot = battleaddict, msg = _("I still want to kill you!")} )
   hook.timer( 36.0, "message", {pilot = klank, msg = _("So do I.")} )
   hook.timer( 38.0, "message", {pilot = klank, msg = _("Luckily enough, I've got my Vendetta in the fighter bay.")} )
   hook.timer( 42.0, "message", {pilot = battleaddict, msg = _("So do I.")} )
end

function moreSound1()
   audio.soundPlay( "beam_off0" )
end
function moreSound2()
   audio.soundPlay( "hyperspace_powerdown" )
end

-- Fighter duel
function fighterDuel()
   klank2 = pilot.add( "Dvaered Vendetta", "Dvaered", klank:pos(), _("General Klank") )
   klank2:control(true)
   klank2:setFaction( fw.fct_dhc() )
   fw.equipVendettaMace( klank2 ) -- Klank's superior equipment should ensure victory once more

   battleaddict2 = pilot.add( "Dvaered Vendetta", "Dvaered", battleaddict:pos(), _("Lord Battleaddict") )
   battleaddict2:control(true)
   battleaddict2:setFaction( fw.fct_warlords() )

   battleaddict2:broadcast( _("Shall we continue?") )
   hook.timer( 1.0, "message", {pilot = klank2, msg = _("Of course, we shall!")} )
   hook.timer( 1.5, "message", {pilot = tam, msg = _("Come on, boss!")} )
   hook.timer( 2.0, "message", {pilot = leblanc, msg = _("DESTROY HIM!")} )
   hook.timer( 2.5, "message", {pilot = hamelsen, msg = _("You're the best, boss!")} )
   hook.timer( 3.0, "message", {pilot = randguy, msg = _("Yeah!")} )

   -- Prevent both Goddards from colliding with Vendetta's ammo. Set the AI so that they don't get stuck.
   for k,v in ipairs{battleaddict, klank} do
      v:setFaction("Dvaered")
      v:memory().atk = atk_generic --atk_drone
   end

   klank2:setNoDeath() -- Actually it should not be necessary, but...
   klank2:setNoDisable()

   battleaddict2:control()
   battleaddict2:moveto( mypos + vec2.new(step,step/4), false, false ) -- Prevent them from staying on the top of their ships
   battleaddict2:attack(klank2)
   klank2:control()
   klank2:moveto( mypos + vec2.new(-step,step/4), false, false )
   klank2:attack(battleaddict2)
   hook.pilot( battleaddict2, "exploded", "battleaddict_killed" )

   --camera.set(klank2, true)
end

function battleaddict_killed()
   tam:broadcast( _("Aha! In your freaking ugly face!") )
   leblanc:broadcast( _("You're the best, general!") )
   hamelsen:broadcast( _("Oooooo...") )
   randguy:broadcast( _("Nooooo!") )
   urnus:broadcast( _("General Klank won the duel!") )

   hook.timer( 2.0, "everyoneLands" )
   camera.set( nil, true )
   cinema.off()

   mem.stage = 7
   misn.osdActive(2)
end

function everyoneLands()
   local everyone = { klank, klank2, urnus, tam, leblanc, hamelsen, randguy }
   for i, pi in ipairs(everyone) do
      pi:taskClear()
      pi:land(duelpla)
   end
end

-- Epilogue
function endMisn()
   tk.msg( _("A good thing done"), fmt.f(_([[As you approach, the major tells General Klank that you are the private pilot they hired recently. "I see," says the general "so you are one of the people I have to thank for still being alive now." You answer that he apparently would not have needed help if Battleaddict had not cheated as well, and he responds: "Damn fake electricians; I should have suspected something. Anyway, citizen, rest assured that we will need your services again." A group of generals approach and congratulate Klank. He stands up and leaves with them, loudly exchanging dubious pleasantries.
   Major Tam speaks to you: "Apparently, Battleaddict had a commando unit dress like electricians and hide an EMP bomb in the General's Goddard. It exploded during the fight, just like our own bomb. And now both ships have their systems ruined. Well, anyway, thank you for your help, here are {credits} for you!"]]), {credits=fmt.credits(fw.credits_01)}) )
   player.pay(fw.credits_01)
   shiplog.create( "dvaered_military", _("Dvaered Military Coordination"), _("Dvaered") )
   shiplog.append( "dvaered_military", _("Major Tam's superior, General Klank, had a Goddard duel with Lord Battleaddict. You took part in an operation to sabotage Battleaddict's cruiser. Lord Battleaddict sabotaged Klank's cruiser as well, but at the end of the day, General Klank won the duel.") )
   var.push( "loyal2klank", false ) -- This ensures the next mission will be available only once the traitor event is done
   misn.finish(true)
end
