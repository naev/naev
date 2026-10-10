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
   1) First try.
   2) Fleeing first time.
   3) Second try
   4) Fight with the Phalanx
   5) Way back
   6) Watch the duel
   7) Final landing
--]]

local atk_generic = require "ai.core.attack.generic"
local lmisn = require "lmisn"
require "proximity"
local vn = require "vn"
local fw = require "common.frontier_war"
local fmt = require "format"
local pir = require "common.pirate"
local vntk = require "vntk"
local cinema = require "cinema"
local ai_setup = require "ai.core.setup"
local equipopt = require "equipopt"
local portrait = require "portrait"
local sfx = require "luaspfx.sfx"
local love_shaders = require "love_shaders"

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

   vn.clear()
   vn.scene()
   local tamVN = vn.newCharacter( fw.vn_char( fw.tam ) )
   local doaccept = false
   vn.transition()
   
   tamVN(fmt.f(_([["Hello, citizen {player}. You remember Lord Battleaddict, the old warlord who tried to kill us twice? I have good news: with a few other members of the Space Force, we've devised a way to make him regret what he did, and we need a civilian pilot, like you. Are you in?"]]), {player=player.name()}))
   vn.menu{
      {_([["Yes"]]), "accept"},
      {_([["No"]]), "dontcare"},
   }
   
   vn.label("dontcare")
   tamVN(_([["Alight, citizen, see you later, then."]]))
   vn.func( function () doaccept = false end )
   vn.done()
   
   vn.label("accept")
   tamVN(_([["I knew you would accept! Here is the situation:
   "The general I am working for, General Klank, is in charge of… hem… in charge of a crucial operation the High Command wants to carry out. This operation will involve troops of the High Command, but also Warlords, including Battleaddict. The problem is that General Klank and Lord Battleaddict disagree on everything about this plan. As a consequence, they are going to have a Goddard duel, which is usually how two important Dvaered generals settle deep disagreements."]]))
   tamVN(_([["The problem is that Battleaddict's plan is far too stupid. It would weaken the Dvaered Nation in the long run and leave us at the mercy of all the other nations around us. We can't afford to show any signs of weakness, or they will attack us and impose their iniquitous and obsolete political systems on our citizenry."]]))
   tamVN(_([["You don't know, citizen, all the dreadful enemies who are waiting in the shadows, their hearts filled with hatred against House Dvaered. Sometimes I look at the star-filled night sky and I wonder. I wonder why the Dvaered Nation has to be the only threatened islet of justice and compassion in this… in this Sea of Darkness."]]))
   tamVN(fmt.f(_([["Hey, citizen! But I have good news! We won't fall to the Barbarian hordes! Because I myself, Major Archibald Tam, I have a plan. We will make sure that Lord Battleaddict loses his duel. Please note, however, that if the very existence of House Dvaered was not threatened, we would never allow ourselves to interfere in a honourable duel between two respectable gentlemen. Go to {pnt} in {sys} and meet Captain Hamfresser. His portrait is attached in the data I will give you. He will explain the details. It is very important that you use a civilian ship that can transport at least {tonnes} of cargo."]]), {pnt=hampla, sys=hamsys, tonnes=fmt.tonnes(bombMass)}))
   vn.func( function () doaccept = true end )
   vn.done()
   vn.run()

   -- Test acceptance
   if not doaccept then misn.finish(false) end
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
      vntk.msg( _("What are you doing here?"), _("This planet belongs to Lord Battleaddict. You will be captured if you land here. The mission failed.") )
      misn.finish(false)
      
   elseif mem.stage == 4 then -- The player landed somewhere instead of attacking the Phalanx
      vntk.msg( _("What are you doing here?"), _("You were supposed to intercept a Phalanx. The mission failed.") )
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
   if (player.fleetCargoMissionFree() >= bombMass) then
   
      local askname = _([["Hello, are you Captain Hamfresser?"]])
      local look = _([[Look at him more closely]])
   
      vn.clear()
      vn.scene()
      local hamfresserVN = vn.newCharacter( fw.vn_char( fw.hamfresser ) )
      vn.transition()
      
      vn.menu{
         {askname, "askname"},
         {fmt.f(_([["My name is {player}, I am pleased to meet you."]]),{player=player.name()}), "introduce"},
         {look, "look"},
      }
      
      vn.label("askname")
      hamfresserVN(_([["Of course, as it is written on my name tag."]]))
      vn.na(_([[Next to his Captain's insignia, and the logo of the Dvaered Space Infantry (a mace with wings), he points to a small label on his chest that reads "Hamfresser"]]))
      vn.jump("moveon")
      
      vn.label("introduce")
      hamfresserVN(_([["For real? You are pleased to meet me. Nobody ever said that to me! It is so kind of you!"]]))
      vn.menu{
         {askname, "askname"},
         {look, "look"},
      }
      
      vn.label("look")
      vn.na(_([[Next to his Captain's insignia, and the logo of the Dvaered Space Infantry (a mace with wings), you see small label on his chest that reads "Hamfresser". Convinced this cyborg is the rigt person, you approach him.]]))
      
      vn.label("moveon")
      hamfresserVN(fmt.f(_([["You're the private pilot, right? Tell me your ship's dock number, and I'll meet you there. Oh, and please make room for {tonnes} of cargo."]]),{tonnes=fmt.tonnes(bombMass)}))
      vn.na(_([[The captain gets up, delicately puts his empty glass on the counter, and leaves. When you arrive at the dock, you see Hamfresser, with five other soldiers and two androids that load a huge and strange machine into your ship.]]))
      vn.menu{
         {_([[Let them proceed]]), "letproceed"},
         {_([["What is this?"]]), "what"},
         {_([["What are you doing with your death machine?"]]), "deathmachine"},
      }
      
      vn.label("deathmachine")
      hamfresserVN(_([["But, mate, this is not a death machine, It's just a bomb. Or even just a bomblet."]]))
      vn.jump("letproceed")
      
      vn.label("what")
      hamfresserVN(_([["Don't worry, mate. It's allright, we are just embarking a bomb in your ship."]]))
      
      vn.label("letproceed")
      vn.na(_([[Once the cargo is loaded, the team take their places in the cabin.]]))
      hamfresserVN(_([["The bomb is destined for Battleaddict's Goddard. But the tricky part is to actually plant it there."]]))
      vn.menu{
         {_([["How are we supposed to do that?"]]), "explain"},
         {_([["Are we going to pretend it's a gift from the High Command to Battleaddict's clownfish?"]]), "clownfish"},
      }
      
      vn.label("clownfish")
      hamfresserVN(_([["No… that's not what the Major… do you think it could work?"]]))
      vn.menu{
         {_([["Of course. Warlords love when people give gifts to their clownfish."]]), "ofcourse"},
         {_([["That was sarcasm, genius."]]), "sarcasm"},
         {_([[Say nothing]]), "explain"},
      }
      
      vn.label("ofcourse")
      vn.na(_([[Hamfresser looks confused. He looks at another soldier questioningly.]]))
      hamfresserVN(_([["Oh… Well, Lieutenant Strafer, what do you think? Should we ask the Major if this idea there is better?"]]))
      vn.na(_([[Lieutenant Strafer approaches with a stern look.]]))
      local straferVN = vn.newCharacter( fw.vn_char( fw.strafer, { pos="left" } ) )
      straferVN(fmt.f(_([["Look, citizen {player}, you should really stop making fun of the Captain. Hamfresser usually kills disrespectful people, and the only reason why you're still alive is that he didn't understand you were bullying him. So please stop that or I'll tell him you don't respect him."]]),{player=player.name()}))
      vn.disappear(straferVN)
      vn.jump("explain")
      
      vn.label("sarcasm")
      hamfresserVN(_([["Oh no, I'm far from being a genius. You know, many parts of my brain that are not linked to combat and space infantry activities have been removed by surgery or are atrophied."]]))
      
      vn.label("explain")
      hamfresserVN(_([["Anyways. Last period, we intercepted a message from Battleaddict to a plumber. His cruiser has issues with sewage disposal, and he requested an intervention. So, we abducted the plumber, and we disguised an EMP bomb as a replacement sewage disposal. We will dock with his ship, plant the bomb, repair the breakdown (so he won't suspect us) and leave. Private Ling here is a Goddard-plumber, so she will lead us."]]))
      vn.na(_([[A young and smiling soldier raises her hand, and says "Hi".]]))
      vn.menu{
         {_([["OMG! This plan is brillant!"]]), "brillant"},
         {_([["OMG! This plan is dead stupid!"]]), "deadstupid"},
         {_([[Say nothing]]), "introduce"},
      }
      
      vn.label("brillant")
      hamfresserVN(_([["I know. It was an idea of Major Tam. He is for sure the best mastermind I have worked with since I've been in the black ops business."]]))
      vn.jump("introduce")
      
      vn.label("deadstupid")
      hamfresserVN(_([["Do you think so? Well… I guess the best way to check if you're right is to try it out and see if we all die."]]))
      
      vn.label("introduce")
      vn.na(_([[The members of the commando introduce themselves.]]))
      vn.move(hamfresserVN,"right")
      local nikolovVN = vn.newCharacter( fw.vn_char( fw.nikolov, { pos="left" } ) )
      nikolovVN(_([["I am Sergeant Nikolov, the squad's second in command. Nice to meet you, citizen."]]))
      vn.disappear(nikolovVN)
      local therusVN = vn.newCharacter( fw.vn_char( fw.therus, { pos="left" } ) )
      therusVN(_([["My name is Corporal Therus. I am our medical support."]]))
      vn.disappear(therusVN)
      local tronkVN = vn.newCharacter( fw.vn_char( fw.tronk, { pos="left" } ) )
      tronkVN(_([["My name is Private Tronk."]]))
      vn.disappear(tronkVN)
      local straferVN = vn.newCharacter( fw.vn_char( fw.strafer, { pos="left" } ) )
      straferVN(_([["I am Lieutenant Strafer. I am a pilot, and I am here in case we need to switch to plan B."]]))
      vn.menu{
         {_([["What is plan B?"]]), "planB"},
         {_([[Say nothing]]), "moveon2"},
      }
      
      vn.label("planB")
      straferVN(_([["You don't want to switch to plan B."]]))
      
      vn.label("moveon2")
      vn.disappear(straferVN)
      hamfresserVN(fmt.f(_([["As usual, Lord Battleaddict's cruiser should be in orbit around {pnt} in {sys}. I propose we leave at once. Ah, and a last point: for safety reasons, please make sure our fuel tanks are not empty when we enter {sys}, just in case we have to leave the system rapidly."]]), {pnt=sabotpla, sys=sabotsys}) )

      vn.done()
      vn.run()

      mem.stage = 1
      hook.enter("enter")
      local c = commodity.new( N_("Bomb"), N_("A gift from the High Command to Lord Battleaddict.") )
      mem.bomblet = misn.cargoAdd( c, bombMass )

      misn.markerRm(mem.mark)
      misn.osdActive(2)
      mem.mark = misn.markerAdd(sabotsys, "low")
      player.takeoff()
   else
      vntk.msg(_("Not enough free space"), fmt.f(_("Your ship does not have enough free space. Come back with {tonnes} free."), {tonnes=fmt.tonnes(bombMass)}))
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
      warlord:moveto( sabotpla:pos() + vec2.newP(rnd.rnd(0,1000), rnd.angle()) )
      warlord:memory().formation = "circleLarge"
      warlord:setHilight()
      warlord:setNoDeath()
      warlord:setNoDisable()
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
   plt:outfitAdd("S&K War Plating",2)
   plt:outfitAdd("Melendez Mammoth Engine",2)
   plt:outfitAdd("Milspec Orion 8601 Core System",2)
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
   vn.clear()
   vn.scene()
   local straferVN = vn.newCharacter( fw.vn_char( fw.strafer, {pos="left"} ) )
   local hamfresserVN = vn.newCharacter( fw.vn_char( fw.hamfresser, {pos="right"} ) )
   vn.transition()
   straferVN(fmt.f(_([["Battleaddict's Goddard should be around {pnt}. I guess he should be adding nanobond plating and repeating railguns everywhere he can by now. There should be a few patrol ships around him that will control our security clearance."]]), {pnt=sabotpla}))
   hamfresserVN(_([["Everyone put your plumber suits on. Nikolov, switch the decoder on so that we can monitor the transmissions of the escort ships. It could tell us if we're detected."]]))
   vn.done()
   vn.run()
end

function enter2_message()
   vn.clear()
   vn.scene()
   local straferVN = vn.newCharacter( fw.vn_char( fw.strafer, {pos="left"} ) )
   local hamfresserVN = vn.newCharacter( fw.vn_char( fw.hamfresser, {pos="right"} ) )
   vn.transition()
   hamfresserVN(_([["Strange, I wouldn't have believed we'd survive this one. Would you, Strafer?"]]))
   straferVN(_([["I agree, captain. I guess we've got a good pilot."]]))
   vn.na(_([[You receive an encoded inter-system message from Major Tam]]))
   local tamVN = vn.newCharacter( fw.vn_char( fw.tam, { shader=love_shaders.hologram() } ) )
   tamVN(fmt.f(_([["Plan A has leaked. Please switch to plan C. Do not jump in {sys} by any means. For your information, the leak is under control and the source has been dealt with."]]),{sys=sabotsys}))
   tamVN(fmt.f(_([["I really hope this message catches you before you enter {sys}. Otherwise, may Dvaerius, the patron saint of mace rockets, have mercy on your souls…"]]),{sys=sabotsys}))
   vn.disappear(tamVN)
   hamfresserVN(_([["Good old Tamtam, he always worries too much about us."]]))
   hamfresserVN(fmt.f(_([["All right, everyone, we're now heading to {pnt} in {sys}. According to our intelligence, there should be a Phalanx from Battleaddict's fleet that will take off from there soon. Its name is 'Gorgon'. It is on its way back from a transport mission. According to the analysts, there should be enough free space in this ship for our bomb. We will disable the ship, neutralize the pilot, and load our material. After that, {player} will report back to the Major on {duel_pnt} and the rest of the team will execute the remainder of the plan. I, or Sergeant Nikolov, will brief you once we're in the Phalanx."]]), {pnt=intpla, sys=intsys, player=player.name(), duel_pnt=duelpla}))
   vn.done()
   vn.run()
end

-- Battleaddict agrees for the player to approach
function meeting()
   if player_civilian() then
      vn.clear()
      vn.scene()
      local straferVN = vn.newCharacter( fw.vn_char( fw.strafer, {pos="farleft"} ) )
      local hamfresserVN = vn.newCharacter( fw.vn_char( fw.hamfresser, {pos="farright"} ) )
      local vendettaVN = vn.newCharacter( _("Vendetta pilot"), { image=portrait.getFullPath("dvaered/dv_military_f2"), shader=love_shaders.hologram() } )
      vn.transition()
      vn.na(_([[A Vendetta opens a communication channel with you.]]))
      hamfresserVN(_([["We're Johnson and Jhonson, associate plumbers. We've an appointment with Mr. Battleaddict. It's about a sewage disposal problem."]]))
      vendettaVN(_([["It's all right citizen, your transponder code is correct. You may pass."]]))
      hamfresserVN(_([["Thank you, mister officer."]]))
      vn.disappear(vendettaVN)
      straferVN(_([["By the way, this cruiser probably has no turreted weapons, in anticipation of the duel, so I would recommend to approach it from the back, just in case."]]))
      vn.done()
      vn.run()
   else
      vn.clear()
      vn.scene()
      local straferVN = vn.newCharacter( fw.vn_char( fw.strafer ) )
      vn.transition()
      vn.na(_([[You realize, but a bit late, that you were supposed to fly a civilian ship.]]))
      straferVN(_([["We are in a combat ship. We told you not to use a combat ship. Now, they are going to attack us! Why did you have to use a combat ship? We'll have to abort the mission now. All because of your bloody combat ship!"]]))
      vn.done()
      vn.run()

      release_baddies()
      misn.finish(false)
   end
end

-- Battleaddict sees that the player is not a plumber
function killing()
   vn.clear()
   vn.scene()
   local pilot1VN = vn.newCharacter( _("First pilot"), { image=portrait.getFullPath("dvaered/dv_military_f2"), shader=love_shaders.hologram(), pos="left" } )
   local pilot2VN = vn.newCharacter( _("Second pilot"), { image=portrait.getFullPath("dvaered/dv_military_m2"), shader=love_shaders.hologram(), pos="right" } )
   vn.transition()
   vn.na(_([[While on approach, you get a better look at the surface of the cruiser. You see a dozen shuttles transporting material and tools from the planet to the ship. As you get closer, you remark that the cruiser looks like a huge construction site with workers in spacesuits welding nanobond reinforcement plates on the hull.]]))
   vn.na(_([[Hamfresser and his team are anxiously listening to the chatter of the escort ships.]]))
   pilot1VN(_([["Hey, Zog, I'm getting concerned about my daughter. Her teacher told me she was non-violent with her classmates. Do you think I should see a specialist?"]]))
   pilot2VN(_([["Meh, I don't know, honestly. The new holomovies are to blame. There is always less violence and more love in there. The government should take measures."]]))
   pilot1VN(_([["Speaking of taking measures, Colonel, when do we take those fake plumbers out? I look forward to using my guns a bit!"]]))
   vn.disappear(pilot2VN)
   local hamelsenVN = vn.newCharacter( fw.vn_char( fw.hamelsen, { shader=love_shaders.hologram() } ) )
   hamelsenVN(_([["Shut up, Corporal!"]]))
   pilot1VN(_([["Oah, come on, I'm on the encoded channel. Plumbers aren't able to break our code."]]))
   hamelsenVN(_([["But they're NOT plumbers, stupid!"]]))
   vn.disappear(pilot1VN)
   vn.disappear(hamelsenVN)
   local hamfresserVN = vn.newCharacter( fw.vn_char( fw.hamfresser ) )
   hamfresserVN(fmt.f(_([["We're aborting the mission. Get us out of this system, {player}!"]]),{player=player.name()}))
   vn.done()
   vn.run()

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

   equipopt.dvaered( p, {
      prefer = { ["Medium Cargo Pod"] = 100 },
      outfits_add = { "Medium Cargo Pod" },
      max_same_stru = 2,
   } )
   p:setEnergy(100)
   p:setFuel(true)

   mem.pattacked = hook.pilot( p, "attacked", "phalanx_attacked" )
   mem.pboarded = hook.pilot( p, "board", "phalanx_boarded" )
   hook.pilot( p, "death", "phalanx_died" )
   mem.pjump = hook.pilot( p, "jump", "phalanx_safe" )
   mem.pland = hook.pilot( p, "land", "phalanx_safe" )

   mem.stage = 4
   misn.osdActive(2)
   
   mem.jumpout = hook.jumpout("jumpoutStage4")
end

-- The player jumps out instead of intercepting the Phalank
function jumpoutStage4()
   vntk.msg( _("Why are you jumping out?"), _("You were supposed to intercept a Phalanx, not to run away. The mission failed.") )
   misn.finish(false)
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
   hook.rm(mem.pjump)
   hook.rm(mem.pland)
   hook.rm(mem.jumpout)
   
   vn.clear()
   vn.scene()
   local hamfresserVN = vn.newCharacter( fw.vn_char( fw.hamfresser ) )
   vn.transition()
   hamfresserVN(fmt.f(_([["Allright folks. Now it's out turn to finally do something useful. Nikolov, Tronk, and I will enter first and clear the area. Remember, we don't have our usual Dudley combat androids. We're stuck with the two useless plumber bots and the few security droids of {player}'s ship so we'll have to get our hands dirty. Corvettes are typically protected by a few 629 Spitfires and an occasional 711 Grillmeister. That's not very much, but still enough to send the inattentive soldier ad patres."]]),{player=player.name()}))
   vn.na(_([[When the corvette's airlock falls under Nikolov's circular saw, the captain waves and the small team enters the ship. You hear shots and explosions coming from further and further into the enemy ship. Finally, the disabled corvette opens a communication channel with you.]]))
   vn.disappear(hamfresserVN)
   local straferVN = vn.newCharacter( fw.vn_char( fw.strafer, { shader=love_shaders.hologram() } ) )
   straferVN(_([["Strafer here, everything went well. We'll now transfer the cargo into the Phalanx… Now that the manoeuvre is finished, you may leave."]]))
   vn.menu{
         {_([[Say nothing]]), "proceed"},
         {_([["Good luck, folks!"]]), "luck"},
         {_([["Good riddance, freaks!"]]), "riddance"},
      }
   
   vn.label("luck")
   straferVN(_([["Thanks, citizen, I'm glad to have met you."]]))
   vn.jump("proceed")
   
   vn.label("riddance")
   straferVN(_([["Whatever. One day I'll give you my handbook for good manners. You look like you need it more than I do."]]))
   
   vn.label("proceed")
   vn.done()
   vn.run()

   mem.stage = 5
   misn.cargoRm(mem.bomblet)
   local c = commodity.new( N_("Bomb"), N_("A gift from the High Command to Lord Battleaddict.") )
   p:cargoAdd(c,bombMass) -- Just in case the player scans the Phalanx

   player.unboard() -- Prevent the player form actually boarding the ship
   p:setFaction( fw.fct_dhc() )
   p:control(true)
   p:taskClear()
   p:hyperspace( mem.nextsys )
   p:setFriendly(true) -- It's ours now!
   p:setHealth( nil, nil, 0 ) -- Re-activate the ship

   misn.osdActive(3)
   misn.markerRm(mem.mark)
   mem.mark = misn.markerAdd(duelpla, "low")
end

-- Mission failed: phalanx died
function phalanx_died()
   vntk.msg( _("Mission Failed: target destroyed"), _("You were supposed to disable that ship, not to destroy it. How are you supposed to transport the bomb now?") )
   misn.finish(false)
end

-- Mission failed: phalanx escaped
function phalanx_safe()
   vntk.msg( _("Mission Failed: target escaped"), _("You were supposed to disable that ship, not to let it escape. How are you supposed to transport the bomb now?") )
   misn.finish(false)
end

function majorTam()
   vn.clear()
   vn.scene()
   local tamVN = vn.newCharacter( fw.vn_char( fw.tam ) )
   vn.transition()
   tamVN(fmt.f(_([["I got a message from Captain Hamfresser. Apparently, everything went according to plan this time. They have docked with the Goddard, allegedly to add their mission log to the central database. Then they planted the bomb in the plumbing, close to the central unit, and they faked an accident while landing on {pnt}. I guess they should be hiking somewhere on the planet's surface by now, looking for the opportunity to steal an unfortunate civilian's Llama in order to make their trip back."]]), {pnt=sabotpla}))
   tamVN(_([["Our boss, General Klank, is ready for the duel. The Captain and I are his duel witnesses, so we should be joining our pageantry ships by now. Oh, and the duel commissioner is someone you already know, Colonel Urnus. In about a period, Lord Battleaddict should arrive, so if you take off soon, you will see the duel."]]))
   vn.done()
   vn.run()

   mem.stage = 6
   misn.osdDestroy()
   misn.osdCreate( _("Dvaered Sabotage"), {_("Attend to the duel"), fmt.f(_("Land on {pnt}"), {pnt=duelpla})} )
   misn.markerRm(mem.mark)
end

-- Starts the duel
function beginDuel()
   vn.clear()
   vn.scene()
   --TODO: Urnus should probably have his own character shared with the anti-FLF campaign
   local urnusVN = vn.newCharacter( _("Colonel Urnus"), { image=portrait.getFullPath("dvaered/dv_military_m3"), shader=love_shaders.hologram() } )
   vn.transition()
   urnusVN(_([["I, Colonel Urnus, have been requested by both parties of this duel to be today's commissioner. I hereby solemnly swear, as an officer of the Dvaered Army, to be respectful of our laws and our customs, and I have never worked under the command nor as a commander of either of the generals involved in this duel. I have verified the pedigree of the four witnesses and I can attest they are respectable officers of the Dvaered Army."]]))
   urnusVN(_([["Lord Battleaddict, General Klank, before proceeding with combat, I must ask you one last time: Are you sure your disagreement cannot be solved by any other means?"]]))
   local klankVN = vn.newCharacter( fw.vn_char( fw.klank, {shader=love_shaders.hologram(), pos="left"} ) )
   klankVN(_("It cannot, Mister Commissioner."))
   urnusVN(_([["I am witness to the fact that this duel conforms to the rules established by our ancestors. I have inspected both ships and I attest that I observed no irregularities. Let the fight begin. May the most virtuous one of you survive."]]))
   vn.done()
   vn.run()
   
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
   klank:setDisable()
   klank:setNoBoard()
   battleaddict:setDisable()
   battleaddict:setNoBoard()

   -- Explosion and such
   sfx( true, nil, audiodata.new("snd/sounds/empexplode") )
   camera.shake()
   hook.timer(1.0, "moreSound1")
   hook.timer(2.0, "moreSound2")

   hook.timer( 2.0, "message", {pilot = tam, msg = _("Damn!")} )
   hook.timer( 4.0, "message", {pilot = leblanc, msg = _("Oooooo…")} )
   hook.timer( 6.0, "message", {pilot = hamelsen, msg = _("What the?")} )
   hook.timer( 8.0, "message", {pilot = randguy, msg = p_("fw01", "Come on!")} )

   hook.timer( 11.0, "message", {pilot = tam, msg = _("Hey, they have put a bomb in the general's ship as well!")} )
   hook.timer( 15.0, "message", {pilot = leblanc, msg = _("The electricians! We've called electricians recently! They planted the bomb!")} )
   hook.timer( 19.0, "message", {pilot = tam, msg = _("Cheaters!")} )
   hook.timer( 23.0, "message", {pilot = hamelsen, msg = _("Cheaters yourselves!")} )

   hook.timer( 28.0, "message", {pilot = klank, msg = _("Hey, Battleaddict, it seems we are both down…")} )
   hook.timer( 32.0, "message", {pilot = battleaddict, msg = _("I still want to kill you!")} )
   hook.timer( 36.0, "message", {pilot = klank, msg = _("So do I.")} )
   hook.timer( 38.0, "message", {pilot = klank, msg = _("Luckily enough, I've got my Vendetta in the fighter bay.")} )
   hook.timer( 42.0, "message", {pilot = battleaddict, msg = _("So do I.")} )
end

function moreSound1()
   sfx( true, nil, audiodata.new("snd/sounds/beam_off0") )
end
function moreSound2()
   sfx( true, nil, audiodata.new("snd/sounds/hyperspace_powerdown") )
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
   hamelsen:broadcast( _("Oooooo…") )
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
   vn.clear()
   vn.scene()
   local tamVN = vn.newCharacter( fw.vn_char( fw.tam, {pos="left"} ) )
   local klankVN = vn.newCharacter( fw.vn_char( fw.klank, {pos="right"} ) )
   vn.transition()
   tamVN(fmt.f(_([["General, allow me to introduce you to {player}, the private pilot I hired for you-know-what."]]),{player=player.name()}))
   klankVN(_([["I see, so you are one of the people I have to thank for still being alive now."]]))
   vn.menu{
         {_([[Say nothing]]), "proceed"},
         {_([["You apparently would not have needed help if Battleaddict had not cheated, General."]]), "flatter"},
         {_([["Yeah, you were lucky to have me in your team. I'm the best in this business."]]), "brag"},
      }
      
   vn.label("brag")
   klankVN(_([["Ah! Ah! Ah! There are so many people who are the best in this business nowadays! You private pilots are so funny to work with!"]]))
   vn.jump("proceed")
      
   vn.label("flatter")
   klankVN(_([["Damn fake electricians; I should have suspected something. Anyway, citizen, rest assured that we will need your services again."]]))
   
   vn.label("proceed")
   vn.na(_([[A group of generals approach and congratulate Klank. He stands up and leaves with them, loudly exchanging dubious pleasantries.]]))
   vn.disappear(klankVN)
   vn.move( tamVN, "center" )
   tamVN(fmt.f(_([["Apparently, Battleaddict had a commando unit dress like electricians and hide an EMP bomb in the General's Goddard. It exploded during the fight, just like our own bomb. And now both ships have their systems ruined. Well, anyway, thank you for your help, here are {credits} for you!"]]), {credits="#g"..fmt.credits(fw.credits_01).."#0"}))
   
   vn.done()
   vn.run()

   player.pay(fw.credits_01)
   shiplog.create( "dvaered_military", _("Dvaered Military Coordination"), _("Dvaered") )
   shiplog.append( "dvaered_military", _("Major Tam's superior, General Klank, had a Goddard duel with Lord Battleaddict. You took part in an operation to sabotage Battleaddict's cruiser. Lord Battleaddict sabotaged Klank's cruiser as well, but at the end of the day, General Klank won the duel.") )
   var.push( "loyal2klank", false ) -- This ensures the next mission will be available only once the traitor event is done
   misn.finish(true)
end
