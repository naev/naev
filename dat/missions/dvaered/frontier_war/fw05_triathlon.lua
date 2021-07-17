--[[
<?xml version='1.0' encoding='utf8'?>
 <mission name="Dvaered Ballet">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>2</priority>
   <chance>100</chance>
   <location>None</location>
  </avail>
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

require "missions/dvaered/frontier_war/fw_common"
require "proximity"
local portrait = require "portrait"

invite_text = {}
invite_title = _("An invitation")
invite_text[1] = _([[Your communication channel informs you that your ship has recieved an holographic message through the Dvaered Army Long-Distance Messaging System. As you press the "Enter" button, the face of a teenage version of Lieutenant Strafer emerges from your holoprojector:
   "I am Private Helmut Strafer and this is my death announcement." A strange smile appears on his face as he continues: "Yep, this is happening. It is my ultmost privilege to have been killed in service to the Dvaered Nation, Great House of the Glorious Galactic Empire. You recieve this recording because you are on the list of people I considered to be my friends, and that are invited to my funeral ceremony, which I sincerely hope you will enjoy. As I obviously don't know yet either the circumstances of my death, or the details about the ceremony, all I can do is to invite you to find more informations in the attached data. Now that I have merged with the void, I would be honoured if my memory would be part of the things that help you remain right, loyal and strong for the rest of your own lifetime."]])
invite_text[2] = _([[You browse the attached folder and find out that the ceremony will take place around Dvaer Prime. As a pilot, you are invited to take part to a Mace Rocket ballet in memory of Lieutenant Strafer, and are strongly advised to show up with a fighter and mace launchers.
   Are you going to go there?]])

title_space = _("The Mace Ballet")
text_space = _([[Your sensors detect a group of warships doing maneuvers around Dvaer Prime. When you approach, a Vigilance hails you and you recognize the voice of Major Tam:
   "Greetings, citizen. Are you here for the ceremony? Of course, you are! You see? We have already thrown a few garlands. The funeral parade will soon be over, and after that we will start with the mace ballet itself. Meet us in the bar of Dvaer Prime."
   Before giving you any chance to ask him what the event is about, Tam closes the communication channel, and your find yourself surrounded by a field of giant crowns of white flowers. You let your mind idly navigate among the bio-engineered plants that have invaded the orbit, and think that even if your alliance with the Dvaered has not been very lucrative until now, it has at least rewarded your eyes with this unique view of overgrown flowers aimlessly drifting on the constellated background.]])

title_npc = _("Dvaered People")
text_npc = _([[A rather large group is gathered around a few reserved tables. You recognize many faces, among which General Klank, Major Tam and the members of their group. But there are people you do not know as well, mostly military, but also civilians. You can tell they're all there for the ceremony from their black armbands and the slow military music broadcasted by the speakers.]])

text_approach = _([[You approach the group, and get close to Major Tam. "Good day, citizen %s. I hope you are ready for the mace ballet!" You answer that you have no idea what this ballet is about, but you are always ready when it comes to mace rockets.
   "The mace ballet, also known as mace triathlon, is a series of three events where pilots must honour the memory of their fallen comrade and show their skills. The members of the Nightclaws squadron will take part to the event, along with both of us, General Klank, a few members of Strafer's family, and some of his former comrades before he joined the squadron. From the outcome of the competition will depend how Strafer's personnal outfits will be distributed. This includes two Vendettas, nice core outfits, weapons and utilities.
   "Come at me when you're ready to take off.]])


tam_name = _("Major Tam")
ham_name = _("Captain Hamfresser")
leb_name = _("Captain Leblanc")
klk_name = _("General Klank")
nkv_name = _("Sergeant Nikolov")
wdw_name = _("Well-dressed woman")
dad_name = _("Retired soldier")
sst_name = _("Sergeant")
pvt_name = _("Technician")

tam_desc = _("Major Tam is ready to explain the next stage of the ceremony to you.")
ham_desc = _("Hamfresser stays behind a group of army technicians. He bows towards them, probably attempting to take part to the conversation, but no one seems to give him any attention. His face seems to reflect not only boredom, but also shame not to be able to fit among the group. He swings his unused cybernetic arms around his hips.")
leb_desc = _("Leblanc is surrounded by her pilots, who somehow exchange jokes about their respective collections of chopped heads. The ambiance feels surprisingly relaxed.")
klk_desc = _("The general is talking to Major Tam.")
nkv_desc = _("Nikolov is arm-wrestling half a dozen of soldiers. The cyborg sergeant seems to be very cautious in order not to harm them.")
wdw_desc = _("One of the rare civilians around, this woman seems however to fit in the place. You think that she must be used to hang out with soldiers.")
dad_desc = _("An old captain who seems to have ironed his pageantry uniform for the occasion is talking to some civilians. His shoulders carry the weight of years spent fighting in space while his face is bent over by days of anguish for comrades he loved and lost fighting up there.")
sst_desc = _("This pilot is not a member of Leblanc's squadron, however, she discusses with them.")
pvt_desc = _("A military technician encourages his comrades who are arm-wrestling with Nikolov.")

text_hamfresser = _([[Hey, %s! Long time no see, huh? How do you do? I've been stuck at the hospital lately because of all the damage taken during last mission. I got a brand new right arm, you see? With the latest bio and cyber enhancements. Targeting abilities have been increased by 0.23 percent, pulling force by 0.26 percent and pushing by 0.22 percent. But its best feature is that I can now scratch my nose without putting oil marks on it. Everyone is jealous at the barracks.]])
text_leblanc = _([[Hello, citizen. I am glad to see you have been invited as well. you deserve it. It always annoys me to lose a good pilot like Strafer, but you know, life is like that, when you are a Dvaered. Anyway, you know, we managed to avenge him, out there. In addition to the four you neutralized, we got seven of them during the pursuit. Unfortunately, the one in the Schroedinger managed to get out, but this time, I doubt they will be able to reconstruct their forces soon. You know, the ex-colonel Hamelsen will have a hard time recruiting pilots with such a high loss rate.]])
text_klank = _([[Good day, citizen. You are the private pilot helping us in our task, right? I have heard that you are doing fairly good job. Continue and you will get rewared for your merit, be sure of that!]])
text_nikolov = _([[Yeah, he was for sure a good guy. Of course, he was an "educated" man, like the others here, always calling everyone "citizen", and annoying people with "righteousness", "valor" and stuff. But he was one of the few who did not despise us spacemarines, and we could count on him. He certainly will be missed.]])
text_widow = _([[It feels so strange. I knew this day could come sooner or later, but yet... I can't really figure out how I and the children will live without my husband from now on. It makes me so sad. Do you think that creating the next generation of Dvaered warriors is the sole purpose of wedding?]])
text_dad = _([[The dark sea took so many of my ancestors, my own father, then my wife, and now my elder child. Fate did not make me die in my time, so I guess I am simply meant to stay home, waiting for the rest of my family members to die one after the other.]])
text_sister = _([[You will take part to the mace ballet too? Ah! I can't wait to fight the friends of my big brother!]])
text_technician = _([["Hello, citizen %s, how do you do?" You ask the soldier how he knows your name and he answers: "Well, I am part of the Nightclaws squadron, and everyone knows you in the squadron. You private pilots aren't used to speaking with technicians, right? This is normal: at each stop, you have different ones, this is not suitable to making friends. But in our army, the situation is different, and we hang out much more together, united by hard work and by our faith in the Nation.
   "You know what they say? Joining the Dvaered army is the best way to find your place in the society."]])

text_tam = _("What do you want to ask Major Tam?")
tam_stage = _("Explain next stage")
tam_score = _("Display the scores")

text_stage = {}
text_stage[1] = _([[The first event consists in the "Mace Throw". There is a series of targets (old Llamas) and you have to hit them in the shortest time. The quickest pilot wins. Are you ready? Have you checked you are only equipped with Unicorp Mace Launchers?]])
text_stage[2] = _([[The second event consists in the "Mace Stadion". Crowns of flowers have been dropped out there, and some junior pilots from the academy camp in the gather zone with their mace launchers. They will aim at the competitors, and if your shield is disabled, you are eliminated. There are %d tons of flowers in total. The pilot who has gathered the most flowers before being eliminated, or when time runs out, wins. It is forbidden to shoot at other competitors. Are you ready? Have you checked you have enough free cargo space?]])
text_stage[3] = _([[The third and last event consists of the "Mace Pankration". Each competitor must fight against an other competitor, and disable their shield. Winners recieve 7 points, and your adversary is %s. Killing is of course not allowed. Are you ready? Have you checked you are only equipped with Unicorp Mace Launchers?]])

end_title = _("Goodbye, Lieutenant Strafer")
end_text1 = _([[Once participants have collected their rewards, you are invited to follow everyone in a shuttle that heads towards an arena that has been reserved for the occasion. While cyborg-gladiators slaughter convicted criminals in the fighting zone, Captain Leblanc, as the direct superior to Strafer, gives a speech:
   "How can space be so dark with all the bright blood we have shed up there? How can planets be so majestic with all the ships that have crashed on their faces? How can stars be so quiet with all the horrors they have witnessed? If one of you has a pleasant answer, please tell me! Please interrupt me! Please reassure me!
   "For an answer I already have. And it is all but pleasant."]])
end_text2 = _([["Space, planets and stars. The reason why they stay untouched by human horror is simple and cruel: they do not care. The universe did not require us to exist. It did not even want it. And nevertheless we are here, aimless, clueless, ripping each other's throat to beguile the time. It is my fate, as a captain, to lead a group of lost men, all meant to die in this dark and empty universe.
   "But what do we know of emptiness? Why can we say in the first place, that the universe is empty? It's because our heart, on the opposite, is full. Full of beauty and ugliness, full of love and hate, full of desires and despair. Our heart is full of all these contrary things that can't be found anywhere else in the universe. Space has no compassion for us? Planets don't care for our ships? Stars have no big plan for us? So much the better! Because we, humans, already have made our own big plans!
   "And Helmut Strafer's big plan is named Great House Dvaered."]])
end_text3 = _([["Great House Dvaered turns scarlet when its citizen spill their blood for it, Great House Dvaered loses a bit of its majesty anytime one of its ships crashes, Great House Dvaered is horrified when it witnesses atrocities.
   "Helmut Strafer dedicated his life to Great House Dvaered, and I have had the priceless honour to make way with him at this occasion. I could tell you how right, loyal and strong this man was. I could tell you the trust his very presence next to my ship made me feel. But you already know. So I will only tell you that story: We were flying, side by side, in the Arandon nebula. We had spent hours in there, with the abrasive gases slowly nibbling our shields, to finally find and destroy a single terrorist fighter. I asked Strafer if he thought it was worth it and he got this simple answer: 'Everything is worth it when it is done for the Dvaered Nation'.
   "In conclusion, if you want to honor the memory of Helmut Strafer as well as it deserves it, please do worthy things: please do work for the Nation!"]])


reward_text0 = _([[While landing, you see the other participants of the ceremony gathered on the dock. Strafer's father, being the master of ceremony, announces:
   "Congratulations to %s, who is the great winner of the Mace Ballet! All participants will be rewarded depending to their rank."]])
reward_text1 = _("You recieve a %s as a reward.")
reward_text2 = _("You recieve a %s and a %s as a reward.")
reward_text3 = _("You recieve three Shredders as a reward.")

warn_title = _("Major Tam warns you, and gives you a new task.")
warn_text1 = _([[After the results have been announced, Major Tam gets close to you. He seems to have something important to say: "It was your first ballet, right? You performed very well out there, citizen.
   "Anyway, there is a matter I need to discuss with you: one of my informants told me that the ex-colonel Hamelsen has put a price on your head. Yes. This kind of thing just happens, you know. It looks scary, doesn't it? Actually, many people here already have a price on their head, including me, General Klank and Captain Leblanc. And one can live very well with it. The only thing is to be a bit more careful than usual.
   "As Hamelsen has gotten so many mercenaries killed under her command, it is in fact rather unlikely that unrelated pilots jump in and try to kill you. I am pretty sure that the ones that will attack you are motivated by more than money. And I want to know what makes them continue to attack us after having recieved so many losses. So your mission will be the following: if some bounty hunter tries to kill you, catch them alive and bring them to me."]])

hit_msg = _("You have been eliminated. You won't be allowed to move any more until the time runs out. Your score will be equal to the number of flowers you already collected.")
cheater_title = _("This is not allowed!")
cheater_text = _("You are not supposed to shoot at the other competitors durnig the Mace Stadion. The Stadion has to be interrupted and you recieve 5 points of penalty. Land and speak again with Major Tam.")
killer_title = _("That was not very smart.")
killer_text = _([[While contemplating the hull of your opponent's ship being dislocated under the impact of your rockets, you suddently remember with horror that this is all just a competition. You think that the Dvaered might probably blame you for that, but then you realize that they will probably simply kill you instead.]])
escape_title = _("You left the system.")
escape_text = _("You were supposed to take part in the competition, not leave the system.")
mace_fail = _("Your only weapons should be Mace rockets: land and speak again with Major Tam.")

timermsg = "%s"
pankrationWinner = _("%s won against %s!")

misn_desc = _("You are invited to a Mace Rocket Ballet in memory of Lieutenant Strafer.")
misn_reward = _("Say goodbye to Lieutenant Strafer")
log_text = _("You took part to a Mace Rocket Ballet in memory of Lieutenant Strafer, and won an atonishing prize. Major Tam warned you that hitmen are on your tracks and requested you to catch one of them alive.")

osd_title = _("Dvaered Ballet")
osd_text1 = _("Fly to %s")
osd_text2 = _("Hit the most targets before your opponents")
osd_text3 = _("Catch the most flowers possible")
osd_text4 = _("Defeat your opponent %s (nullify their shield)")
osd_text5 = _("Land on %s")
osd_text6 = _("Wait for the signal")

-- Pointless chatter
chat_00 = _("The stars look very different today")
chat_01 = _("My rockets can't wait to fly for Strafer")
chat_02 = _("Poor small Llamas!")

chat_10 = _("Time to pick up flowers")
chat_11 = _("You may as well give up now, I'm invincible!")
chat_12 = _("Flower Power, I'm coming!")

chat_20 = _("Hey, Tamtam, take your protein pills and put your helmet on, because I'M COMING FOR YOU!")
chat_21 = _("I will win for my brother!")
chat_22 = _("%s, are you ready for the punishment?")

-- Joy cries (for the Mace throw)
joy = { _("Wohoo!"),
        _("One less!"),
        _("Baoum!"),
        _("I'm in such a good shape!"),
        _("Top score, I'm coming!"),
        _("Eat that, you Llama freak!"),
        _("I am so a-Mace-ing!"), -- Yep, I remember the puns I read on discord
        _("Dodge that!"),
        _("I got one right in the a... Hem. The engine..."),
        _("Got you!"),}

function create()
   destpla, destsys = planet.get("Dvaer Prime")

   if not misn.claim(destsys) then
      misn.finish(false)
   end

   tk.msg( invite_title, invite_text[1], ("portraits/"..portrait_strafer) )
   if not tk.yesno( invite_title, invite_text[2] ) then
      misn.finish(false)
   end

   misn.accept()

   misn.osdCreate( osd_title, {osd_text1:format(destpla:name())} )
   misn.setDesc(misn_desc)
   misn.setReward(misn_reward)
   misn.markerAdd(destsys, "low")

   stage = 0
   hook.land("land")
   hook.takeoff("takeoff")
   hook.jumpout("testEscape")
   hook.load("spawnNpcs")
   enterhook = hook.enter("enter")

   portrait_dad = portrait.getMaleMil("Dvaered")
   portrait_wdw = portrait.getFemale()
   portrait_sst = portrait.getFemaleMil("Dvaered")
   portrait_pvt = portrait.getMaleMil("Dvaered")

   -- Prepare storing of total scores of competitors
   score_total = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
   score_total["__save"] = true
   competitors_names = { _("Major Tam"),
                         _("Captain Leblanc"),
                         _("General Klank"),
                         _("Sergeant Strafer"),
                         _("Private Micoult"),
                         _("Sergeant Garcia"),
                         _("Corporal Johnson"),
                         _("Private Ernst"),
                         _("Lieutenant Guo"),
                         player.name(), }
   competitors_names["__save"] = true

   radius = 4000
end

-- Entering (Dvaer)
function enter()
   if system.cur() == destsys then
      hook.rm(enterhook)
      for i = 1, 100 do -- Flowers
         local pos = destpla:pos() + vec2.newP( rnd.rnd(0,1000), rnd.rnd(0,360) )
         local vel = vec2.newP( rnd.rnd(0,10), rnd.rnd(0,360) )
         system.addGatherable( "Flowers", 1, pos, vel, 3600 )
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
      hook.timer(500, "proximity", {location = destpla:pos(), radius = 2000, funcname = "introduction", focus = player.pilot()})
   end
end

-- Introduction scene (with flowers everywhere)
function introduction()
   tk.msg( title_space, text_space )
   leader:taskClear()
   leader:land(destpla)
   for i, p in ipairs(followers) do
      p:control()
      p:land(destpla) -- Actually, fleet AI should not require that, but...
   end
end

-- Landing (on Dv Prime)
function land()
   testEscape()
   if stage == 5 or stage == 0 then -- Player may have picked up flowers
      player.pilot():cargoRm( "Flowers", 100 )
   end

   spawnNpcs() -- This handles people on the bar
end

function spawnNpcs()
   if (planet.cur() == destpla) then

      -- First, compute the people's scores
      local iter = {1,2,3,4,5,6,7,8,9,10}
      table.sort( iter, sortTotal )
      totalTitle = _("Total Scores")
      totalString = ""
      for i = 10, 1, -1 do
         totalString = (totalString..competitors_names[iter[i]]..": "..score_total[iter[i]].."\n")
      end

      if stage == 0 then
         npc = misn.npcAdd( "approach", title_npc, portrait.getMil( "Dvaered" ), text_npc)
      else
         populate_bar()

         if stage == 1 or stage == 3 or stage == 5 then
            tam = misn.npcAdd("tamCommon", tam_name, portrait_tam, tam_desc)
         elseif stage == 7 then
            tk.msg( totalTitle, totalString ) -- Ex-aequo always profit the player.
            tk.msg("",reward_text0:format(competitors_names[10]))

            player.addOutfit("Handbook for Dvaered Good Manners") -- TODO: add lore about this Handbook

            -- Give a reward depending on the rank (10 is highest and 1 lowest)
            playerRank = elt_inlist( 10, iter )
            if playerRank == 10 then
               tk.msg("",reward_text1:format(_("Dvaered Vendetta")))
               player.addShip("Dvaered Vendetta")
            elseif playerRank == 9 then
               tk.msg("",reward_text1:format(_("Vendetta")))
               player.addShip("Vendetta")
            elseif playerRank == 8 then
               tk.msg("",reward_text2:format(_("Tricon Zephyr II Engine"),_("Emergency Shield Booster")))
               player.addOutfit("Tricon Zephyr II Engine")
               player.addOutfit("Emergency Shield Booster")
            elseif playerRank == 7 then
               tk.msg("",reward_text2:format(_("Milspec Orion 3701 Core System"),_("Shield Capacitor I")))
               player.addOutfit("Milspec Orion 3701 Core System")
               player.addOutfit("Shield Capacitor I")
            elseif playerRank == 6 then
               tk.msg("",reward_text1:format(_("S&K Light Combat Plating")))
               player.addOutfit("S&K Light Combat Plating")
            elseif playerRank == 5 then
               tk.msg("",reward_text3)
               player.addOutfit("Shredder",3)
            elseif playerRank == 4 then
               tk.msg("",reward_text1:format(_("Hellburner")))
               player.addOutfit("Hellburner")
            elseif playerRank == 3 then
               tk.msg("",reward_text1:format(_("Reactor Class I")))
               player.addOutfit("Reactor Class I")
            elseif playerRank == 2 then
               tk.msg("",reward_text1:format(_("Small Shield Booster")))
               player.addOutfit("Small Shield Booster")
            else
               tk.msg("",reward_text1:format(_("Shield Capacitor I")))
               player.addOutfit("Shield Capacitor I")
            end
            tk.msg(warn_title,warn_text1)
            tk.msg(end_title,end_text1)
            tk.msg(end_title,end_text2)
            tk.msg(end_title,end_text3)

            shiplog.create( "frontier_war", _("Frontier War"), _("Dvaered") )
            shiplog.append( "frontier_war", log_text )
            misn.finish(true)
         end
      end
   end
end

-- All the cases where the player is not authorized to land nor jump
function testEscape()
   if stage == 2 or stage == 4 or stage == 6 then
      tk.msg( escape_title, escape_text )
      misn.finish(false)
   end
end

-- Approaching the group of soldiers
function approach()
   tk.msg("",text_approach:format(player.name()) )
   misn.npcRm(npc)
   stage = 1
   spawnNpcs()
end

-- Add the random people
function populate_bar()
   lbl = misn.npcAdd("discussLbl", leb_name, portrait_leblanc, leb_desc)
   klk = misn.npcAdd("discussKlk", klk_name, portrait_klank, klk_desc)
   nkv = misn.npcAdd("discussNkv", nkv_name, portrait_nikolov, nkv_desc)
   ham = misn.npcAdd("discussHam", ham_name, portrait_hamfresser, ham_desc)
   wdw = misn.npcAdd("discussWdw", wdw_name, portrait_wdw, wdw_desc)
   dad = misn.npcAdd("discussDad", dad_name, portrait_dad, dad_desc)
   sst = misn.npcAdd("discussSst", sst_name, portrait_sst, sst_desc)
   pvt = misn.npcAdd("discussPvt", pvt_name, portrait_pvt, pvt_desc)
end

-- Discussions
function discussLbl()
   tk.msg( "", text_leblanc )
end
function discussKlk()
   tk.msg( "", text_klank )
end
function discussNkv()
   tk.msg( "", text_nikolov )
end
function discussHam()
   tk.msg( "", text_hamfresser:format(player.name()) )
end
function discussWdw()
   tk.msg( "", text_widow )
end
function discussDad()
   tk.msg( "", text_dad )
end
function discussSst()
   tk.msg( "", text_sister )
end
function discussPvt()
   tk.msg( "", text_technician:format(player.name()) )
end

-- Instructions
function tamCommon()
   local c = tk.choice("", text_tam, tam_stage, tam_score)
   if c == 1 then
      if stage == 1 then
         tamStage1()
      elseif stage == 3 then
         tamStage2()
      else --if stage == 5 then
         tamStage3()
      end
   else -- Display the scores
      tk.msg(totalTitle, totalString)
   end
end
function tamStage1()
   tk.msg( "", text_stage[1] )
   stage = 2
end
function tamStage2()
   tk.msg( "", text_stage[2]:format(60) )
   stage = 4
end
function tamStage3()
   tk.msg( "", text_stage[3]:format(competitors_names[5]) )
   stage = 6
end

-- Take off for the next stage
function takeoff()
   center = destpla:pos() + vec2.new(0,radius)

   if stage == 2 then -- Mace Throw: spawn Llamas and competitors.
      -- Check the player only has mace rockets
      if checkMace() then
         misn.osdDestroy()
         misn.osdCreate( osd_title, {osd_text6,osd_text2,osd_text5:format(destpla:name())} )

         score_throw = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
         pilot.toggleSpawn(false)
         pilot.clear()
         spawnCompetitors()
         player.pilot():control()
         player.pilot():face(center)

         targets = {}
         for i = 1, 30 do
            pos = center + vec2.newP( rnd.rnd(0,radius), rnd.rnd(0,360) )
            targets[i] = pilot.add( "Llama", "Warlords", pos, _("Target "))
            targets[i]:control()
            targets[i]:setHostile() -- Just in case
            pos = center + vec2.newP( rnd.rnd(0,radius), rnd.rnd(0,360) )
            targets[i]:moveto( pos, false, false )
            hook.pilot( targets[i], "idle", "targetIdle" )
            hook.pilot( targets[i], "attacked", "targetHit" )
         end
         targetShot = 0

          -- Remember which joy cries have been used  /!\ this should have the same length as joy /!\
         joyyesno = {true,true,true,true,true,true,true,true,true,true}

         -- Timer and messages
         hook.timer( 3000, "message", {pilot = competitors[1], msg = chat_00} )
         hook.timer( 6000, "message", {pilot = competitors[2], msg = chat_01} )
         hook.timer( 9000, "message", {pilot = competitors[4], msg = chat_02} )
         hook.timer( 10000, "startThrow" )
         countdown = 10
         hook.timer( 1000, "timerIncrement")
         omsg = player.omsgAdd(timermsg:format(countdown), 0, 50)

      else -- Need to land and continue at previous stage
         misn.osdDestroy()
         misn.osdCreate( osd_title, {osd_text5:format(destpla:name())} )
         tk.msg("", mace_fail)
         stage = 1
      end

   elseif stage == 4 then -- Mace Stadion: spawn flowers, competitors and annoyers
      -- No need to check the player only has mace rockets
      misn.osdDestroy()
      misn.osdCreate( osd_title, {osd_text6,osd_text3,osd_text5:format(destpla:name())} )

      score_stadion = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
      pilot.toggleSpawn(false)
      pilot.clear()
      spawnCompetitors( )
      player.pilot():control()
      player.pilot():face(center)

      for i, p in ipairs(competitors) do
         p:setNoDeath() -- Bouuuh! they r cheating!
         p:memory().gather_range = 4*radius
      end

      annoyers = {}
      for i = 1, 10 do
         pos = center + vec2.newP( rnd.rnd(0,radius-500), rnd.rnd(0,360) )
         annoyers[i] = pilot.add( "Dvaered Vendetta", "Warlords", pos, _("Shooter"))
         equipVendettaMace( annoyers[i] )
         annoyers[i]:setSpeedLimit( .0001 )
         annoyers[i]:control()
      end

      for i = 1, 60 do
         pos = center + vec2.newP( rnd.rnd(0,radius), rnd.rnd(0,360) )
         system.addGatherable( "Flowers", 1, pos, vec2.new(0,0), 3600 )
      end

      playerHitHook = hook.pilot( player.pilot(), "attacked", "playerHitS" ) -- If player has zero shield: eliminated

      -- Timer and messages
      hook.timer( 3000, "message", {pilot = competitors[2], msg = chat_10} )
      hook.timer( 6000, "message", {pilot = competitors[4], msg = chat_11} )
      hook.timer( 9000, "message", {pilot = competitors[8], msg = chat_12} )
      hook.timer( 10000, "startStadion" )
      countdown = 10
      hook.timer( 1000, "timerIncrement")
      omsg = player.omsgAdd(timermsg:format(countdown), 0, 50)

   elseif stage == 6 then -- Mace Pankration: competitors and make teams
      -- Check the player only has mace rockets
      if checkMace() then
         misn.osdDestroy()
         misn.osdCreate( osd_title, {osd_text6,osd_text4:format(competitors_names[5]),osd_text5:format(destpla:name())} )

         score_pankration = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
         pilot.toggleSpawn(false)
         pilot.clear()
         spawnCompetitors( )
         player.pilot():control()
         player.pilot():face(center)

         compHitHook = {nil,nil,nil,nil,nil,nil,nil,nil,nil}
         playerHitHook = hook.pilot( player.pilot(), "attacked", "playerHit" )

         -- Mark this one as player's opponent
         competitors[5]:setFaction("Warlords")
         competitors[5]:setHostile()
         competitors[5]:setHilight()
         duelsEnded = 0

         -- Timer and messages
         hook.timer( 3000, "message", {pilot = competitors[6], msg = chat_20} )
         hook.timer( 6000, "message", {pilot = competitors[4], msg = chat_21} )
         hook.timer( 9000, "message", {pilot = competitors[5], msg = chat_22:format(player.name())} )
         hook.timer( 10000, "startPankration" )
         countdown = 10
         hook.timer( 1000, "timerIncrement")
         omsg = player.omsgAdd(timermsg:format(countdown), 0, 50)

      else
         misn.osdDestroy()
         misn.osdCreate( osd_title, {osd_text5:format(destpla:name())} )
         tk.msg("", mace_fail)
         stage = 5
      end
   end
end

-- Test wether the player only has mace rockets
function checkMace()
   local weap = player.pilot():outfits("weapon")
   for i, w in ipairs(weap) do
      if not (w == outfit.get("Unicorp Mace Launcher")) then
         return false
      end
   end
   return true
end

-- Increment the printing timer
function timerIncrement()
   countdown = countdown - 1
   if countdown == 0 then
      player.omsgChange(omsg, _("Go!"), 3)
   else
      hook.timer( 1000, "timerIncrement")
      player.omsgChange(omsg, timermsg:format(countdown), 0)
   end
end

-- Increment the time over timer
function timerIncrementT()
   countdown = countdown - 1
   if countdown == 0 then
      player.omsgChange(omsg, _("Time Over!"), 3)
   else
      hook.timer( 1000, "timerIncrementT")
      player.omsgChange(omsg, timermsg:format(countdown), 0)
   end
end

-- Actually start the Mace Throw
function startThrow()
   hook.timer( 500, "dehostilify" )
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
   hook.timer( 60000, "endStadion" )
   hook.timer( 50000, "endTimer" ) -- Near-end timer
   misn.osdActive(2)
end

-- Timer for the end of the Stadion
function endTimer()
   countdown = 10
   hook.timer( 1000, "timerIncrementT" ) -- Near-end timer
   omsg = player.omsgAdd(timermsg:format(countdown), 0, 50)
end

function startPankration()
   player.pilot():control(false)
   -- Make people attack each other.
   competitors[5]:memory().atk = atk_generic -- A very agressive AI
   competitors[5]:taskClear()
   competitors[5]:attack(player.pilot())
   compHitHook[5] = hook.pilot( competitors[5], "attacked", "compHit" )
   for i = 1, 4 do
      competitors[i]:setFaction("Warlords")
      competitors[i]:taskClear()
      competitors[i]:attack(competitors[i+5])
      competitors[i+5]:taskClear()
      competitors[i+5]:attack(competitors[i])
      compHitHook[i]   = hook.pilot( competitors[i], "attacked", "compHit" )
      compHitHook[i+5] = hook.pilot( competitors[i+5], "attacked", "compHit" )
      competitors[i]:memory().atk = atk_generic
      competitors[i+5]:memory().atk = atk_generic
   end
   misn.osdActive(2)
end

-- One of the competitors is killed
function compDie( victim, attacker )
   -- This was a bad idea
   if attacker == player.pilot() then
      tk.msg( killer_title, killer_text )
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
   mypilots = pilot.get( { faction.get("Independent") } )
   for i, p in ipairs(mypilots) do
      p:setHostile(false)
   end
   hook.timer( 500, "dehostilify" )
end

-- Spawn Competitors
function spawnCompetitors( )
   competitors = {tam, leblanc, klank, strafer, caros, micoult, johnson, ernst, guo}
   for i = 1, 9 do
      pos = center + vec2.newP( radius, i*360/10 - 90 )
      competitors[i] = pilot.add( "Dvaered Vendetta", "DHC", pos, competitors_names[i])
      equipVendettaMace( competitors[i] )
      competitors[i]:memory().Cindex = i -- Store their index
      competitors[i]:setVisible()
      competitors[i]:control()
      competitors[i]:face(center)
      hook.pilot( competitors[i], "death", "compDie" )
   end
end

-- One of the targets is idle (Mace Throw)
function targetIdle( self )
   pos = center + vec2.newP( rnd.rnd(0,radius), rnd.rnd(0,360) )
   self:moveto( pos, false, false )
end

-- One of the targets is hit (Mace Throw)
function targetHit( victim, attacker )
   victim:setInvincible()
   victim:taskClear()
   victim:land(destpla)
   victim:setFaction( "Independent" )
   victim:setHostile( false )

   if attacker == player.pilot() then
      score_throw[10] = score_throw[10] + 1
      score_total[10] = score_total[10] + 1
   else
      ind = attacker:memory().Cindex
      score_throw[ind] = score_throw[ind] + 1
      score_total[ind] = score_total[ind] + 1
   end

   targetShot = targetShot + 1
   if targetShot >= #targets then
      hook.timer( 3000, "endThrow" )
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
   stage = 3
   misn.osdActive(3)

   -- Sort and print top scores
   local iter = {1,2,3,4,5,6,7,8,9,10}
   table.sort( iter, sortThrow )
   throwTitle = _("Scores for the Mace Throw")
   throwString = ""
   for i = 10, 1, -1 do
      throwString = (throwString..competitors_names[iter[i]]..": "..score_throw[iter[i]].."\n")
   end
   tk.msg( throwTitle, throwString )
end

-- One of the competitors is Idle (Mace Stadion)
function competitorIdle( self )
   self:gather()
end

-- A competitor is hit during Stadion (problematic if the player is the agressor)
function compHitS( victim, attacker )
   if attacker == player.pilot() then
      tk.msg( cheater_title, cheater_text )
      for i, p in ipairs(competitors) do
         p:taskClear()
         p:land( destpla )
      end
      score_total[10] = score_total[10] - 5
      stage = 3 -- Back to previous stage
   end
end

-- Player is hit during Stadion
function playerHitS()
   if player.pilot():health() < 100 then -- Player has lost
      hook.rm(playerHitHook)
      player.pilot():setInvincible()
      player.pilot():control()
      player.pilot():brake()
      tk.msg("",hit_msg)
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
   stage = 5
   misn.osdActive(3)

   for i = 1, 9 do
      score_stadion[i] = competitors[i]:cargoHas("Flowers")
      score_total[i] = score_total[i] + score_stadion[i]
   end
   score_stadion[10] = player.pilot():cargoHas("Flowers")
   score_total[10]   = score_total[10] + score_stadion[10]

   local iter = {1,2,3,4,5,6,7,8,9,10}
   table.sort( iter, sortStadion )
   stadionTitle = _("Scores for the Mace Stadion")
   stadionString = ""
   for i = 10, 1, -1 do
      stadionString = (stadionString..competitors_names[iter[i]]..": "..score_stadion[iter[i]].."\n")
   end
   tk.msg( _("Time Over"), _("Land on Dvaer Prime.") )
   tk.msg( stadionTitle, stadionString )

   -- Free the player
   hook.rm(playerHitHook)
   player.pilot():setInvincible(false)
   player.pilot():control(false)
end

-- One of the competitors (or the player) is hit (Mace Pankration)
function playerHit()
   if player.pilot():health() < 100 then -- Player has lost
      hook.rm(playerHitHook)
      hook.rm(compHitHook[5])
      duelsEnded = duelsEnded + 1
      score_pankration[5] = score_pankration[5] + 7
      score_total[5] = score_total[5] + 7
      competitors[5].taskClear()
      competitors[5].land(destpla)
      competitors[5]:setHostile(false)
      competitors[5]:setInvincible()
      player.msg(pankrationWinner:format(competitors_names[5],competitors_names[10]))
      endPankration()
   end
end
function compHit( victim )
   if victim:health() < 100 then -- Identify who has lost
      ind = victim:memory().Cindex
      if ind == 5 then -- Player won
         hook.rm(playerHitHook)
         hook.rm(compHitHook[5])
         duelsEnded = duelsEnded + 1
         score_pankration[10] = score_pankration[10] + 7
         score_total[10] = score_total[10] + 7
         competitors[5]:taskClear()
         competitors[5]:land(destpla)
         competitors[5]:setHostile(false)
         competitors[5]:setInvincible()
         player.msg(pankrationWinner:format(competitors_names[10],competitors_names[5]))
         endPankration()
      elseif ind < 5 then
         hook.rm(compHitHook[ind+5])
         hook.rm(compHitHook[ind])
         duelsEnded = duelsEnded + 1
         score_pankration[ind+5] = score_pankration[ind+5] + 7
         score_total[ind+5] = score_total[ind+5] + 7
         competitors[ind]:taskClear()
         competitors[ind]:land(destpla)
         competitors[ind+5]:taskClear()
         competitors[ind+5]:land(destpla)
         competitors[ind+5]:setInvincible()
         competitors[ind]:setInvincible()
         player.msg(pankrationWinner:format(competitors_names[ind+5],competitors_names[ind]))
         endPankration()
      else
         hook.rm(compHitHook[ind-5])
         hook.rm(compHitHook[ind])
         duelsEnded = duelsEnded + 1
         score_pankration[ind-5] = score_pankration[ind-5] + 7
         score_total[ind-5] = score_total[ind-5] + 7
         competitors[ind]:taskClear()
         competitors[ind]:land(destpla)
         competitors[ind-5]:taskClear()
         competitors[ind-5]:land(destpla)
         competitors[ind]:setInvincible()
         competitors[ind-5]:setInvincible()
         player.msg(pankrationWinner:format(competitors_names[ind-5],competitors_names[ind]))
         endPankration()
      end
   end
end

-- See if all duels are over
function endPankration()
   if duelsEnded >= 5 then
      hook.timer(1000, "endPankrationT")
   end
end
function endPankrationT()
   stage = 7
   misn.osdActive(3)

   local iter = {1,2,3,4,5,6,7,8,9,10}
   table.sort( iter, sortPankration )
   pankrationTitle = _("Scores for the Mace Pankration")
   pankrationString = ""
   for i = 10, 1, -1 do
      pankrationString = (pankrationString..competitors_names[iter[i]]..": "..score_pankration[iter[i]].."\n")
   end
   tk.msg( pankrationTitle, pankrationString )
end

-- Sorting functions
function sortTotal(i,j)
   return score_total[i] < score_total[j]
end
function sortThrow(i,j)
   return score_throw[i] < score_throw[j]
end
function sortStadion(i,j)
   return score_stadion[i] < score_stadion[j]
end
function sortPankration(i,j)
   return score_pankration[i] < score_pankration[j]
end
