--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Diplomacy">
 <unique />
 <priority>2</priority>
 <chance>30</chance>
 <done>Dvaered Escape</done>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <cond>var.peek("dv_pirate_debt") == false</cond>
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
local lmisn = require "lmisn"
local fw = require "common.frontier_war"
require "proximity"
local fmt = require "format"
local pir = require "common.pirate"

local attackhooks, badguys, escort, hamelsen, tamteam, target, targetList, toldya -- Non-persistent state
local compute_reward, elt_dest_inlist, increment_baddie, payNfinish, spawnBaddies, spawnEscort, spawnHamelsen, start_battle -- Forward-declared functions

escort_hailed = fw.escort_hailed -- common hooks

-- Mission constants
local destpla, destsys = spob.getS("Mannannan")
local lore_text = {}

lore_text[1] = _([["Good question, citizen! It is likely the intelligence services of the other nations know that General Klank and I are involved in the invasion project. Though, they probably don't know how imminent it is. It's likely they hired henchmen to hound or outright stop us. We have identified some of the henchmen actually. They are the kind of independent mercenaries we find in any kind of shady operations... A bit like you in fact."]])

lore_text[2] = _([["Aha! You're worrying about the soldiers of the warlord we killed last time? Don't worry too much about them, most of them did not stay unemployed for long. When a warlord dies, his surviving followers are sent to the Dvaered Military Reserve, and they apply for positions in other warlords' armies, or in the DHC. The only one I'm worried about is Colonel Hamelsen: it will be very difficult for her to find a position with a warlord since they prefer to have people they know at higher ranks, and the DHC only rarely recruits new colonels.
   "That's a pity actually because she is probably the most talented officer of her generation. Do you know that she was the first one since the Independence War who managed to become an ace before the end of her training?"]])

lore_text[3] = _([[You ask Major Tam to continue his story about when he used to be a fighter pilot at the DHC base on Rhaana. A small smile appears on his face and he turns his eyes to the cieling.
   "Do you want me to tell you how I became an ace? Oh yes, you do. So. To become a Dvaered ace, one has to score four confirmed fighter victories. An confirmed victory is recorded when you, alone, destroy any kind of enemy warship. As we tend to work in squads, we pretty often destroy ships without scoring any confirmed victories. My first victory was a Pirate Hyena. The pilot was probably drunk or something. They headed right towards my Vendetta, broadcasting stupid taunts. My sergeant was nice and left the Hyena all to me. My second victory was against a freak... I mean a local warlord's pilot who claimed she could kill any DHC pilot in fair fight. Apparently, it was not true.
   "But the two others... Actually those are a much longer story. I'll tell you another time maybe."]])

lore_text[4] = _([["Oh, the captain is doing well. He spent a few periods at the hospital, but it was not so serious. The rest of his squad, I'm not too sure. I don't know them very well actually. They are more like statistics to me. I know it's bad, but hey, I have so many things to think about right now.
   "Oh, and we went to the funeral of private Amadeus Tronk, officially killed in training, of course. It was nice. Leblanc's squadron made an aerial display with a mace rocket concerto for the occasion, and we organized a fight to death between a convicted criminal and a gladiator in General Klank's private arena. Not only to honour the memory of the dead warrior, but also to have some fun."]])

local flee_text = _("You were supposed to wait for Strafer to scan the system, and then if needed, kill the hostiles.")

local tflee_text = _([[Two of your targets went away. If there was only one, it could be salvageable, but now, the mission has failed.]])

function create()
   mem.startsys = system.cur()
   local d = mem.startsys:jumpDist( destsys )

   -- The starting point should not be too close from there
   if d < 3 then
      misn.finish(false)
   end

   mem.path = system.cur():jumpPath(destsys)
   mem.ambushsys = mem.path[ rnd.rnd(2,3) ]:dest()

   if not misn.claim(mem.ambushsys) then
      misn.finish(false)
   end

   misn.setNPC(_("Major Tam"), fw.portrait_tam, _("The major seems to be waiting for you."))
end

function accept()
   if not tk.yesno( _("Ready for some diplomacy?"), fmt.f(_([[Major Tam seems to be in a very good mood: "Hello, citizen {player}! Have you got plans for tonight?" You fear he might invite you to a brawl-party, or some other kind of event Dvaered usually organize when they want to have good time, but he continues: "Because I've got a very important mission for you.
   "Let me explain: I am currently in the middle of a secret diplomatic campaign with the Imperials, House Goddard, and the Sirii. But a group of assassins is constantly harassing me as I travel. This is very annoying and we have already lost two pilots of Leblanc's squadron in these ambushes, including the second in command. The attackers use a mix of fighters supported by medium ships. They even have a Kestrel!
   "So we have decided to deal with them before we proceed with the diplomatic meetings. Are you in?"]]), {player=player.name()}) ) then
      tk.msg(_("Another time maybe"), _([[Major Tam really seems disappointed. "As you wish, citizen..."]]))
      return
   end
   tk.msg(_("This is how we deal with our problems"), fmt.f(_([["You won't regret it!" Tam says... "Unless you're killed in the process I guess... Anyway, I will go to {pnt} in {sys} in my Vigilance. No doubt the assassins will set up an ambush somewhere on the way. You and a few pilots from the Special Operations Force (SOF) will locate them and wait for me to jump in. When they spring the ambush, the assassins will be caught in our crossfire, and we will annihilate them to the last ship. Actually, that sounds much more enticing than a three-period-long meeting with rogue Sirian diplomats, doesn't it?
   "Our side will not have much firepower, so I'm counting on you to arrive with a lot of armour and guns (but you should still be able to outrun mid-range destroyers). Because you two worked so well in the past, you'll again team up with Lieutenant Strafer, and two other pilots. But the SOF does not have an endless supply of pilots, so your reward will depend on how many of them come back. Strafer will fly a scout with high performance sensors. He will be our eyes, so his death would mean the end of the mission.
   "If you have any additional questions, I'll stay at the bar until we take off."]]), {pnt=destpla, sys=destsys}))

   misn.accept()
   misn.osdCreate( _("Dvaered Diplomacy"), {_("Go to next system"), _("Wait until Strafer has scanned the system"), _("Wait for Tam and destroy the highlighted hostile ships"), _("Land anywhere to collect your pay") } )
   misn.setDesc(_("You take part in an operation to trap and destroy a group of well armed henchmen who are after Major Tam."))
   misn.setReward(_("It depends how many of your wingmen come back."))

   -- Markers
   mem.marklist = {}
   mem.marklist[1] = misn.markerAdd(system.cur(), "low")
   for i,jp in ipairs(mem.path) do
      mem.marklist[i+1] = misn.markerAdd(jp:dest(), "low")
   end

   mem.stage = 0

   mem.previous = spob.cur()
   mem.enterhook = hook.enter("enter")
   mem.landhook = hook.land("land")

   -- Stores the state of the escort
   mem.alive = {true,true,true}

   toldya = {false,false,false,false}
   misn.npcAdd("discussWithTam", _("Major Tam"), fw.portrait_tam, _("The major seems to be waiting for you."))
end

-- Discussions with Major Tam at the bar
function discussWithTam()
   local c = tk.choice( _("Ask Major Tam some questions"), _("What do you want to ask Major Tam?"), _("Ask about the assassins"), _("Ask about Battleaddict's soldiers"), _("Ask about his deeds as fighter pilot"), _("Ask for news about Hamfresser"), _("Goodbye, we meet in space.") )
   if c <= 4 then
      if toldya[c] then
         tk.msg( _("Ask Major Tam some questions"), _("Hey, do you think I'm some kind of amnesic? You literally asked this question 10 seconds ago!") )
      else
         tk.msg( _("Ask Major Tam some questions"), lore_text[c] )
         toldya[c] = true
      end
   end
end

function enter()
   local ambJp

   -- Entering the next system of the mem.path
   if mem.stage == 0 then
      spawnEscort(mem.previous)

      mem.sysind = elt_dest_inlist(system.cur(), mem.path) -- Save the index of the system in the list (useful to remove the marker later)
      if (mem.sysind > 0) or (system.cur() == mem.startsys) then
         mem.stage = 1
         misn.osdActive(2)

         -- Decide from where the ambushers come
         if system.cur() == destsys then
            mem.ambStart = destpla
         else
            mem.ambStart = lmisn.getNextSystem(system.cur(), destsys)
            ambJp = jump.get(system.cur(), mem.ambStart) -- We assume there are no one-way jumps
         end

         escort[1]:taskClear()
         escort[1]:moveto( ambJp:pos() )  -- Let's say Strafer knows where they are supposed to come from...

         if system.cur() == mem.ambushsys then
            for k,f in ipairs(pir.factions) do
               pilot.toggleSpawn(f)
               pilot.clearSelect(f)
            end
            spawnBaddies( mem.ambStart )

            -- Find the waiting point: on the line mem.tamPoint -> ambJp, at 3000 of mem.tamPoint
            -- Normally, mem.tamPoint and absStart are never the same
            mem.tamPoint = lmisn.getNextSystem(system.cur(), mem.startsys)
            local tamJp = jump.get(system.cur(), mem.tamPoint)
            local PA = ambJp:pos() - tamJp:pos()
            local PW = PA/PA:mod() * 3000
            mem.waitPoint = tamJp:pos() + PW
            badguys[1]:control()
            badguys[1]:moveto(mem.waitPoint)
            hook.timer(0.5, "proximity", {location = mem.waitPoint, radius = 3000, funcname = "badInPosition", focus = badguys[1]})

            escort[1]:taskClear()
            escort[1]:follow( badguys[1] ) -- Yes, he's not supposed to know where they are, but it's safer for the trigger
            hook.timer(0.5, "proximityScan", {anchor = badguys[1], funcname = "straferScans", focus = escort[1]})
         else
            hook.timer(0.5, "proximity", {location = ambJp:pos(), radius = 4000, funcname = "straferReturns", focus = escort[1]})
         end
      end

   -- Illegitimate system entrances
   elseif mem.stage == 1 then
      tk.msg(_("What are you doing here?"), flee_text)
      misn.finish(false)
   elseif mem.stage == 3 or mem.stage == 5 then
      tk.msg( _("What are you doing here?"), _("You were supposed to clear the system before pursuing the fleeing ship.") )
      misn.finish(false)
   elseif mem.stage == 6 then
      tk.msg(_("What are you doing here?"), fmt.f(_("You were supposed to land on {pnt}."), {pnt=mem.nextt}))
      misn.finish(false)

   -- Enter after a fleeing enemy
   elseif mem.stage == 4 then
      if system.cur() == mem.nextt then
         target = pilot.add( mem.shi:nameRaw(), "Mercenary", mem.previous )
         target:setHealth( mem.arm, mem.sld, mem.str )
         target:setTemp( mem.tem )
         target:setHilight()
         target:setFaction(fw.fct_warlords())

         hook.pilot(target, "death","lastOne_died")
         hook.pilot(target, "jump","lastOne_jumped")
         hook.pilot(target, "land","lastOne_landed")
      else
      tk.msg(_("What are you doing here?"), fmt.f(_("You were supposed to jump to {sys}."), {sys=mem.nextt}))
         misn.finish(false)
      end
   end

   mem.previous = system.cur()
end

function land()
   -- Illegitimate landing
   if mem.stage == 1 then
      tk.msg(_("What are you doing here?"), flee_text)
      misn.finish(false)

   -- Land for reward
   elseif mem.stage == 2 then
      compute_reward()
      tk.msg( _("Mission accomplished"), fmt.f(_("Your mission is a success, except for the escape of the enemy leader, Colonel Hamelsen. You can now collect your {credits} reward."), {credits=fmt.credits(mem.effective_credits)}) )
      payNfinish()

   -- More illegitimate landings
   elseif mem.stage == 3 or mem.stage == 5 then
      tk.msg( _("What are you doing here?"), _("You were supposed to clear the system before pursuing the fleeing ship.") )
      misn.finish(false)
   elseif mem.stage == 4 then
      tk.msg(_("What are you doing here?"), fmt.f(_("You were supposed to jump to {sys}."), {sys=mem.nextt}))
      misn.finish(false)

   -- Landing after an enemy (victory as well)
   elseif mem.stage == 6 then
      if spob.cur() == mem.nextt then
         compute_reward()
         tk.msg( _("End of the hunt"), fmt.f(_([[You land and walk around the spacedock, in search of your target's ship. You finally see it. A mighty {ship}, covered in the score marks of the battle that just occurred. You hide yourself behind some crates near the ship and wait for the pilot to come back and take off in order to finish your job in space.
   When looking closer at the ship, you see both ancient and recent marks on the hull, caused by all kinds of weapons during the lifetime of the ship. Among the ship's scars, you see a twisted welding around the ship's nose, filled with bubbles and think: "Damn! They had to deal with the same old deficient welding android that fixed my airlock on Alteris last time!"
   Suddenly, you realize someone whispers behind you "Hey, {player}, you're blocking my firing line!" You turn around and see nothing but a deformed crate that continues to speak: "It's me, Sergeant Nikolov. In the box. Hide yourself better or you will ruin our mission." You then remember that she is a member of the space infantry commandos, and Hamfresser's second in command. Tam probably sent her to execute the enemy pilot.]]), {ship=mem.shi, player=player.name()}) )
         if mem.shi:nameRaw() == "Kestrel" then -- it's Hamelsen and she escapes
            tk.msg( _("End of the hunt"), fmt.f(_([[A few moments later, you hear a message from Nikolov's radio: "Tam here. The target escaped and won't come back to the ship. Clear the spacedock." The spacemarines emerge from the crates and disappear in a flash, while you start heading to the bar. On the way, you meet Strafer who explains the situation: "We identified the hostile pilot: it was Colonel Hamelsen, Battleaddict's former second in command, but she got away using one of her other ships and we lost her track.
   "Poor woman. It's hard to get a new post when you're the second in command of a dead warlord, you know. So I guess someone managed to hire her to assassinate the major. Anyway, I guess you should have received your payment of {credits} by now."]]), {credits=fmt.credits(mem.effective_credits)}) )
         else -- No pity for non-Hamelsen henchmen
            tk.msg( _("End of the hunt"), _([[A bit later, you see a woman coming from the empty corridor, anxiously looking behind her and pulling a key out of her pocket. While still approaching the ship, she presses the key's button and the ship beeps. At that very moment, a sudden and loud fracas erupts from all around you shakes your stomach. The pilot falls without a word. Nikolov and two other soldiers emerge from the crates. The sergeant approaches the pilot, kneels and takes her pulse. She thoughtfully looks at her face "Damn! She looked like a nice person..." And then addresses the soldiers: "All right, folks we pack up!" and the unit enters the ship with the body and takes off.
   You stay alone, on the empty dock, with nothing but your thoughts. Even the broken crates have been picked up by the commandos. You think about all the causes that pilot must have served in her life. The just causes, the evil ones... and all the others. "Meh," you think "killing people in space is definitely much better for morale."]]), "portraits/neutral/female1.webp" )
            tk.msg( _("End of the hunt"), fmt.f(_([[When you finally go to the bar to think about something else, you get notified on your holowatch that {credits} have been transferred to your account. The leader of the ambushers has been identified: it's Colonel Hamelsen, who used to work for Battleaddict before his death. Unfortunately, the Colonel has escaped.]]), {credits=fmt.credits(mem.effective_credits)}) )
         end
         payNfinish()
      else
         tk.msg(_("What are you doing here?"), fmt.f(_("You were supposed to land on {pnt}."), {pnt=mem.nextt}))
         misn.finish(false)
      end
   end
   mem.previous = spob.cur()
end

-- Spawn the escort
function spawnEscort( origin )
   escort = {}
   if mem.alive[1] then
      escort[1] = pilot.add( "Schroedinger", fw.fct_dhc(), origin, _("Lieutenant Strafer") )

      -- Give him nice outfits
      escort[1]:outfitRm("all")
      escort[1]:outfitRm("cores")
      escort[1]:outfitAdd("Nexus Light Stealth Plating")
      escort[1]:outfitAdd("Tricon Zephyr Engine")
      escort[1]:outfitAdd("Milspec Orion 2301 Core System")
      escort[1]:outfitAdd("Scanning Combat AI")
      escort[1]:outfitAdd("Nexus Stealth Coating")
      escort[1]:outfitAdd("Hellburner")
      escort[1]:outfitAdd("Milspec Scrambler")
      escort[1]:outfitAdd("Improved Stabilizer")
      escort[1]:outfitAdd("Gauss Gun")
      escort[1]:setHealth(100,100)
      escort[1]:setEnergy(100)
      escort[1]:setFuel(true)
      escort[1]:setHilight()
      escort[1]:setVisplayer()

      escort[1]:memory().angle = math.rad(225)
      escort[1]:memory().radius = 200
      escort[1]:memory().shield_run = 90
      escort[1]:memory().shield_return = 95

      hook.pilot(escort[1], "hail", "escort_hailed") -- TODO: see if we want that for Strafer
      hook.pilot(escort[1], "death", "escort_died1")

      escort[1]:control()
      escort[1]:follow(player.pilot(), true)
   end

   if mem.alive[2] then
      escort[2] = pilot.add( "Vendetta", fw.fct_dhc(), origin )
      hook.pilot(escort[2], "hail", "escort_hailed")
      hook.pilot(escort[2], "death", "escort_died2")
      escort[2]:control()
      escort[2]:follow(player.pilot(), true)
      escort[2]:memory().angle = math.rad(225)
      escort[2]:setHilight()
      escort[2]:setVisplayer()
   end
   if mem.alive[3] then
      escort[3] = pilot.add( "Phalanx", fw.fct_dhc(), origin )
      hook.pilot(escort[3], "hail", "escort_hailed")
      hook.pilot(escort[3], "death", "escort_died3")
      escort[3]:control()
      escort[3]:follow(player.pilot(), true)
      escort[3]:memory().angle = math.rad(135)
      escort[3]:setHilight()
      escort[3]:setVisplayer()
   end
end

-- Spawn the bad guys
function spawnBaddies( origin )
   badguys = {}
   -- They're mercenaries to avoid getting too high outfits
   badguys[1]  = pilot.add( "Kestrel", "Mercenary", origin )
   badguys[2]  = pilot.add( "Pacifier", "Mercenary", origin )
   badguys[3]  = pilot.add( "Vigilance", "Mercenary", origin )
   badguys[4]  = pilot.add( "Phalanx", "Mercenary", origin )
   badguys[5]  = pilot.add( "Lancelot", "Mercenary", origin )
   badguys[6]  = pilot.add( "Lancelot", "Mercenary", origin )
   badguys[7]  = pilot.add( "Vendetta", "Mercenary", origin )
   badguys[8]  = pilot.add( "Vendetta", "Mercenary", origin )
   badguys[9]  = pilot.add( "Shark", "Mercenary", origin )
   badguys[10] = pilot.add( "Shark", "Mercenary", origin )

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

   mem.stampede   = true  -- This variable ensure the enemies only runaway once
   mem.maxbad     = #badguys
   mem.deadbad    = 0
   mem.deadtarget = 0
end

-- Spawn Tam and his crew
local function spawnTam( origin )
   tamteam = {}
   tamteam[1] = pilot.add( "Dvaered Vigilance", "Dvaered", origin )
   tamteam[2] = pilot.add( "Dvaered Phalanx", "Dvaered", origin )
   tamteam[3] = pilot.add( "Dvaered Vendetta", "Dvaered", origin )
   tamteam[4] = pilot.add( "Dvaered Vendetta", "Dvaered", origin )

   local fdhc = fw.fct_dhc()
   for i,p in ipairs(tamteam) do
      p:setFaction( fdhc )
   end
   hook.pilot(tamteam[1], "death", "tamDied")
end

-- Strafer sees the bountyhunters
function straferScans()
   escort[1]:comm( _("Hostiles spotted!"), true )
   for i = 1, 3 do
      badguys[i]:setVisible()
      badguys[i]:setHilight()
   end
   escort[1]:taskClear()
   escort[1]:follow( player.pilot(), true )
   misn.osdActive(3)

   -- Remove all the markers.
   for i, j in ipairs(mem.marklist) do
      misn.markerRm( j )
   end
end

-- Strafer doesn't see anybody and returns
function straferReturns()
   escort[1]:comm( _("No hostiles here. We can proceed to next system."), true )
   escort[1]:taskClear()
   escort[1]:follow( player.pilot(), true )
   mem.stage = 0
   misn.osdActive(1)

   misn.markerRm( mem.marklist[mem.sysind+1] )
end

-- Baddies are in position: start the battle after a small delay
function badInPosition()
   badguys[1]:comm(_("All right, let's wait for Tam"))
   hook.timer(2.0, "roastedTam")
   hook.timer(4.0, "tamNattack")
end
function roastedTam()
   badguys[2]:comm(_("There will be roasted Tam for lunch today!"))
end
function tamNattack()
   spawnTam( mem.tamPoint )
   badguys[1]:comm(_("Tam is in place, folks! Don't miss him, this time!"))
   start_battle()
end

-- Death hooks
function escort_died1( )
   mem.alive[1] = false
   tk.msg(_("This is not good"),_([[You suddenly realize that your radar no longer displays Strafer's sensor data. This means the Lieutenant got killed. As a result, the fleet is flying blind and the mission has failed.]]))
   misn.finish(false)
end
function escort_died2( )
   mem.alive[2] = false
end
function escort_died3( )
   mem.alive[3] = false
end
function tamDied()
   tk.msg(_("This is not good"), _([[The blinking cross marked "Major Tam" on your radar screen suddenly turns off. At first you think your screen is buggy and hit it with your open hand, but soon, you realize that the radar works perfectly well. This means that the major's ship was annihilated by the ambushers, and therefore your mission is a miserable failure.]]))
   misn.finish(false)
end

-- Start the battle
function baddie_attacked()
   for i, h in ipairs(attackhooks) do
      hook.rm(h)
   end
   badguys[1]:comm( _("What about I cook you to death?") )
   start_battle()
end

-- Baddie death
function baddie_death( pilot )
   increment_baddie()

   -- See if it's one of the 3 targets
   if (fw.elt_inlist( pilot, targetList ) > 0) then
      mem.deadtarget = mem.deadtarget + 1
      player.msg( _("One of the targets was destroyed!") )
      if mem.deadtarget >= 3 then
         tk.msg(_("All targets eliminated"), _([[All three primary targets have been eliminated. The remaining ones are no longer a threat. You can land to get your reward.]]))
         player.msg( _("The last target was destroyed! You can now land.") )
         mem.stage = 2
         misn.osdActive(4)
      end
      if pilot == badguys[1] then -- It's Hamelsen: she escapes with a Schroedinger
         spawnHamelsen( badguys[1]:pos() )
      end
   end
end

function baddie_jump( pilot, jump )
   if (fw.elt_inlist( pilot, targetList ) > 0) then
      if mem.stage == 1 then -- It's the first one who escapes
         mem.nextt = jump:dest()
         mem.arm, mem.sld, mem.str = pilot:health() -- Store target values
         mem.tem = pilot:temp()
         mem.shi = pilot:ship()

         tk.msg(_("This is not good"), fmt.f(_([[While your sensors lose track of the target you were supposed to kill, you expect to receive a mission failure message, but instead, you hear Tam's ship communication: "One of them escaped. Continue destroying the others. Afterwards, {player}, you will jump to {sys} and destroy that ship."]]), {player=player.name(), sys=mem.nextt})) -- TODO: ensure the cleaning doesn't take too long
         mem.stage = 3
         misn.osdDestroy()
         misn.osdCreate( _("Dvaered Diplomacy"), {
            _("Clear the current system"),
            fmt.f(_("Jump to {sys} to follow your target"), {sys=mem.nextt}),
         } )
         misn.markerAdd( mem.nextt, "high" )
      else -- You won't follow several enemies
         tk.msg(_("This is not good"), tflee_text)
         misn.finish(false)
      end
   end
   increment_baddie()
end

-- Actually, after checking, this is very unlikely to happen...
function baddie_land( pilot, planet )
   if (fw.elt_inlist( pilot, targetList ) > 0) then
      if mem.stage == 1 then -- It's the first one who escapes
         mem.nextt = planet
         tk.msg(_("This is not good"), fmt.f(_([[While your sensors lose track of the target you were supposed to kill, you expect to receive a mission failure message, but instead, you hear Tam's ship communication: "One of them escaped. Continue destroying the others. Afterwards, {player}, you will land on {pnt} to keep track on the pilot."]]), {player=player.name(), pnt=mem.nextt}))
         mem.stage = 5

         mem.shi = pilot:ship()
         misn.osdDestroy()
         misn.osdCreate( _("Dvaered Diplomacy"), {
            _("Clear the current system"),
            fmt.f(_("Land on {pnt} to follow your target"), {pnt=mem.nextt}),
         } )
      else -- You won't follow several enemies
         tk.msg(_("This is not good"), tflee_text)
         misn.finish(false)
      end
   end
   increment_baddie()
end

function start_battle()
   local fwarlords = fw.fct_warlords()
   badguys[1]:control(false)
   for i,p in ipairs(badguys) do
      p:setFaction( fwarlords )
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
   escort[1]:moveto( vec2.newP(sysrad, rnd.angle()), false, false )
end

function increment_baddie()
   mem.deadbad = mem.deadbad+1
   if mem.deadbad >= mem.maxbad/2 and mem.stampede then -- time for stampede
      escort[1]:comm( _("Hostiles are running away. Don't let them escape!"), true )
      mem.stampede = false
      for i,p in ipairs(badguys) do
         if p:exists() then
            p:control(true)
            p:runaway( player.pilot() )
         end
      end
   end
   if mem.deadbad >= mem.maxbad then -- System cleaned
      escort[1]:comm( _("Hostiles eliminated."), true )
      if mem.stage == 3 then
         mem.stage = 4
         misn.osdActive(2)
      end
      if mem.stage == 5 then
         mem.stage = 6
         misn.osdActive(2)
      end
   end
end

-- Killed the last enemy after the pursuit
function lastOne_died()
   player.msg( _("The last target was destroyed! You can now land.") )
   mem.stage = 2
   misn.osdActive(4)
   if mem.shi:nameRaw() == "Kestrel" then
      spawnHamelsen( target:pos() )
   end
end

-- Pursued enemy jumped out (this is actually kind of unlikely, but...)
function lastOne_jumped( pilot, jump )
   player.msg(_("The target ran away again: continue the pursuit."))
   mem.stage = 4
   mem.nextt = jump:dest()
   mem.arm, mem.sld, mem.str = pilot:health()
   mem.tem = pilot:temp()
   mem.shi = pilot:ship()
end

-- Pursued enemy landed (this is actually kind of unlikely, but...)
function lastOne_landed( pilot, planet )
   player.msg(_("The target ran away again: continue the pursuit."))
   mem.stage = 6
   mem.nextt = planet
   mem.shi = pilot:ship()
end

-- Compute the effective reward from nb of alive pilots
function compute_reward()
   mem.effective_credits = fw.credits_03
   for i, j in ipairs(mem.alive) do
      if j then
         mem.effective_credits = mem.effective_credits + fw.credits_03
      end
   end
end

-- Spawn Hamelsen and make her escape
function spawnHamelsen( origin )
   hamelsen = pilot.add( "Schroedinger", "Independent", origin, _("Colonel Hamelsen") )
   hamelsen:setInvincible()
   hamelsen:setFaction(fw.fct_warlords())

   hamelsen:comm( _("You won't take me alive! Never!") )
   hamelsen:control()
   hamelsen:runaway( player.pilot(), true )
end

-- Pay and finish the mission
function payNfinish()
   player.pay(mem.effective_credits)
   shiplog.create( "frontier_war", _("Frontier War"), _("Dvaered") )
   shiplog.append( "frontier_war", _("A group of assasins, who were after Major Tam, have been trapped and killed. For now, we don't know who paid them, but they were led by Colonel Hamelsen, who managed to escape.") )
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
