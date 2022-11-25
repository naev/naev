--[[
<?xml version='1.0' encoding='utf8'?>
 <mission name="Dvaered Ballet">
 <unique />
 <priority>2</priority>
 <chance>100</chance>
 <location>None</location>
 <notes>
   <done_evt name="Strafer's Ceremony">Triggers</done_evt>
   <campaign>Frontier Invasion</campaign>
 </notes>
</mission>
 --]]
--[[
-- Dvaered Ballet
-- This is the 6th mission of the Frontier War Dvaered campaign.
-- The player takes part to a funeral ceremony in memory of Lieutenant Strafer.
-- The ceremony turns out to be a Mace Ballet
-- Pilots must perform deeds with only mace rockets
-- Depending of their performance, the player might get rewarded a Dvaered Vendetta
-- (While not having access to Dvaered Restricted planets yet)

   Stages :
   0) Way to Dvaer Prime
   1) Before speaking to anyone
   2) Before the Mace Throw
   3) Mace Throw performed, before landing
   4) Before the Mace Stadion
   5) Mace Stadion performed, before landing
   6) Before the Mace Pankration
   7) Mace Pankration performed
--]]

local fmt = require "format"
local fw = require "common.frontier_war"
require "proximity"
local portrait = require "portrait"

-- Non-persistent state
local annoyers, compHitHook, competitors, followers, joyyesno, leader, score_pankration, score_stadion, score_throw, targets
local checkMace, endPankration, populate_bar, spawnCompetitors, tamStage1, tamStage2, tamStage3 -- Forward-declared functions

-- common hooks
message = fw.message

-- Mission constants
local destpla, destsys = spob.getS("Dvaer Prime")
local radius = 4000
local mace_fail = _("Your only weapons should be Mace rockets: land and speak again with Major Tam.")

-- Joy cries (for the Mace throw)
local joy = { _("Wohoo!"),
        _("One less!"),
        _("Baoum!"),
        _("I'm in such good shape!"),
        _("Top score, here I come!"),
        _("Eat that, you Llama freak!"),
        _("I am so a-Mace-ing!"), -- Yep, I remember the puns I read on discord
        _("Dodge that!"),
        _("I got one right in the a... Hem. The engine..."),
        _("Got you!"),}

function create()
   if not misn.claim(destsys) then
      misn.finish(false)
   end

   tk.msg( _("An invitation"), _([[Your ship receives a holographic message from the Dvaered Army Long-Distance Messaging System. As you press the "Enter" button, the teenage-face of a young Lieutenant Strafer emerges from your holoprojector:
   "I am Private Helmut Strafer and this is my death announcement." A strange smile appears on his face as he continues: "Yep, this is happening. It is my utmost privilege to have been killed in service to the Dvaered Nation, Great House of the Glorious Galactic Empire. You received this recording because you are on the list of people I considered to be my friends. You are invited to my funeral ceremony, which I sincerely hope you will enjoy. As I obviously don't know yet either the circumstances of my death, or the details about the ceremony, all I can do is invite you to find more information in the attached data. Now that I have merged with the void, I would be honoured if my memory helps you remain right, loyal, and strong for the rest of your own lifetime."]]), ("portraits/" .. fw.portrait_strafer) )
   if not tk.yesno( _("An invitation"), _([[You browse the attached data and find out the ceremony will take place around Dvaer Prime. As a pilot, you are invited to take part in a Mace Rocket Ballet in memory of Lieutenant Strafer, and are strongly advised to show up with a fighter and mace launchers.
   Are you going to go there?]]) ) then
      -- TODO should probably add a timer or something or the player will get spammed every time they try to land with this
      misn.finish(false)
   end

   misn.accept()

   misn.osdCreate( _("Dvaered Ballet"), {fmt.f(_("Fly to {pnt}"), {pnt=destpla})} )
   misn.setDesc(_("You are invited to a Mace Rocket Ballet in memory of Lieutenant Strafer."))
   misn.setReward(_("Say goodbye to Lieutenant Strafer"))
   misn.markerAdd(destpla, "low")

   mem.stage = 0
   hook.land("land")
   hook.takeoff("takeoff")
   hook.jumpout("testEscape")
   hook.load("spawnNpcs")
   mem.enterhook = hook.enter("enter")

   mem.portrait_dad = portrait.getMaleMil("Dvaered")
   mem.portrait_wdw = portrait.getFemale()
   mem.portrait_sst = portrait.getFemaleMil("Dvaered")
   mem.portrait_pvt = portrait.getMaleMil("Dvaered")

   -- Prepare storing of total scores of competitors
   mem.score_total = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
   mem.competitors_names = { _("Major Tam"),
                         _("Captain Leblanc"),
                         _("General Klank"),
                         _("Sergeant Strafer"),
                         _("Private Micoult"),
                         _("Sergeant Garcia"),
                         _("Corporal Johnson"),
                         _("Private Ernst"),
                         _("Lieutenant Guo"),
                         player.name(), }
end

local cargo_flowers
local function _flowers()
   if not cargo_flowers then
      cargo_flowers = commodity.new( N_("Flowers"), N_("Pretty flowers."), {gfx_space="flowers.webp"} )
   end
   return cargo_flowers
end

-- Entering (Dvaer)
function enter()
   if system.cur() == destsys then
      hook.rm(mem.enterhook)
      local cflowers = _flowers()
      for i = 1, 100 do -- Flowers
         local pos = destpla:pos() + vec2.newP( rnd.rnd(0,1000), rnd.angle() )
         local vel = vec2.newP( rnd.rnd(0,10), rnd.angle() )
         system.addGatherable( cflowers, 1, pos, vel, 3600 )
      end
      leader = pilot.add("Dvaered Goddard","Dvaered",destpla,_("General Klank"))
      leader:memory().formation = "wedge"
      leader:control()
      leader:moveto( destpla:pos() + vec2.new(0,-500) )

      followers = {}
      followers[1] = pilot.add("Dvaered Vigilance","Dvaered",destpla,_("Major Tam"))
      followers[2] = pilot.add("Dvaered Phalanx","Dvaered",destpla,_("Captain Leblanc"))
      followers[3] = pilot.add("Dvaered Phalanx","Dvaered",destpla,_("Lieutenant Guo"))
      followers[4] = pilot.add("Dvaered Ancestor","Dvaered",destpla,_("Private Micoult"))
      followers[5] = pilot.add("Dvaered Ancestor","Dvaered",destpla,_("Sergeant Garcia"))
      followers[6] = pilot.add("Dvaered Vendetta","Dvaered",destpla,_("Corporal Johnson"))
      followers[7] = pilot.add("Dvaered Vendetta","Dvaered",destpla,_("Private Ernst"))
      for i, p in ipairs(followers) do
         p:setLeader(leader)
      end
      hook.timer(0.5, "proximity", {location = destpla:pos(), radius = 2000, funcname = "introduction", focus = player.pilot()})
   end
end

-- Introduction scene (with flowers everywhere)
function introduction()
   tk.msg( _("The Mace Ballet"), _([[Your sensors detect a group of warships doing maneuvers around Dvaer Prime. When you approach, a Vigilance hails you and you recognize the voice of Major Tam:
   "Greetings, citizen. Are you here for the ceremony? Of course, you are! You see? We have already thrown a few garlands. The funeral parade will soon be over, and after that we will start with the mace ballet. Meet us in the bar of Dvaer Prime."
   Before giving you any chance to ask him what the event is about, Tam closes the communication channel, and your find yourself surrounded by a field of giant blooms of white flowers. You let your mind idly navigate among the bio-engineered plants that have invaded Dvaer's orbit, and think that even if your alliance with the Dvaered has not been very lucrative until now, it has at least rewarded you with this unique view of overgrown flowers aimlessly drifting on the starry background.]]) )
   leader:taskClear()
   leader:land(destpla)
   for _i, p in ipairs(followers) do
      p:control()
      p:land(destpla) -- Actually, fleet AI should not require that, but...
   end
end

-- Landing (on Dv Prime)
function land()
   testEscape()
   local cflowers = _flowers()
   if mem.stage == 5 or mem.stage == 0 then -- Player may have picked up flowers
      player.pilot():cargoRm( cflowers, 100 )
   end

   spawnNpcs() -- This handles people on the bar
end

-- Sorting functions
local function sortTotal(i,j)
   return mem.score_total[i] < mem.score_total[j]
end
local function sortThrow(i,j)
   return score_throw[i] < score_throw[j]
end
local function sortStadion(i,j)
   return score_stadion[i] < score_stadion[j]
end
local function sortPankration(i,j)
   return score_pankration[i] < score_pankration[j]
end

function spawnNpcs()
   if (spob.cur() == destpla) then
      -- First, compute the people's scores
      local iter = {1,2,3,4,5,6,7,8,9,10}
      table.sort( iter, sortTotal )
      mem.totalTitle = _("Total Scores")
      mem.totalString = ""
      for i = 10, 1, -1 do
         mem.totalString = (mem.totalString..mem.competitors_names[iter[i]]..": "..mem.score_total[iter[i]].."\n")
      end

      if mem.stage == 0 then
         mem.npc = misn.npcAdd( "approach", _("Dvaered People"), portrait.getMil( "Dvaered" ), _([[A rather large group is gathered around a few reserved tables. You recognize many faces, among which General Klank, Major Tam, and members of their group. But there are people you do not know as well, mostly military, but also civilians. You can tell they're all there for the ceremony from their black armbands and the slow military music playing on the speakers.]]))
      else
         populate_bar()

         if mem.stage == 1 or mem.stage == 3 or mem.stage == 5 then
            misn.npcAdd("tamCommon", _("Major Tam"), fw.portrait_tam, _("Major Tam is ready to explain the next stage of the ceremony to you."))
         elseif mem.stage == 7 then
            tk.msg( mem.totalTitle, mem.totalString ) -- Ex-aequo always profit the player.
            tk.msg("", fmt.f(_([[While landing, you see the other participants of the ceremony gathered on the dock. Strafer's father, being the master of ceremonies, announces:
   "Congratulations to {name}, who is the great winner of the Mace Ballet! All participants will be rewarded according to their rank."]]), {name=mem.competitors_names[10]}))

            player.outfitAdd("Handbook for Dvaered Good Manners") -- TODO: add lore about this Handbook

            -- Give a reward depending on the rank (10 is highest and 1 lowest)
            local playerRank = fw.elt_inlist( 10, iter )

            if playerRank == 10 then
               tk.msg("",fmt.f(_("You receive a {1} as a reward."), {_("Dvaered Vendetta")}))
               player.shipAdd("Dvaered Vendetta", _("You obtained this ship as a reward from Mace Rocket Ballet."))
            elseif playerRank == 9 then
               tk.msg("",fmt.f(_("You receive a {1} as a reward."), {_("Vendetta")}))
               player.shipAdd("Vendetta", _("You obtained this ship as a reward from Mace Rocket Ballet."))
            elseif playerRank == 8 then
               tk.msg("",fmt.f(_("You receive a {1} and a {2} as a reward."), {_("Tricon Zephyr II Engine"),_("Emergency Shield Booster")}))
               player.outfitAdd("Tricon Zephyr II Engine")
               player.outfitAdd("Emergency Shield Booster")
            elseif playerRank == 7 then
               tk.msg("",fmt.f(_("You receive a {1} and a {2} as a reward."), {_("Milspec Orion 3701 Core System"),_("Shield Capacitor I")}))
               player.outfitAdd("Milspec Orion 3701 Core System")
               player.outfitAdd("Shield Capacitor I")
            elseif playerRank == 6 then
               tk.msg("",fmt.f(_("You receive a {1} as a reward."), {_("S&K Light Combat Plating")}))
               player.outfitAdd("S&K Light Combat Plating")
            elseif playerRank == 5 then
               tk.msg("",_("You receive three Shredders as a reward."))
               player.outfitAdd("Shredder",3)
            elseif playerRank == 4 then
               tk.msg("",fmt.f(_("You receive a {1} as a reward."), {_("Hellburner")}))
               player.outfitAdd("Hellburner")
            elseif playerRank == 3 then
               tk.msg("",fmt.f(_("You receive a {1} as a reward."), {_("Reactor Class I")}))
               player.outfitAdd("Reactor Class I")
            elseif playerRank == 2 then
               tk.msg("",fmt.f(_("You receive a {1} as a reward."), {_("Small Shield Booster")}))
               player.outfitAdd("Small Shield Booster")
            else
               tk.msg("",fmt.f(_("You receive a {1} as a reward."), {_("Shield Capacitor I")}))
               player.outfitAdd("Shield Capacitor I")
            end
            tk.msg(_("Major Tam warns you, and gives you a new task."),_([[After the results have been announced, Major Tam approaches you. He seems to have something important to say: "It was your first ballet, right? You performed very well out there, citizen.
   "Anyway, there is a matter I need to discuss with you: one of my informants told me that ex-Colonel Hamelsen has put a price on your head. Yes. This kind of thing just happens, you know. It seems scary, doesn't it? Actually, many people here already have a price on their head, including me, General Klank, and Captain Leblanc. And one can live very well despite it. The only thing is to be a bit more careful than usual.
   "Since Hamelsen has had so many mercenaries killed under her command, it is unlikely that freelance pilots will jump in and try to kill you. I am pretty sure your attackers will be motivated by more than money. I want to know why they continue to attack us after so many losses. So, your mission will be the following: if some bounty hunter tries to kill you, capture them alive and bring them to me."]]))
            tk.msg(_("Goodbye, Lieutenant Strafer"),_([[Once the participants have collected their rewards, you follow everyone to an arena that has been reserved for the occasion. While cyborg-gladiators slaughter convicted criminals in the fighting ring, Captain Leblanc, as the direct superior to Strafer, gives a speech:
   "How can space be so dark with all the bright blood we have shed up there? How can planets be so majestic with all the ships that have crashed on their surfaces? How can stars be so quiet with all the horrors they have witnessed? If one of you has a good answer, please tell me! Please interrupt me! Please reassure me!
   "For an answer I already have. And it is all but pleasant."]]))
            tk.msg(_("Goodbye, Lieutenant Strafer"),_([["Space, planets, and stars. The reason why they stay unmoved by human horrors is simple and cruel: they do not care. The universe did not require us to exist. It did not even want it. Nevertheless we are here, aimless, clueless, ripping each others' throats to pass the time. It is my fate, as a captain, to lead a group of lost men, all meant to die in this dark and empty universe.
   "But what do we know of emptiness? How can we say in the first place, that the universe is empty? Because our hearts are full. Full of beauty and ugliness. Full of love and hate. Full of desires and despair. Our hearts are full of all these contrary things that can't be found anywhere else in the universe. Space has no compassion for us? Planets don't care for our ships? Stars have no big plan for us? So much the better! Because we, humans, already have made our own big plans!
   "And Helmut Strafer's big plan is named Great House Dvaered."]]))
            tk.msg(_("Goodbye, Lieutenant Strafer"),_([["Great House Dvaered turns scarlet when its citizens spill their blood for it. Great House Dvaered loses a bit of its majesty anytime one of its ships crashes. Great House Dvaered is horrified when it witnesses atrocities.
   "Helmut Strafer dedicated his life to Great House Dvaered, and I have the great honour to speak of him at this occasion. I could tell you how right, loyal, and strong this man was. I could tell you the confidence his very presence next to my ship made me feel. But you already know. So I will only tell you this story: We were flying, side by side, in the Arandon nebula. We had spent hours in there, with the abrasive gases slowly gnawing on our shields. All to find and destroy a single terrorist fighter. I asked Strafer if he thought it was worth it and he gave this simple answer: 'Everything is worth it when it is done for the Dvaered Nation'.
   "In conclusion, if you want to honor the memory of Helmut Strafer, do worthy things: do work for the Nation!"]]))

            shiplog.create( "frontier_war", _("Frontier War"), _("Dvaered") )
            shiplog.append( "frontier_war", _("You took part to a Mace Rocket Ballet in memory of Lieutenant Strafer, and won an astonishing prize. Major Tam warned you that assassins are on your tracks and requested you to capture one of them alive.") )
            misn.finish(true)
         end
      end
   end
end

-- All the cases where the player is not authorized to land nor jump
function testEscape()
   if mem.stage == 2 or mem.stage == 4 or mem.stage == 6 then
      tk.msg( _("You left the system."), _("You were supposed to take part in the competition, not leave the system.") )
      misn.finish(false)
   end
end

-- Approaching the group of soldiers
function approach()
   tk.msg("",fmt.f(_([[You approach Major Tam and the group. "Good day, citizen {player}. I hope you are ready for the mace ballet!" You answer that you have no idea what this ballet is about, but you are always ready when it comes to mace rockets.
   "The mace ballet, also known as mace triathlon, is a series of three events where pilots must honour the memory of their fallen comrade and show their skills. The members of the Nightclaws squadron will take part to the event, along with both of us, General Klank, a few members of Strafer's family, and some of his former comrades before he joined the squadron. The outcome of the competition will determine how Strafer's personal outfits will be distributed. This includes two Vendettas, nice core outfits, weapons, and utilities.
   "Come tell me when you're ready to take off.]]), {player=player.name()}) )
   misn.npcRm(mem.npc)
   mem.stage = 1
   spawnNpcs()
end

-- Add the random people
function populate_bar()
   misn.npcAdd("discussLbl", _("Captain Leblanc"), fw.portrait_leblanc, _("Leblanc is surrounded by her pilots, who somehow exchange jokes about their respective collections of decapitated heads. Their demeanor feels surprisingly relaxed."))
   misn.npcAdd("discussKlk", _("General Klank"), fw.portrait_klank, _("The general is talking to Major Tam."))
   misn.npcAdd("discussNkv", _("Sergeant Nikolov"), fw.portrait_nikolov, _("Nikolov is arm-wrestling half a dozen soldiers. The cyborg sergeant seems to be very cautious in order so as to not harm them."))
   misn.npcAdd("discussHam", _("Captain Hamfresser"), fw.portrait_hamfresser, _("Hamfresser attempts to converse with a group of army technicians, but no one seems to give him any attention. His face seems to reflect not only boredom, but also shame in not being able to fit in among the group."))
   misn.npcAdd("discussWdw", _("Well-dressed woman"), mem.portrait_wdw, _("One of the few civilians around, this woman seems however to fit in with the place. You think that she must be used to hanging out with soldiers."))
   misn.npcAdd("discussDad", _("Retired soldier"), mem.portrait_dad, _("An old captain who seems to have ironed his dress uniform for the occasion is talking to some civilians. His shoulders carry the weight of years spent fighting in space while his face sags from days of anguish over comrades he loved and lost fighting up there."))
   misn.npcAdd("discussSst", _("Sergeant"), mem.portrait_sst, _("This pilot is not a member of Leblanc's squadron, however, she converses with them."))
   misn.npcAdd("discussPvt", _("Technician"), mem.portrait_pvt, _("A military technician encourages his comrades who are arm-wrestling with Nikolov."))
end

-- Discussions
function discussLbl()
   tk.msg( "", _([[Hello, citizen. I am glad to see you were invited. You deserve it. It always bothers me to lose a good pilot like Strafer, but you know, life is like that, when you are a Dvaered. Anyway, we managed to avenge him out there. In addition to the four you neutralized, we got seven of them during the pursuit. Unfortunately, the one in the Schroedinger managed to get away. But this time I doubt they will be able to rebuild their forces soon. You know, ex-Colonel Hamelsen will have a hard time recruiting pilots after such a high loss rate.]]) )
end
function discussKlk()
   tk.msg( "", _([[Good day, citizen. You are the private pilot helping us in our task, right? I have heard that you are doing a fairly good job. Continue and you will get rewarded for your merit, be sure of that!]]) )
end
function discussNkv()
   tk.msg( "", _([[Yeah, he was for sure a good guy. Of course, he was an "educated" man, like the others here, always calling everyone "citizen", and annoying people with "righteousness", "valor" and stuff. But he was one of the few who did not despise us spacemarines, and we could count on him. He certainly will be missed.]]) )
end
function discussHam()
   tk.msg( "", fmt.f(_([[Hey, {player}! Long time no see, huh? How are you doing? I've been stuck at the hospital due to all the damage I took during the last mission. I got a brand new right arm, see? It has the latest bio and cyber enhancements. Targeting abilities have been increased by 0.23 percent, pulling force by 0.26 percent, and pushing by 0.22 percent. But its best feature is that I can now scratch my nose without leaving oil marks on it. Everyone is jealous at the barracks.]]), {player=player.name()}) )
end
function discussWdw()
   tk.msg( "", _([[It feels so strange. I knew this day could come sooner or later, but yet... I can't really figure out how the children and I will live without my husband from now on. It makes me so sad. Do you think that creating the next generation of Dvaered warriors is the sole purpose of marriage?]]) )
end
function discussDad()
   tk.msg( "", _([[The dark sea took so many of my ancestors, my own father, then my wife, and now my elder child. Fate did not have me die in my time, so I guess I am simply meant to stay home, waiting for the rest of my family members to die one after the other.]]) )
end
function discussSst()
   tk.msg( "", _([[You will take part to the mace ballet too? Ah! I can't wait to fight the friends of my big brother!]]) )
end
function discussPvt()
   tk.msg( "", fmt.f(_([["Hello, citizen {player}, how are you?" You ask the soldier how he knows your name and he answers: "Well, I am part of the Nightclaws, and everyone knows you in the squadron. You private pilots aren't used to speaking with technicians, right? This is normal: at each port, you encounter different workers.  It's hard to make friends that way. In our army, the situation is different. We hang out together a lot and are united by hard work and by our faith in the Nation.
   "You know what they say? Joining the Dvaered army is the best way to find your place in the society."]]), {player=player.name()}) )
end

-- Instructions
function tamCommon()
   local c = tk.choice("", _("What do you want to ask Major Tam?"), _("Explain next stage"), _("Display the scores"))
   if c == 1 then
      if mem.stage == 1 then
         tamStage1()
      elseif mem.stage == 3 then
         tamStage2()
      else --if mem.stage == 5 then
         tamStage3()
      end
   else -- Display the scores
      tk.msg(mem.totalTitle, mem.totalString)
   end
end
function tamStage1()
   tk.msg( "", _([[The first event is the "Mace Throw". There is a series of targets (old Llamas) and you have to hit them in the shortest time. The quickest pilot wins. Are you ready? Make sure you only have TeraCom Mace Launchers equipped.]]) )
   mem.stage = 2
end
function tamStage2()
   tk.msg( "", fmt.f(_([[The second event is the "Mace Stadion". Crowns of flowers have been dropped out there. Some junior pilots from the academy patrol the "gather zone" with their mace launchers. They will fire on the competitors. If your shield is disabled, you are eliminated. There are {tonnes} of flowers in total. The pilot who has gathered the most flowers before being eliminated, or when time runs out, wins. It is forbidden to shoot at other competitors. Are you ready? Have you checked you have enough free cargo space?]]), {tonnes=fmt.tonnes(60)}) )
   mem.stage = 4
end
function tamStage3()
   tk.msg( "", fmt.f(_([[The third and last event is the "Mace Pankration". Each competitor must fight against another competitor, and disable their shields. Winners receive 7 points. Your adversary is {name}. Killing is, of course, not allowed. Are you ready? Make sure you only have TeraCom Mace Launchers equipped.]]), {name=mem.competitors_names[5]}) )
   mem.stage = 6
end

-- Take off for the next stage
function takeoff()
   local pos
   mem.center = destpla:pos() + vec2.new(0,radius)

   if mem.stage == 2 then -- Mace Throw: spawn Llamas and competitors.
      -- Check the player only has mace rockets
      if checkMace() then
         misn.osdDestroy()
         misn.osdCreate( _("Dvaered Ballet"), {
            _("Wait for the signal"),
            _("Hit the most targets before your opponents"),
            fmt.f(_("Land on {pnt}"), {pnt=destpla}),
         } )

         score_throw = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
         pilot.toggleSpawn(false)
         pilot.clear()
         spawnCompetitors()
         player.pilot():control()
         player.pilot():face(mem.center)

         local fwarlords = fw.fct_warlords()
         targets = {}
         for i = 1, 30 do
            pos = mem.center + vec2.newP( rnd.rnd(0,radius), rnd.angle() )
            targets[i] = pilot.add( "Llama", fwarlords, pos, _("Target "))
            targets[i]:control()
            targets[i]:setHostile() -- Just in case
            pos = mem.center + vec2.newP( rnd.rnd(0,radius), rnd.angle() )
            targets[i]:moveto( pos, false, false )
            hook.pilot( targets[i], "idle", "targetIdle" )
            hook.pilot( targets[i], "attacked", "targetHit" )
         end
         mem.targetShot = 0

          -- Remember which joy cries have been used  /!\ this should have the same length as joy /!\
         joyyesno = {true,true,true,true,true,true,true,true,true,true}

         -- Timer and messages
         hook.timer( 3.0, "message", {pilot = competitors[1], msg = _("The stars look very different today")} )
         hook.timer( 6.0, "message", {pilot = competitors[2], msg = _("My rockets can't wait to fly for Strafer")} )
         hook.timer( 9.0, "message", {pilot = competitors[4], msg = _("Poor small Llamas!")} )
         hook.timer( 10.0, "startThrow" )
         mem.countdown = 10
         hook.timer( 1.0, "timerIncrement")
         mem.omsg = player.omsgAdd(tostring(mem.countdown), 0, 50)

      else -- Need to land and continue at previous stage
         misn.osdDestroy()
         misn.osdCreate( _("Dvaered Ballet"), {fmt.f(_("Land on {pnt}"), {pnt=destpla})} )
         tk.msg("", mace_fail)
         mem.stage = 1
      end

   elseif mem.stage == 4 then -- Mace Stadion: spawn flowers, competitors and annoyers
      -- No need to check the player only has mace rockets
      misn.osdDestroy()
      misn.osdCreate( _("Dvaered Ballet"), {
         _("Wait for the signal"),
         _("Catch the most flowers possible"),
         fmt.f(_("Land on {pnt}"), {pnt=destpla}),
      } )

      score_stadion = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
      pilot.toggleSpawn(false)
      pilot.clear()
      spawnCompetitors( )
      player.pilot():control()
      player.pilot():face(mem.center)

      for i, p in ipairs(competitors) do
         p:setNoDeath() -- Bouuuh! they r cheating!
         p:memory().gather_range = 4*radius
      end

      local fwarlords = fw.fct_warlords()
      annoyers = {}
      for i = 1, 10 do
         pos = mem.center + vec2.newP( rnd.rnd(0,radius-500), rnd.angle() )
         annoyers[i] = pilot.add( "Dvaered Vendetta", fwarlords, pos, _("Shooter"))
         fw.equipVendettaMace( annoyers[i] )
         annoyers[i]:setSpeedLimit( .0001 )
         annoyers[i]:control()
      end

      local cflowers = _flowers()
      for i = 1, 60 do
         pos = mem.center + vec2.newP( rnd.rnd(0,radius), rnd.angle() )
         system.addGatherable( cflowers, 1, pos, vec2.new(0,0), 3600 )
      end

      mem.playerHitHook = hook.pilot( player.pilot(), "attacked", "playerHitS" ) -- If player has zero shield: eliminated

      -- Timer and messages
      hook.timer( 3.0, "message", {pilot = competitors[2], msg = _("Time to pick up flowers")} )
      hook.timer( 6.0, "message", {pilot = competitors[4], msg = _("You may as well give up now, I'm invincible!")} )
      hook.timer( 9.0, "message", {pilot = competitors[8], msg = _("Flower Power, I'm coming!")} )
      hook.timer( 10.0, "startStadion" )
      mem.countdown = 10
      hook.timer( 1.0, "timerIncrement")
      mem.omsg = player.omsgAdd(tostring(mem.countdown), 0, 50)

   elseif mem.stage == 6 then -- Mace Pankration: competitors and make teams
      -- Check the player only has mace rockets
      if checkMace() then
         misn.osdDestroy()
         misn.osdCreate( _("Dvaered Ballet"), {
            _("Wait for the signal"),
            fmt.f(_("Defeat your opponent {name} (nullify their shield)"), {name=mem.competitors_names[5]}),
            fmt.f(_("Land on {pnt}"), {pnt=destpla}),
         } )

         score_pankration = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
         pilot.toggleSpawn(false)
         pilot.clear()
         spawnCompetitors( )
         player.pilot():control()
         player.pilot():face(mem.center)

         compHitHook = {nil,nil,nil,nil,nil,nil,nil,nil,nil}
         mem.playerHitHook = hook.pilot( player.pilot(), "attacked", "playerHit" )

         -- Mark this one as player's opponent
         competitors[5]:setFaction( fw.fct_warlords() )
         competitors[5]:setHostile()
         competitors[5]:setHilight()
         mem.duelsEnded = 0

         -- Timer and messages
         hook.timer( 3.0, "message", {pilot = competitors[6], msg = _("Hey, Tamtam, take your protein pills and put your helmet on, because I'M COMING FOR YOU!")} )
         hook.timer( 6.0, "message", {pilot = competitors[4], msg = _("I will win for my brother!")} )
         hook.timer( 9.0, "message", {pilot = competitors[5], msg = fmt.f(_("{player}, are you ready for punishment?"), {player=player.name()})} )
         hook.timer( 10.0, "startPankration" )
         mem.countdown = 10
         hook.timer( 1.0, "timerIncrement")
         mem.omsg = player.omsgAdd(tostring(mem.countdown), 0, 50)

      else
         misn.osdDestroy()
         misn.osdCreate( _("Dvaered Ballet"), {fmt.f(_("Land on {pnt}"), {pnt=destpla})} )
         tk.msg("", mace_fail)
         mem.stage = 5
      end
   end
end

-- Test wether the player only has mace rockets
function checkMace()
   local weap = player.pilot():outfitsList("weapon")
   for i, w in ipairs(weap) do
      if w ~= outfit.get("TeraCom Mace Launcher") then
         return false
      end
   end
   return true
end

-- Increment the printing timer
function timerIncrement()
   mem.countdown = mem.countdown - 1
   if mem.countdown == 0 then
      player.omsgChange(mem.omsg, _("Go!"), 3)
   else
      hook.timer( 1.0, "timerIncrement")
      player.omsgChange(mem.omsg, tostring(mem.countdown), 0)
   end
end

-- Increment the time over timer
function timerIncrementT()
   mem.countdown = mem.countdown - 1
   if mem.countdown == 0 then
      player.omsgChange(mem.omsg, _("Time Over!"), 3)
   else
      hook.timer( 1.0, "timerIncrementT")
      player.omsgChange(mem.omsg, tostring(mem.countdown), 0)
   end
end

-- Actually start the Mace Throw
function startThrow()
   hook.timer( 0.5, "dehostilify" )
   player.pilot():control(false)
   for i, p in ipairs(competitors) do
      p:control(false)
   end
   misn.osdActive(2)
end

-- Actually start the Mace Stadion
function startStadion()
   player.pilot():control(false)
   for i, p in ipairs(competitors) do
      p:taskClear()
      hook.pilot( p, "idle", "competitorIdle" )
      p:gather()
      hook.pilot( p, "attacked", "compHitS" )
   end
   for i, p in ipairs(annoyers) do
      p:control(false)
   end
   hook.timer( 60.0, "endStadion" )
   hook.timer( 50.0, "endTimer" ) -- Near-end timer
   misn.osdActive(2)
end

-- Timer for the end of the Stadion
function endTimer()
   mem.countdown = 10
   hook.timer( 1.0, "timerIncrementT" ) -- Near-end timer
   mem.omsg = player.omsgAdd(tostring(mem.countdown), 0, 50)
end

function startPankration()
   player.pilot():control(false)
   -- Make dynamic factions for everyone
   local dynFact = {}
   -- Make people attack each other.
   for i = 1, 9 do
      dynFact[i] = faction.dynAdd( "Warlords", mem.competitors_names[i], mem.competitors_names[i], {ai="dvaered"} )
      competitors[i]:setFaction(dynFact[i])
      competitors[i]:taskClear()
      compHitHook[i] = hook.pilot( competitors[i], "attacked", "compHit" )
      competitors[i]:memory().atk = nil -- Should default to generic, a very aggressive AI
   end
   for i = 1, 4 do
      competitors[i]:attack(competitors[i+5])
      competitors[i+5]:attack(competitors[i])
      dynFact[i]:dynEnemy( dynFact[i+5] )
   end
   competitors[5]:attack(player.pilot())
   misn.osdActive(2)
end

-- One of the competitors is killed
function compDie( _victim, attacker )
   -- This was a bad idea
   if attacker and attacker:withPlayer() then
      tk.msg( _("That was not very smart."), _([[While watching the hull of your opponent's ship collapsing under the impact of your rockets, you suddently remember with horror that this is all just a competition. You think that the Dvaered might be upset at you for this, but then you realize they will probably just kill you instead.]]) )
      faction.get("Dvaered").setPlayerStanding(-100)
      for i, p in ipairs(competitors) do
         p:taskClear()
         p:attack( player.pilot() )
      end
      pilot.toggleSpawn(true)
      misn.finish(false)
   end
end

-- Hack for the ships not to be hostile anymore once shot (Mace Throw)
function dehostilify()
   local mypilots = pilot.get( { faction.get("Independent") } )
   for i, p in ipairs(mypilots) do
      p:setHostile(false)
   end
   hook.timer( 0.5, "dehostilify" )
end

-- Spawn Competitors
function spawnCompetitors( )
   local fdhc = fw.fct_dhc()
   competitors = {} -- tam, leblanc, klank, strafer, caros, micoult, johnson, ernst, guo
   for i = 1, 9 do
      local pos = mem.center + vec2.newP( radius, i*math.pi/5 - math.pi/2 )
      competitors[i] = pilot.add( "Dvaered Vendetta", fdhc, pos, mem.competitors_names[i])
      fw.equipVendettaMace( competitors[i] )
      competitors[i]:memory().Cindex = i -- Store their index
      competitors[i]:setVisible()
      competitors[i]:control()
      competitors[i]:face(mem.center)
      hook.pilot( competitors[i], "death", "compDie" )
   end
end

-- One of the targets is idle (Mace Throw)
function targetIdle( self )
   local pos = mem.center + vec2.newP( rnd.rnd(0,radius), rnd.angle() )
   self:moveto( pos, false, false )
end

-- One of the targets is hit (Mace Throw)
function targetHit( victim, attacker )
   victim:setInvincible()
   victim:taskClear()
   victim:land(destpla)
   victim:setFaction( "Independent" )
   victim:setHostile( false )

   if attacker and attacker:withPlayer() then
      score_throw[10] = score_throw[10] + 1
      mem.score_total[10] = mem.score_total[10] + 1
   else
      local ind = attacker:memory().Cindex
      score_throw[ind] = score_throw[ind] + 1
      mem.score_total[ind] = mem.score_total[ind] + 1
   end

   mem.targetShot = mem.targetShot + 1
   if mem.targetShot >= #targets then
      hook.timer( 3.0, "endThrow" )
   end

   -- pick a joy cry
   local c = rnd.rnd(1,20)
   if c <= #joyyesno then
      if joyyesno[c] then
         attacker:broadcast(joy[c])
         joyyesno[c] = false
      end
   end
end

function endThrow()
   for i, p in ipairs(competitors) do
      p:control()
      p:land(destpla)
   end
   mem.stage = 3
   misn.osdActive(3)

   -- Sort and print top scores
   local iter = {1,2,3,4,5,6,7,8,9,10}
   table.sort( iter, sortThrow )
   local throwTitle = _("Scores for the Mace Throw")
   local throwString = ""
   for i = 10, 1, -1 do
      throwString = (throwString..mem.competitors_names[iter[i]]..": "..score_throw[iter[i]].."\n")
   end
   tk.msg( throwTitle, throwString )
end

-- One of the competitors is Idle (Mace Stadion)
function competitorIdle( self )
   self:gather()
end

-- A competitor is hit during Stadion (problematic if the player is the agressor)
function compHitS( _victim, attacker )
   if attacker and attacker:withPlayer() then
      tk.msg( _("This is not allowed!"), _("You are not supposed to shoot at the other competitors during the Mace Stadion. The Stadion must now be interrupted and you receive 5 penalty points. Land and speak again with Major Tam.") )
      for i, p in ipairs(competitors) do
         p:taskClear()
         p:land( destpla )
      end
      mem.score_total[10] = mem.score_total[10] - 5
      mem.stage = 3 -- Back to previous mem.stage
   end
end

-- Player is hit during Stadion
function playerHitS()
   if player.pilot():health() < 100 then -- Player has lost
      hook.rm(mem.playerHitHook)
      player.pilot():setInvincible()
      player.pilot():control()
      player.pilot():brake()
      tk.msg("",_("You have been eliminated. You won't be allowed to move any more until the time runs out. Your score will be equal to the number of flowers you already collected."))
   end
end

-- End of the Mace Stadion
function endStadion()
   for i, p in ipairs(annoyers) do
      p:control()
   end

   for i, p in ipairs(competitors) do
      p:taskClear()
      p:land(destpla)
   end
   mem.stage = 5
   misn.osdActive(3)

   local cflowers = _flowers()
   for i = 1, 9 do
      score_stadion[i] = competitors[i]:cargoHas(cflowers)
      mem.score_total[i] = mem.score_total[i] + score_stadion[i]
   end
   score_stadion[10] = player.pilot():cargoHas(cflowers)
   mem.score_total[10]   = mem.score_total[10] + score_stadion[10]

   local iter = {1,2,3,4,5,6,7,8,9,10}
   table.sort( iter, sortStadion )
   local stadionTitle = _("Scores for the Mace Stadion")
   local stadionString = ""
   for i = 10, 1, -1 do
      stadionString = (stadionString..mem.competitors_names[iter[i]]..": "..score_stadion[iter[i]].."\n")
   end
   tk.msg( _("Time Over"), _("Land on Dvaer Prime.") )
   tk.msg( stadionTitle, stadionString )

   -- Free the player
   hook.rm(mem.playerHitHook)
   player.pilot():setInvincible(false)
   player.pilot():control(false)
end

-- One of the competitors (or the player) is hit (Mace Pankration)
function playerHit()
   if player.pilot():health() < 100 then -- Player has lost
      hook.rm(mem.playerHitHook)
      hook.rm(compHitHook[5])
      mem.duelsEnded = mem.duelsEnded + 1
      score_pankration[5] = score_pankration[5] + 7
      mem.score_total[5] = mem.score_total[5] + 7
      competitors[5]:taskClear()
      competitors[5]:land(destpla)
      competitors[5]:setHostile(false)
      competitors[5]:setInvincible()
      player.msg(fmt.f(_("{1} won against {2}!"), {mem.competitors_names[5],mem.competitors_names[10]}))
      endPankration()
   end
end
function compHit( victim )
   if victim:health() < 100 then -- Identify who has lost
      local ind = victim:memory().Cindex
      if ind == 5 then -- Player won
         hook.rm(mem.playerHitHook)
         hook.rm(compHitHook[5])
         mem.duelsEnded = mem.duelsEnded + 1
         score_pankration[10] = score_pankration[10] + 7
         mem.score_total[10] = mem.score_total[10] + 7
         competitors[5]:taskClear()
         competitors[5]:land(destpla)
         competitors[5]:setHostile(false)
         competitors[5]:setInvincible()
         player.msg(fmt.f(_("{1} won against {2}!"), {mem.competitors_names[10],mem.competitors_names[5]}))
         endPankration()
      elseif ind < 5 then
         hook.rm(compHitHook[ind+5])
         hook.rm(compHitHook[ind])
         mem.duelsEnded = mem.duelsEnded + 1
         score_pankration[ind+5] = score_pankration[ind+5] + 7
         mem.score_total[ind+5] = mem.score_total[ind+5] + 7
         competitors[ind]:taskClear()
         competitors[ind]:land(destpla)
         competitors[ind+5]:taskClear()
         competitors[ind+5]:land(destpla)
         competitors[ind+5]:setInvincible()
         competitors[ind]:setInvincible()
         player.msg(fmt.f(_("{1} won against {2}!"), {mem.competitors_names[ind+5],mem.competitors_names[ind]}))
         endPankration()
      else
         hook.rm(compHitHook[ind-5])
         hook.rm(compHitHook[ind])
         mem.duelsEnded = mem.duelsEnded + 1
         score_pankration[ind-5] = score_pankration[ind-5] + 7
         mem.score_total[ind-5] = mem.score_total[ind-5] + 7
         competitors[ind]:taskClear()
         competitors[ind]:land(destpla)
         competitors[ind-5]:taskClear()
         competitors[ind-5]:land(destpla)
         competitors[ind]:setInvincible()
         competitors[ind-5]:setInvincible()
         player.msg(fmt.f(_("{1} won against {2}!"), {mem.competitors_names[ind-5],mem.competitors_names[ind]}))
         endPankration()
      end
   end
end

-- See if all duels are over
function endPankration()
   if mem.duelsEnded >= 5 then
      hook.timer(1.0, "endPankrationT")
   end
end
function endPankrationT()
   mem.stage = 7
   misn.osdActive(3)

   local iter = {1,2,3,4,5,6,7,8,9,10}
   table.sort( iter, sortPankration )
   local pankrationTitle = _("Scores for the Mace Pankration")
   local pankrationString = ""
   for i = 10, 1, -1 do
      pankrationString = (pankrationString..mem.competitors_names[iter[i]]..": "..score_pankration[iter[i]].."\n")
   end
   tk.msg( pankrationTitle, pankrationString )
end
