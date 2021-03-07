--[[
<?xml version='1.0' encoding='utf8'?>
 <mission name="Dvaered Triathlon">
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
-- Dvaered Triathlon
-- This is the 6th mission of the Frontier War Dvaered campaign.
-- The player takes part to a funeral ceremony in memory of Lieutenant Strafer.
-- The ceremony turns out to be a Mace Ballet
-- Pilots must perform deeds with only mace rockets

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
local portrait = require "portrait"

-- TODO: hooks to penalize attacking people
-- TODO: penalties for landing at wrong place

invite_text = {}
invite_title = _("An invitation")
invite_text[1] = _([[Your communication channel informs you that your ship has recieved an holographic message through the Dvaered Army Long-Distance Messaging System. As you press the "Enter" button, the face of a teenage version of Lieutenant Strafer emerges from your holoprojector:
   "I am Private Helmut Strafer and this is my death announcement." A sad and peaceful smile appears on his face as he continues: "Yep, this is happening. It is my ultmost priviledge to have been killed at the service of the Dvaered Nation, Great House of the Glorious Galactic Empire. You recieve this recording because you are on the list of people I considered to be my friends, and that are invited to my funeral ceremony, which I sincerely hope you will enjoy. As I obviously don't know yet either the circumstances of my death, neither the details about the ceremony, all I can do is to invite you to find more informations in the attached data. Now that I have merged with the nothingness, I would be honoured if my memory would be part of the things that help you remain right, loyal and strong for the rest of your own lifetime."]])
invite_text[2] = _([[You browse the attached floder and find out that the ceremony will take place around Dvaer Prime. As a pilot, you are invited to take part to a Mace Rocket ballet in memory of Lieutenant Strafer, and are strongly advised to show up with a fighter and mace launchers.
   Are you going to go there?]])


title_npc = "Dvaered People" -- TODO: describe music
text_npc = [[A rather large group is gathered around a few reserved tables. You recognize many faces, among which General Klank, Major Tam and the members of their group. But there are people you do not know as well, mostly military, but also civilians. All of them seem to be there for the ceremony as attests their black armbands.]]

text_approach = [[You approach the group, and get close to Major Tam. "Good day, citizen %s. I hope you are ready for the mace ballet!" You answer that you have no idea what this ballet is about, but you are always ready when it comes to mace rockets.
   "The mace ballet, also known as mace triathlon, is a series of three events where pilots must honour the memory of their fallen comrade and show their skills. The members of the Nightclaws squadron will take part to the event, along with both of us, General Klank, a few members of Strafer's family, and some of his former comrades before he joined the squadron. From the outcome of the competition will depend how Strafer's personnal outfits will be distributed. This includes two Vendettas, nice core outfits, weapons and utilities.
   "Come at me when you're ready to take off."]]


tam_name = _("Major Tam")
ham_name = _("Captain Hamfresser")
leb_name = _("Captain Leblanc")
klk_name = _("General Klank")
nkv_name = _("Sergeant Nikolov")
wdw_name = _("Well-dressed woman")
dad_name = _("Retired soldier")
sst_name = _("Sergeant")
pvt_name = _("Technician")

tam_desc = _("Major Tam is ready to explain the next stage of the ceremony to you")
ham_desc = _("Hamfresser stays behind a group of army technicians. He bows towards them, probably attempting to take part to the conversation, but no one seems to give him any attention. His face seems to reflect not only boredom, but also shame not to be able to fit among the group. He swings his unused cybernetic arms around his hips.")
leb_desc = _("Leblanc is surrounded by her pilots, who somehow exchange jokes about their respective collections of chopped heads.")
klk_desc = _("The general discusses with Major Tam.")
nkv_desc = _("The cyborg sergeant is doing arm-wrestling against half a dozen of soldiers, and seems to be very cautious in order not to harm them.")
wdw_desc = _("One of the rare civilians around, this woman seems however to be used to hang out with soldiers.")
dad_desc = _("An old captain who seems to have ironed his pageantry uniform for the occasion discusses with some civilians.")
sst_desc = _("This pilot is not a mamber of Leblanc's squadron, however, she discusses with the pilots.")
pvt_desc = _("A military technician encourages his comrades who are arm-wrestling with Nikolov.")

text_hamfresser = [[Hey, %s! Long time no see, huh? How do you do? I've been stuck at the hospital lately because of all the damage taken during last mission. I got a brand new right arm, you see? With the latest bio and cyber enhancements. Targetting abilities have been increased by 0.23 percent, traction power by 0.26 percent and thrust by 0.22 percent. But its best feature is that I can now scratch my nose without putting oil marks on it. Everyone is jealous at the barracks.]]
text_leblanc = [[Hello, citizen. I am glad to see you have been invited as well. you deserve it. It always annoys me to loose a good pilot like Strafer, but you know, life is like that, when you are a Dvaered. Anyway, you know, we managed to avenge him, out there. In addition to the four ones you did neutralize, we got seven of them during the pursuit. Unfortunately, the one in the Schroedinger managed to get out, but this time, I doubt they will be able to reconstruct their forces soon. You know, the ex-colonel Hamelsen will have hard time recruiting pilots with such a high loss rate.]]
text_klank = [[Good day, citizen. You are the private pilot helping us in our task, right? I have heard that you are doing fairly good job. Continue and you will get rewared for your merit, be sure of that!]]
text_nikolov = [[Yeah, he was for sure a good guy. Of course, he was an "educated" man, like the others here, always calling everyone "citizen", and annoying people with "righteousness", "valor" and stuff. But he was one of the few who did not despise us spacemarines, and we could count on him. He certainly will be missed.]]
text_widow = [[It feels so strange. I knew this day could come sooner or later, but yet... I can't really figure out that me and the children will live without my husband from now. It makes me so sad. Do you think that creating the next generation of Dvaered warriors is the sole purpose of wedding?]]
text_dad = [[The dark see took so many of my ancestors, my own father, then my wife, and now my elder child. Fate did not make me die in my time, so I guess I am simply meant to stay home, waiting for the rest of my family members to die one after the other.]]
text_sister = [[You will take part to the mace ballet too? Ah! I can't wait to fight the friends of my big brother!]]
text_technician = [[Hi!]] -- TODO: better text

text_stage = {}
text_stage[1] = [[The first event consists in the "Mace Throw". There is a series of targets (old Llamas) and you have to hit them in the shortest time. The quickest pilot wins. Are you ready? Have you checked you are only equipped with Mace Launchers?]]
text_stage[2] = [[The second event consists in the "Mace Stadion". Crowns of flowers have been dropped out there, and some junior pilots from the academy camp in the gather zone with their mace launchers. They will aim at the competitors, and if your shield is disabled, you are eliminated. The pilot who has gathered the most flowers before being eliminated, or the time runs out wins. It is forbitten to shoot at other competitors. Are you ready? Have you checked you have at least %d tons of free cargo space?]]
text_stage[3] = [[The third and last event consists in the "Mace Pankration". Each competitor must fight against an other competitor, and disable his shield. Killing is of course not allowed. Are you ready? Have you checked you are only equipped with Mace Launchers?]]


misn_desc = _("You are invited to a Mace Rocket Ballet in memory of Lieutenant Strafer.")
misn_reward = _("Say goodbye to Lieutenant Strafer")
log_text = _("You took part to a Mace Rocket Ballet in memory of Lieutenant Strafer.")

osd_title = _("Dvaered Triathlon") -- TODO: this name feels odd
osd_text1 = _("Fly to %s")
osd_text2 = _("Hit the most targets in the shortest time")
osd_text3 = _("Catch the most flowers possible")
osd_text4 = _("Defeat your opponent (nullify their shield)")

-- TODO: add pointless chatter

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
   hook.load("spawnNpcs")

   portrait_dad = portrait.getMaleMil("Dvaered")
   portrait_wdw = portrait.getFemale()
   portrait_sst = portrait.getFemaleMil("Dvaered")
   portrait_pvt = portrait.getMil("Dvaered")

   -- Prepare storing of total scores of competitors
   score_total = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } -- TODO: display the score when landed
   score_total["__save"] = true
   competitors_names = { _("Major Tam"),
                         _("Captain Leblanc"),
                         _("General Klank"),
                         _("Sergeant Strafer"),
                         _("Sergeant Garcia"),
                         _("Private Micoult"),
                         _("Corporal Johnson"),
                         _("Private Ernst"),
                         _("Lieutenant Guo"),
                         player.name(), }
   competitors_names["__save"] = true

   radius = 4000
end

-- Landing (on Dv Prime)
function land()
   spawnNpcs() -- This handles people on the bar
end

function spawnNpcs()
   if (planet.cur() == destpla) then
      if stage == 0 then
         npc = misn.npcAdd( "approach", title_npc, portrait.getMil( "Dvaered" ), text_npc)
      else
         populate_bar()
         if stage == 1 or stage == 2 then
            tam = misn.npcAdd("tamStage1", tam_name, portrait_tam, tam_desc)
         elseif stage == 3 then
            tam = misn.npcAdd("tamStage2", tam_name, portrait_tam, tam_desc)
         elseif stage == 5 then
            tam = misn.npcAdd("tamStage3", tam_name, portrait_tam, tam_desc)
         elseif stage == 7 then -- TODO: reward: 1: Dvaered Vendetta, 2: Vendetta, 3: Zephir II + Power Regul Override, 4: Milspec Orion + Shield Capacitor, 5: Small Combat Plating, 6: Shredders, 7: Hellburner, 8: Reactor, 9: Shield Booster, 10: Battery
            
         end
      end
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
   tk.msg( "", text_technician )
end

-- Instructions
function tamStage1()
   tk.msg( "", text_stage[1] )
   stage = 2
end
function tamStage2()
   tk.msg( "", text_stage[2]:format(16) )
   stage = 4
end
function tamStage3()
   tk.msg( "", text_stage[3] )
   stage = 6
end

-- Take off for the next stage
function takeoff()
   center = destpla:pos() + vec2.new(0,radius)

   if stage == 2 then -- Mace Throw: spawn Llamas and competitors.
      -- TODO: check the player only has mace rockets
      -- TODO: OSD
      -- TODO: timer before it begins

      score_throw = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
      pilot.toggleSpawn(false)
      pilot.clear()
      spawnCompetitors( ) -- TODO: comm

      hook.timer( 500, "dehostilify" )
      targets = {}
      for i = 1, 30 do
         pos = center + vec2.newP( rnd.rnd(0,radius), rnd.rnd(0,360) )
         targets[i] = pilot.add( "Llama", "Warlords", pos, "Target "..tostring(i))
         targets[i]:control()
         targets[i]:setHostile() -- Just in case
         pos = center + vec2.newP( rnd.rnd(0,radius), rnd.rnd(0,360) )
         targets[i]:moveto( pos, false, false )
         hook.pilot( targets[i], "idle", "targetIdle" )
         hook.pilot( targets[i], "attacked", "targetHit" )
      end
      targetShot = 0

   elseif stage == 4 then -- Mace Stadion: spawn flowers, competitors and annoyers
      -- TODO: check the player only has mace rockets
      -- TODO: OSD
      -- TODO: timer before it begins

      score_stadion = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
      pilot.toggleSpawn(false)
      pilot.clear()
      spawnCompetitors( ) -- TODO: comm

      for i, p in ipairs(competitors) do
         p:setInvincible() -- Bouuuh! they r cheating!
         p:control()
         p:memory().gather_range = 4*radius
         hook.pilot( p, "idle", "competitorIdle" )
         p:gather()
      end

      annoyers = {}
      for i = 1, 10 do
         pos = center + vec2.newP( rnd.rnd(0,radius), rnd.rnd(0,360) )
         annoyers[i] = pilot.add( "Dvaered Vendetta", "Warlords", pos, "Shooter")
         equipVendettaMace( annoyers[i] )
         annoyers[i]:setSpeedLimit( .0001 )
      end -- TODO: if player has zero shield, eliminated

      for i = 1, 30 do
         pos = center + vec2.newP( rnd.rnd(0,radius), rnd.rnd(0,360) )
         print(i)
         print( system.addGatherable( "Ore", 1, pos, vec2.new(0,0), 3600 ) )
      end

      hook.timer( 100000, "endStadion" ) -- TODO: explain there is a timer

   elseif stage == 6 then -- Mace Pankration: competitors and make teams
      -- TODO: check the player only has mace rockets
      -- TODO: OSD
      -- TODO: timer before it begins

      score_pankration = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
      pilot.toggleSpawn(false)
      pilot.clear()
      spawnCompetitors( ) -- TODO: comm

      -- Make people attack each other. TODO: see if we put everyone is the same time
      competitors[5]:memory().atk = atk_generic -- A very agressive AI
      competitors[5]:control()
      competitors[5]:setHostile()
      competitors[5]:setHilight()
      competitors[5]:attack(player.pilot())
      playerHitHook = hook.attacked( "playerHit" )
      compHitHook = {nil,nil,nil,nil,nil,nil,nil,nil,nil}
      compHitHook[5] = hook.pilot( competitors[5], "attacked", "compHit" )
      for i = 1, 4 do
         competitors[i]:control()
         competitors[i]:attack(competitors[i+5])
         competitors[i+5]:control()
         competitors[i+5]:attack(competitors[i])
         compHitHook[i]   = hook.pilot( competitors[i], "attacked", "compHit" )
         compHitHook[i+5] = hook.pilot( competitors[i+5], "attacked", "compHit" )
         competitors[i]:memory().atk = atk_generic 
         competitors[i+5]:memory().atk = atk_generic 
      end
      duelsEnded = 0
   end
end

-- Hack for the ships not to be hostile anymore once shot (Mace Throw)
function dehostilify()
   mypilots = pilot.get( { faction.get("Civilian") } )
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
      competitors[i] = pilot.add( "Dvaered Vendetta", "DHC", pos, competitors_names[1])
      equipVendettaMace( competitors[i] )
      competitors[i]:memory().Cindex = i -- Store their index
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
   victim:setFaction( "Civilian" )
   victim:setHostile( false )

   if attacker == player.pilot() then
      -- TODO: play a sound, be happy
      score_throw[10] = score_throw[10] + 1
      score_total[10] = score_total[10] + 1
   else
      ind = attacker:memory().Cindex
      score_throw[ind] = score_throw[ind] + 1
      score_total[ind] = score_total[ind] + 1
   end

   targetShot = targetShot + 1
   if targetShot >= #targets then
      for i, p in ipairs(competitors) do
         p:control()
         p:land(destpla)
      end
      stage = 3
      -- TODO: osd

      -- TODO: use table.sort
      throwTitle = _("Scores for the Mace Throw")
      throwString = ""
      for i = 1, 10 do
         throwString = (throwString..competitors_names[i]..": "..score_throw[i].."\n")
      end
      tk.msg( throwTitle, throwString )
   end
end

-- One of the competitors is Idle (Mace Stadion)
function competitorIdle( self )
   self:gather()
end

-- End of the Mace Stadion -- TODO: warn the player
function endStadion()
   for i, p in ipairs(competitors) do
      p:taskClear()
      p:land(destpla)
   end
   stage = 5
   -- TODO: osd

   for i = 1, 9 do
      score_stadion[i] = competitors[i]:cargoHas("Ore")
      score_total[i] = score_total[i] + score_stadion[i]
   end
   score_stadion[10] = player.pilot():cargoHas("Ore") -- TODO: it should be changed to Flowers
   score_total[10]   = score_total[10] + score_stadion[10]

   stadionTitle = _("Scores for the Mace Stadion")
   stadionString = ""
   for i = 1, 10 do
      stadionString = (stadionString..competitors_names[i]..": "..score_stadion[i].."\n")
   end
   tk.msg( stadionTitle, stadionString )
end

-- One of the competitors (or the player) is hit (Mace Pankration)
function playerHit()
   if player.pilot():health() < 100 then -- Player has lost
      hook.rm(playerHitHook)
      hook.rm(compHitHook[5])
      duelsEnded = duelsEnded + 1
      score_pankration[5] = score_pankration[5] + 10
      score_total[5] = score_total[5] + 10
      competitors[5].taskClear()
      competitors[5].land(destpla)
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
         score_pankration[10] = score_pankration[10] + 10
         score_total[10] = score_total[10] + 10
         competitors[5].taskClear()
         competitors[5].land(destpla)
         endPankration()
      elseif ind < 5 then
         hook.rm(compHitHook[ind+5])
         hook.rm(compHitHook[ind])
         duelsEnded = duelsEnded + 1
         score_pankration[ind+5] = score_pankration[ind+5] + 10
         score_total[ind+5] = score_total[ind+5] + 10
         competitors[ind].taskClear()
         competitors[ind].land(destpla)
         competitors[ind+5].taskClear()
         competitors[ind+5].land(destpla)
         endPankration()
      else
         hook.rm(compHitHook[ind-5])
         hook.rm(compHitHook[ind])
         duelsEnded = duelsEnded + 1
         score_pankration[ind-5] = score_pankration[ind-5] + 10
         score_total[ind-5] = score_total[ind-5] + 10
         competitors[ind].taskClear()
         competitors[ind].land(destpla)
         competitors[ind-5].taskClear()
         competitors[ind-5].land(destpla)
         endPankration()
      end
   end
end

-- See if all duels are over
function endPankration()
   if duelsEnded >= 5 then
      stage = 7
      pankrationTitle = _("Scores for the Mace Pankration")
      pankrationString = ""
      for i = 1, 10 do
         pankrationString = (pankrationString..competitors_names[i]..": "..score_pankration[i].."\n")
      end
      tk.msg( pankrationTitle, pankrationString )
   end
end
