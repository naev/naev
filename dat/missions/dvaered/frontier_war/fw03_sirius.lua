--[[
<?xml version='1.0' encoding='utf8'?>
 <mission name="Dvaered Diplomacy">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>2</priority>
   <chance>30</chance>
   <done>Dvaered Escape</done>
   <location>Bar</location>
   <faction>Dvaered</faction>
   <cond>var.peek("dv_pirate_debt") == false</cond>
  </avail>
  <notes>
   <campaign>Frontier Invasion</campaign>
   <done_evt name="Repay General Klank"/>
  </notes>
 </mission>
 --]]
--[[
-- Dvaered Diplomacy
-- It does not go together well, does it?
-- This is the 4th mission of the Frontier War Dvaered campaign.
-- The player has to trap and kill a group of assassins who are after Major Tam

   Stages :
   0) On the way (can leave the system)
   1) Looking for hostiles (cannot leave the system)
   2) Land to get rewarded
   3) Pursuit: cleaning the first system before jumping
   4) Pursuit: you may jump after the target
   5) Pursuit: cleaning the first system before landing
   6) Pursuit: you may land after the target
--]]

require "missions/dvaered/frontier_war/fw_common"
require "selectiveclear"
require "nextjump"
require "proximity"
require "numstring"

npc_name = _("Major Tam")
npc_desc = _("The major seems to be waiting for you.")

propose_title = _("Ready for some diplomacy?")
propose_text = _([[Major Tam seems to be in a very good mood: "Hello, citizen %s! Have you got plans for tonight?" You fear he may invite you to a brawl-party, or any other kind of event Dvaered usually organize when they want to have good time, but he continues: "Because I've got a very important mission for you.
   "Let me explain the context to you: I am currently in the middle of a secret diplomatic campaign with the Imperials, House Goddard and the Sirii. But there is a group of assassins who are constantly attacking me when I am moving. This is very annoying and we have already lost two pilots of Leblanc's squadron in these ambushes, including the second in command. The problem is that the ambushers use a mix of fighters, supported by medium ships. They even have a Kestrel!
   "So we have decided to deal with this problem before we proceed with the diplomatic meetings. Are you in?"]])

accept_title = _("This is how we deal with our problems")
accept_text = _([["You won't regret that!" Tam says... "Unless you're killed in the process I guess... Anyway, I will go to %s in %s in my Vigilance. No doubt the assassins will set up an ambush somewhere on the way. You and a few pilots from the Special Operations Force (SOF) will locate them and wait for me to jump in. During the ambush, the assassins will be caught in the crossfire, and we will annihilate them to the last ship. Actually, that sounds much more attractive than a 3 periods long meeting with rogue Sirian diplomats, doesn't it?
   "On our side, we will not have much firepower, so I count on you to provide with the armor and the guns (but you should still be able to outrun mid-range destroyers). You will team up with Lieutenant Strafer, because it worked well last time, and two other pilots. But the SOF does not have countless pilots, so the amount of your reward will depend on how many of them come back. Strafer will fly a scout with high performance sensors. He will be our eyes, so his death would mean the end of the mission.
   "If you have any additional questions, I'll stay at the bar until we take off."]])

refuse_title = _("Another time maybe")
refuse_text = _([[Major Tam really seems disappointed. "As you wish, citizen..."]])


lore_title = _("Ask your questions to Major Tam")
lore_text0 = _("What do you want to ask to Major Tam?")
lore_already_told = _("Hey, do you think I'm some kind of amnesic? You literally asked this question 10 seconds ago!")
quittam = _("Goodbye, we meet in space.")
question = {}
lore_text = {}

question[1] = _("Ask who are the assassins")
lore_text[1] = _([["Good question, citizen! It is very likely that some people among the intelligence services of the other nations know that General Klank and myself are implicated in the invasion project. They probably just don't know how imminent it is. So they could have hired henchmen to stop or annoy us. We have identified some of the henchmen actually, and they are the kind of independent mercenaries we find on any kind of shady operations... A bit like you in fact."]])

question[2] = _("Ask about Battleaddict's soldiers")
lore_text[2] = _([["Aha! You're worrying about the soldiers of the Warlord we killed last time? Don't worry too much for them, most of them did not stay unemployed for long. When a warlord dies, his surviving followers are sent to the Dvaered Military Reserve, and they apply for positions in other warlords' armies, or in the DHC. The only one I'm worrying about is the Colonel Hamelsen: it will be very difficult for her to find a position with a warlord as they prefer to have people they know at higher ranks, and the DHC only rarely recruits new colonels.
   "That's a pity actually because this woman is probably the most talented officer of her generation. Do you know that she was the first one since the independence war who managed to become an ace before the end of her training?"]])

question[3] = _("Ask about his deeds as fighter pilot")
lore_text[3] = _([[You ask Major Tam to continue his story about when he used to be a fighter pilot at the DHC base on Rhaana. A small smile appears on his face and he turns his eyes to the roof.
   "Do you want me to tell you how I became an ace? Oh yes, you do. So. To become a Dvaered ace, one has to score four attested fighter victories. An attested victory is recorded when you destroy alone any kind of enemy warship. As we tend to work in squads, we pretty often destroy ships without scoring any victory. My first one was a pirate hyena. The pilot was probably drunk or so and they headed right towards my Vendetta, broadcasting stupid taunts in the radio. My sergeant has been nice and did let the hyena to me. My second victory was against a freak... I mean a local warlord's pilot who claimed she could kill any DHC pilot in fair fight. Apparently, it was not true.
   "But the two others... Actually it is a much longer story. I'll tell you another time maybe."]])

question[4] = _("Ask news from Hamfresser")
lore_text[4] = _([["Oh, the captain is doing well. He spent a few periods at the hospital, but it was not so serious. About the other ones, I'm not too sure. I don't know them very well actually, they are more like statistics to me. I know it's bad, but hey, I have so many things to think about right now.
   "Oh, and we went to the funerals of the one who died, the private Amadeus Tronk, officially killed in training, of course. It was nice. Leblanc's squadron has made an aerial meeting with a mace rocket concerto for the occasion, and we organized a fight to death between a convicted criminal and a gladiator in General Klank's private arena. The role of it is to honour the memory of the dead warrior, and also to have some fun."]])


located_title = _("Enemies located")
located_text = _([[You receive an encrypted message from Strafer. "I've located the hostiles. We need to follow them from afar, and get ready for action."]])
scan_msg = _("Wait until Strafer has scanned the system")

flee_title = _("What are you doing here?")
flee_text = _("You were supposed to wait for Strafer to scan the system, and then if needed, kill the hostiles.")
clean_text = _("You were supposed to clean the system before pursuing the fleeing ship.")
sjump_text = _("You were supposed to jump to %s.")
sland_text = _("You were supposed to land on %s.")

failed_title = _("This is not good")
jumpout_text = _([[While your sensors lose the signal of the target you were supposed to kill, you expect to receive a mission failure message, but instead, you hear Tam's ship communication: "One of them escaped. Continue destroying the others. Afterwards, %s, you will jump to %s and destroy that ship."]])
land_text = _([[While your sensors lose the signal of the target you were supposed to kill, you expect to receive a mission failure message, but instead, you hear Tam's ship communication: "One of them escaped. Continue destroying the others. Afterwards, %s, you will land on %s to keep track on the pilot."]])
tamdie_text = _([[The blinking cross marked "Major Tam" on your radar screen suddenly turns off. You first believe this is an output bug and hit the screen with your open hand, but soon, you realize that the radar works perfectly well. This means that the major's ship got annihilated by the ambushers, and therefore your mission is a miserable failure.]])
strdie_text = _([[You suddenly realize that your radar doesn't receive Strafer's sensors data. This means that the Lieutenant got killed. As a result, the fleet is flying blind and the mission is failed.]])
tflee_text = _([[Two of your targets went away. If there was only one, it could be manageable, but now, the mission has failed.]])

execution_title = _("End of the hunt")
execution_text1 = _([[You land and walk around the spacedock, in search of your target's ship. You finally see it. A mighty %s, covered by the stigmas of the battle that just occurred. You decide to hide yourself behind crates close to the ship and wait for the pilot to come back and take off in order to finish your job in space.
   When looking closer at the ship, you see ancient and recent marks on the hull, caused by all kinds of weapons during the lifetime of the ship, that have been repaired on spaceports. Among the ship's scars,you see a twisted welding around the ship's nose, filled with bubbles and think: "Damn! They've got to deal with the same old deficient welding android that fixed my airlock on Alteris last time!"
   Suddenly, you realize someone is whispering behind you "Hey, %s, you're wrecking my firing line!" You turn around and see nothing but a deformed crate that continues to speak: "It's me, Sergeant Nikolov. In the box. Hide yourself better or you will ruin our mission." You then remember that she is a member of the space infantry commandos, and Hamfresser's second in command. Tam probably sent her to execute the enemy pilot.]])
execution_text2 = _([[A bit later, you see a woman coming from the empty corridor, anxiously looking behind her and pulling a key out of her pocket. While still approaching the ship, she presses the key's button and the ship beeps. At this very moment, a sudden and loud din erupts from all around you shake your stomach and the pilot falls without a word. Nikolov and two other soldiers emerge from the crates. The sergeant approaches the pilot, kneels and takes her pulse. She thoughtfully looks at her face "Damn! she looked like a nice person..." And then addresses to the soldiers: "All right, folks we pack up!" and the unit enters the ship with the body and takes off.
   You stay alone, on the empty dock, with nothing but your thoughts. Even the broken crates have been picked up by the commandos. You think about all the causes that pilot must have served in her life. The just causes, the evil ones... and all the others. "Meh," you think "killing people in space is definitely much better for morale."]])
execution_text3 = _([[When you finally go to the bar to think about something else, you get notified on your holowatch that %s have been transferred to your account. The leader of the ambushers has been identified: it's Colonel Hamelsen, who used to work for Battleaddict before his death. Unfortunately, the Colonel has escaped.]])

execution_failed_text2 = _([[A few times later, you hear a message from Nikolov's radio: "Tam here. The target escaped and won't come back to the ship. Clear the spacedock." The spacemarines emerge from the crates and disappear in a blink, while you start heading to the bar. On the way, you meet Strafer who explains the situation: "We identified the hostile pilot: it was Colonel Hamelsen, Battleaddict's former second in command, but she got away using one of her other ships and we lost her track.
   "Poor woman. It's hard to get a new post when you're the second in command of a dead warlord, you know. So I guess someone has managed to hire her to assassinate the major. Anyway, I guess you should have received your payment of %s by now."]])

won_title = _("All targets eliminated")
won_text = _([[All three primary targets have been eliminated. The remaining ones are not dangerous anymore now. You can land to get your reward.]])

pay_title = _("Mission accomplished")
pay_text = _("Your mission is a success, except for the escape of the enemy leader, Colonel Hamelsen. You can now collect your %s reward.")

killMsg      = _("One of the targets was destroyed!")
killAll      = _("The last target was destroyed! You can now land.")
clearMsg     = _("No hostiles here. We can proceed to next system.")
hostiMsg     = _("Hostiles spotted!")
stampedeMsg  = _("Hostiles are running away. Don't let them escape!")
killedMsg    = _("Hostiles eliminated.")
pursuitMsg  = _("The target ran away again: continue the pursuit.")
preAmbushMsg = _("All right, let's wait for Tam")
preAmbush1   = _("There will be roasted Tam for lunch today!")

baddie_taunt  = _("What about I cook you to death?")
baddie_signal = _("Tam is in place, folks! don't miss him, this time!")
ham_runaway   = _("You won't catch me alive! Never!")


misn_desc = _("You take part of an operation whose goal is to trap and destroy a group of well armed henchmen who are after Major Tam.")
misn_reward = _("It depends how many of your wingmen come back.")
log_text = _("A group of killers, who were after Major Tam have been trapped and killed. For now, we don't know who has paid them, but they were led by Colonel Hamelsen, who has managed to escape.")

osd_title = _("Dvaered Diplomacy")
osd_text1 = _("Go to next system")
osd_text2 = _("Wait until Strafer has scanned the system")
osd_text3 = _("Wait for Tam and destroy the highlighted hostile ships")
osd_text4 = _("Land anywhere to collect your pay")
osd_text5 = _("Clean the current system")
osd_text6 = _("Jump to %s to follow your target")
osd_text7 = _("Land on %s to follow your target")


function create()
   destpla, destsys = planet.get("Mannannan")
   startsys = system.cur()
   d = startsys:jumpDist( destsys )

   -- The starting point should not be too close from there
   if d < 3 then
      misn.finish(false)
   end

   path = system.cur():jumpPath(destsys)
   path["__save"] = true
   ambushsys = path[ rnd.rnd(2,3) ]:dest()

   if not misn.claim(ambushsys) then
      misn.finish(false)
   end

   misn.setNPC(npc_name, portrait_tam)
   misn.setDesc(npc_desc)
end

function accept()
   if not tk.yesno( propose_title, propose_text:format(player.name()) ) then
      tk.msg(refuse_title, refuse_text)
      misn.finish(false)
   end
   tk.msg(accept_title, accept_text:format(destpla:name(), destsys:name()))

   misn.accept()
   misn.osdCreate( osd_title, {osd_text1, osd_text2, osd_text3, osd_text4 } )
   misn.setDesc(misn_desc)
   misn.setReward(misn_reward)

   -- Markers
   marklist = {}
   marklist[1] = misn.markerAdd(system.cur(), "low")
   for i,jp in ipairs(path) do
      marklist[i+1] = misn.markerAdd(jp:dest(), "low")
   end
   marklist["__save"] = true

   stage = 0

   previous = planet.cur()
   enterhook = hook.enter("enter")
   landhook = hook.land("land")

   -- Stores the state of the escort
   alive = {true,true,true}
   alive["__save"] = true

   toldya = {false,false,false,false}
   boozingTam = misn.npcAdd("discussWithTam", npc_name, portrait_tam, npc_desc)
end

-- Discussions with Major Tam at the bar
function discussWithTam()
   local c = tk.choice( lore_title, lore_text0, question[1], question[2], question[3], question[4], quittam )
   if c <= 4 then
      if toldya[c] then
         tk.msg( lore_title, lore_already_told )
      else
         tk.msg( lore_title, lore_text[c] )
         toldya[c] = true
      end
   end
end

function enter()

   -- Entering the next system of the path
   if stage == 0 then
      spawnEscort(previous)

      sysind = elt_dest_inlist(system.cur(), path) -- Save the index of the system in the list (useful to remove the marker later)
      if (sysind > 0) or (system.cur() == startsys) then
         stage = 1
         misn.osdActive(2)

         -- Decide from where the ambushers come
         if system.cur() == destsys then
            ambStart = destpla
         else
            ambStart = getNextSystem(system.cur(), destsys)
            ambJp = jump.get(system.cur(), ambStart) -- We assume there are no one-way jumps
         end

         escort[1]:control(true)
         escort[1]:moveto( ambJp:pos() )  -- Let's say Strafer knows where they are supposed to come from...

         if system.cur() == ambushsys then
            pilot.toggleSpawn("Pirate")
            pilot.clearSelect("Pirate")
            spawnBaddies( ambStart )

            -- Find the waiting point: on the line tamPoint -> ambJp, at 3000 of tamPoint
            -- Normally, tamPoint and absStart are never the same
            tamPoint = getNextSystem(system.cur(), startsys)
            tamJp = jump.get(system.cur(), tamPoint)
            PA = ambJp:pos() - tamJp:pos()
            PW = PA/PA:mod() * 3000
            waitPoint = tamJp:pos() + PW
            badguys[1]:control()
            badguys[1]:moveto(waitPoint)
            hook.timer(500, "proximity", {location = waitPoint, radius = 3000, funcname = "badInPosition", focus = badguys[1]})

            escort[1]:taskClear()
            escort[1]:follow( badguys[1] ) -- Yes, he's not supposed to know where they are, but it's safer for the trigger
            hook.timer(500, "proximityScan", {anchor = badguys[1], funcname = "straferScans", focus = escort[1]})
         else
            hook.timer(500, "proximity", {location = ambJp:pos(), radius = 4000, funcname = "straferReturns", focus = escort[1]})
         end
      end

   -- Illegitimate system entrances
   elseif stage == 1 then
      tk.msg(flee_title, flee_text)
      misn.finish(false)
   elseif stage == 3 or stage == 5 then
      tk.msg( flee_title, clean_text )
      misn.finish(false)
   elseif stage == 6 then
      tk.msg(flee_title, sland_text:format(nextt:name()))
      misn.finish(false)

   -- Enter after a fleeing enemy
   elseif stage == 4 then
      if system.cur() == nextt then
         target = pilot.addRaw( shi:nameRaw(), "Mercenary", previous )
         target:setHealth( arm, sld, str )
         target:setTemp( tem )
         target:setHilight()
         target:setFaction("Warlords")

         hook.pilot(target, "death","lastOne_died")
         hook.pilot(target, "jump","lastOne_jump")
         hook.pilot(target, "land","lastOne_land")
      else
      tk.msg(flee_title, sjump_text:format(nextt:name()))
         misn.finish(false)
      end
   end

   previous = system.cur()
end

function land()
   -- Illegitimate landing
   if stage == 1 then
      tk.msg(flee_title, flee_text)
      misn.finish(false)

   -- Land for reward
   elseif stage == 2 then
      compute_reward()
      tk.msg( pay_title, pay_text:format(creditstring(effective_credits)) )
      payNfinish()

   -- More illegitimate landings
   elseif stage == 3 or stage == 5 then
      tk.msg( flee_title, clean_text )
      misn.finish(false)
   elseif stage == 4 then
      tk.msg(flee_title, sjump_text:format(nextt:name()))
      misn.finish(false)

   -- Landing after an enemy (victory as well)
   elseif stage == 6 then
      if planet.cur() == nextt then
         compute_reward()
         if shi:nameRaw() == "Kestrel" then -- it's Hamelsen and she escapes
            tk.msg( execution_title, execution_text1:format(shi:name(), player.name()) )
            tk.msg( execution_title, execution_failed_text2:format(creditstring(effective_credits)) )
         else -- No pity for non-Hamelsen henchmen
            tk.msg( execution_title, execution_text1:format(shi:name(), player.name()) )
            tk.msg( execution_title, execution_text2, "portraits/neutral/female1.png" )
            tk.msg( execution_title, execution_text3:format(creditstring(effective_credits)) )
         end
         payNfinish()
      else
         tk.msg(flee_title, sland_text:format(nextt:name()))
         misn.finish(false)
      end
   end
   previous = planet.cur()
end

-- Spawn the escort
function spawnEscort( origin )
   escort = {}
   if alive[1] then
      escort[1] = pilot.addRaw( "Schroedinger", "DHC", origin )
      escort[1]:rename("Lieutenant Strafer")

      -- Give him nice outfits
      escort[1]:rmOutfit("all")
      escort[1]:rmOutfit("cores")
      escort[1]:addOutfit("S&K Ultralight Stealth Plating")
      escort[1]:addOutfit("Tricon Zephyr Engine")
      escort[1]:addOutfit("Milspec Aegis 2201 Core System")
      escort[1]:addOutfit("Reactor Class II")
      escort[1]:addOutfit("Reactor Class I")
      escort[1]:addOutfit("Hellburner")
      escort[1]:addOutfit("Milspec Scrambler")
      escort[1]:addOutfit("Improved Stabilizer")
      escort[1]:addOutfit("Shredder")
      escort[1]:setHealth(100,100)
      escort[1]:setEnergy(100)
      escort[1]:setFuel(true)
      escort[1]:setHilight()
      escort[1]:setVisplayer()

      escort[1]:memory().angle = 225
      escort[1]:memory().radius = 200
      escort[1]:memory().shield_run = 90
      escort[1]:memory().shield_return = 95

      hook.pilot(escort[1], "hail", "escort_hailed") -- TODO: see if we want that for Strafer
      hook.pilot(escort[1], "death", "escort_died1")

      escort[1]:control()
      escort[1]:follow(player.pilot(), true)
   end

   if alive[2] then
      escort[2] = pilot.addRaw( "Vendetta", "DHC", origin )
      hook.pilot(escort[2], "hail", "escort_hailed")
      hook.pilot(escort[2], "death", "escort_died2")
      escort[2]:control()
      escort[2]:follow(player.pilot(), true)
      escort[2]:memory().angle = 225
      escort[2]:setHilight()
      escort[2]:setVisplayer()
   end
   if alive[3] then
      escort[3] = pilot.addRaw( "Phalanx", "DHC", origin )
      hook.pilot(escort[3], "hail", "escort_hailed")
      hook.pilot(escort[3], "death", "escort_died3")
      escort[3]:control()
      escort[3]:follow(player.pilot(), true)
      escort[3]:memory().angle = 135
      escort[3]:setHilight()
      escort[3]:setVisplayer()
   end
end

-- Spawn the bad guys
function spawnBaddies( origin )
   badguys = {}
   -- They're mercenaries to avoid getting too high outfits
   badguys[1]  = pilot.addRaw( "Kestrel", "Mercenary", origin )
   badguys[2]  = pilot.addRaw( "Pacifier", "Mercenary", origin )
   badguys[3]  = pilot.addRaw( "Vigilance", "Mercenary", origin )
   badguys[4]  = pilot.addRaw( "Phalanx", "Mercenary", origin )
   badguys[5]  = pilot.addRaw( "Lancelot", "Mercenary", origin )
   badguys[6]  = pilot.addRaw( "Lancelot", "Mercenary", origin )
   badguys[7]  = pilot.addRaw( "Vendetta", "Mercenary", origin )
   badguys[8]  = pilot.addRaw( "Vendetta", "Mercenary", origin )
   badguys[9]  = pilot.addRaw( "Shark", "Mercenary", origin )
   badguys[10] = pilot.addRaw( "Shark", "Mercenary", origin )

   badguys[1]:memory().formation = "wedge"  -- I love wedge formation
   attackhooks = {}
   for i,p in ipairs(badguys) do
      hook.pilot(p, "death", "baddie_death")
      hook.pilot(p, "jump", "baddie_jump")
      hook.pilot(p, "land", "baddie_land")
      attackhooks[i] = hook.pilot(p, "attacked", "baddie_attacked")
      if i >= 2 then
         p:setLeader(badguys[1])
      end
   end

   -- Targets
   targetList = { badguys[1], badguys[2], badguys[3] }

   stampede   = true  -- This variable ensure the enemies only runaway once
   maxbad     = #badguys
   deadbad    = 0
   deadtarget = 0
end

-- Spawn Tam and his crew
function spawnTam( origin )
   tamteam = {}
   tamteam[1] = pilot.add("Dvaered Vigilance", origin)[1]
   tamteam[2] = pilot.add("Dvaered Phalanx", origin)[1]
   tamteam[3] = pilot.add("Dvaered Vendetta", origin)[1]
   tamteam[4] = pilot.add("Dvaered Vendetta", origin)[1]

   for i,p in ipairs(tamteam) do
      p:setFaction("DHC")
   end
   hook.pilot(tamteam[1], "death", "tamDied")
end

-- Strafer sees the bountyhunters
function straferScans()
   escort[1]:comm( hostiMsg, true )
   for i = 1, 3 do
      badguys[i]:setVisible()
      badguys[i]:setHilight()
   end
   escort[1]:taskClear()
   escort[1]:follow( player.pilot(), true )
   misn.osdActive(3)

   -- Remove all the markers.
   for i, j in ipairs(marklist) do
      misn.markerRm( j )
   end
end

-- Strafer doesn't see anybody and returns
function straferReturns()
   escort[1]:comm( clearMsg, true )
   escort[1]:taskClear()
   escort[1]:follow( player.pilot(), true )
   stage = 0
   misn.osdActive(1)

   misn.markerRm( marklist[sysind+1] )
end

-- Baddies are in position: start the battle after a small delay
function badInPosition()
   badguys[1]:comm(preAmbushMsg)
   hook.timer(2000, "roastedTam")
   hook.timer(4000, "tamNattack")
end
function roastedTam()
   badguys[2]:comm(preAmbush1)
end
function tamNattack()
   spawnTam( tamPoint )
   badguys[1]:comm(baddie_signal)
   start_battle()
end

-- Death hooks
function escort_died1( )
   alive[1] = false
   tk.msg(failed_title,strdie_text)
   misn.finish(false)
end
function escort_died2( )
   alive[2] = false
end
function escort_died3( )
   alive[3] = false
end
function tamDied()
   tk.msg(failed_title, tamdie_text)
   misn.finish(false)
end

-- Start the battle
function baddie_attacked()
   for i, h in ipairs(attackhooks) do
      hook.rm(h)
   end
   badguys[1]:comm( baddie_taunt )
   start_battle()
end

-- Baddie death
function baddie_death( pilot )
   increment_baddie()

   -- See if it's one of the 3 targets
   if (elt_inlist( pilot, targetList ) > 0) then
      deadtarget = deadtarget + 1
      player.msg( killMsg )
      if deadtarget >= 3 then
         tk.msg(won_title, won_text)
         player.msg( killAll )
         stage = 2
         misn.osdActive(4)
      end
      if pilot == badguys[1] then -- It's Hamelsen: she escapes with a Schroedinger
         spawnHamelsen( badguys[1]:pos() )
      end
   end
end

function baddie_jump( pilot, jump )
   if (elt_inlist( pilot, targetList ) > 0) then
      if stage == 1 then -- It's the first one who escapes
         nextt = jump:dest()
         arm, sld, str = pilot:health() -- Store target values
         tem = pilot:temp()
         shi = pilot:ship()

         tk.msg(failed_title, jumpout_text:format(player.name(), nextt:name())) -- TODO: ensure the cleaning doesn't take too long
         stage = 3
         misn.osdDestroy()
         misn.osdCreate( osd_title, {osd_text5, osd_text6:format(nextt:name())} )
         misn.markerAdd( nextt, "high" )
      else -- You won't follow several enemies
         tk.msg(failed_title, tflee_text)
         misn.finish(false)
      end
   end
   increment_baddie()
end

-- Actually, after checking, this is very unlikely to happen...
function baddie_land( pilot, planet )
   if (elt_inlist( pilot, targetList ) > 0) then
      if stage == 1 then -- It's the first one who escapes
         nextt = planet
         tk.msg(failed_title, land_text:format(player.name(), nextt:name()))
         stage = 5

         shi = pilot:ship()
         misn.osdDestroy()
         misn.osdCreate( osd_title, {osd_text5, osd_text7:format(nextt:name())} )
      else -- You won't follow several enemies
         tk.msg(failed_title, tflee_text)
         misn.finish(false)
      end
   end
   increment_baddie()
end

function start_battle()
   badguys[1]:control(false)
   for i,p in ipairs(badguys) do
      p:setFaction("Warlords")
   end
   for i,p in ipairs(escort) do
      p:control(false)
   end
   escort[1]:setNoDeath() -- That's not very elegant...
   hook.pilot( escort[1], "attacked", "strafer_choosePoint" )
   hook.pilot( escort[1], "idle", "strafer_choosePoint" )
   --escort[1]:setNoDisable()
end

-- Strafer is attacked: he runs away around the system
function strafer_choosePoint()
   escort[1]:control()
   escort[1]:taskClear()
   local sysrad = rnd.rnd() * system.cur():radius()
   local angle = rnd.rnd() * 2 * math.pi
   escort[1]:moveto( vec2.new(math.cos(angle) * sysrad, math.sin(angle) * sysrad), false, false )
end

function increment_baddie()
   deadbad = deadbad+1
   if deadbad >= maxbad/2 and stampede then -- time for stampede
      escort[1]:comm( stampedeMsg, true )
      stampede = false
      for i,p in ipairs(badguys) do
         if p:exists() then
            p:control(true)
            p:runaway( player.pilot() )
         end
      end
   end
   if deadbad >= maxbad then -- System cleaned
      escort[1]:comm( killedMsg, true )
      if stage == 3 then
         stage = 4
         misn.osdActive(2)
      end
      if stage == 5 then
         stage = 6
         misn.osdActive(2)
      end
   end
end

-- Killed the last enemy after the pursuit
function lastOne_died()
   player.msg( killAll )
   stage = 2
   misn.osdActive(4)
   if shi:nameRaw() == "Kestrel" then
      spawnHamelsen( target:pos() )
   end
end

-- Pursued enemy jumped out (this is actually kind of unlikely, but...)
function lastOne_jumped( pilot, jump )
   player.msg(pursuitMsg)
   stage = 4
   nextt = jump:dest()
   arm, sld, str = pilot:health()
   tem = pilot:temp()
   shi = pilot:ship()
end

-- Pursued enemy landed (this is actually kind of unlikely, but...)
function lastOne_landed( pilot, planet )
   player.msg(pursuitMsg)
   stage = 6
   nextt = planet
   shi = pilot:ship()
end

-- Compute the effective reward from nb of alive pilots
function compute_reward()
   effective_credits = credits_03
   for i, j in ipairs(alive) do
      if j then
         effective_credits = effective_credits + credits_03
      end
   end
end

-- Spawn Hamelsen and make her escape
function spawnHamelsen( origin )
   hamelsen = pilot.add( "Civilian Schroedinger", origin )[1]
   hamelsen:setInvincible()
   hamelsen:setFaction("Warlords")
   hamelsen:rename("Colonel Hamelsen")

   hamelsen:comm( ham_runaway )
   hamelsen:control()
   hamelsen:runaway( player.pilot(), true )
end

-- Pay and finish the mission
function payNfinish()
   player.pay(effective_credits)
   shiplog.createLog( "frontier_war", _("Frontier War"), _("Dvaered") )
   shiplog.appendLog( "frontier_war", log_text )
   misn.finish(true)
end

-- Test if an element is in a list of destinations
function elt_dest_inlist( elt, list )
   for i, elti in ipairs(list) do
      if elti:dest() == elt then
         return i
      end
   end
   return 0
end
