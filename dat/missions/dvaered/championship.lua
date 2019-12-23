--[[
   
   The player takes part into a challemge between fighter pilots in Dvaered space, he has to defeat 5 opponents to win.
   
   Stages :
   0) Way to positions
   1) Begin of the fight
   2) Player won
   3) End of competition

--]]

--Needed scripts
include "dat/scripts/numstring.lua"
include "dat/scripts/proximity.lua"
include "dat/factions/equip/generic.lua"

title = {}
text = {}
comptitle = {}
comptext = {}
osd_msg = {}
npc_desc = {}
bar_desc = {}

title[1] = _("Do you want to take part to a challenge?")
text[1] = _([["Hello, I'm a member of the staff of the Dvaered dogfight challenge. Here are the rules: you need to take off with a fighter-class ship and to join your starting mark. After that, you will try to disable your opponent. Don't kill him or the security staff won't forgive it. It is forbidden to use missiles, so you won't be allowed to have some while taking off. It's also forbidden to board the opponent's ship and to attack him before the signal is given. You are not allowed to land on an other planet or to jump away during the championship.
   We are looking for pilots, are you in?"]])
   
refusetitle = _("Sorry, not interested")
refusetext = _([["That's your choice, goodbye" the person says.]])

dismisstitle = _("You are dismissed")
missiletext = _("You aren't allowed to use missiles")
fightertext = _("You had to use a fighter")
fleetext = _("You weren't supposed to go away")

title[2] = _("Let's go")
text[2] = _([[For this round, your opponent is %s. Remember: use a fighter with no launchers. You still have to defeat %s opponents to win.]])

title[3] = _("You won this round")
text[3] = _([["Congratulations", the staff says to you. Come back when you are ready for the next round!]])

title[4] = _("You are the new champion")
text[4] = _([[Congratulations! The staff pays you %s credits.]])

title[5] = _("You are the vice-champion")
text[5] = _([[Congratulations! The staff pays you %s credits.]])

title[6] = _("Thanks for playing")
text[6] = _([[The staff pays you %s credits.]])

comptitle[1] = _("I am here to win the championship")
comptext[1] = _([["Hello," the pilot says, "I came to become the new champion of this cycle! I prepared myself since the first day I piloted a ship. Trust me, I'm nearly invincible, and my Vendetta is indestructible.
   You know that the Vendetta is the best fighter in this part of the galaxy? It's the reason why every pilot who won this championship had one."]])

comptitle[2] = _("Hello")
comptext[2] = _([["Are you here for the Dvaered dogfight championship? I am a competitor. You know, I fly a Shark, so I don't hope to win lots of rounds... But I still enjoy the battle. Every cycle, a Dvaered pilot wins. Do you know why? It's because the rules of the championship advantage heavy armoured, well armed fighters, like the Vendetta. Imperial pilots are used to do electronical war, with guided missiles and stealth ships while Dvaered pilots only understand dogfight."]])

comptitle[3] = _("Imperial Pilot")
comptext[3] = _([["What a pity, in my squad, I am the best, I trained cycles to be able to take down these stupids Vendettas with my missiles before they only see my Lancelot on their radar. But in this championship, only armor and firepower are usefull."]])

comptitle[4] = _("Dvaered Pilot")
comptext[4] = _([["Nice to see you," the soldier says, "I am a Vendetta pilot. I hope I will win this time, because for us, being the champion here means that you become member of the senior staff, witch makes you closer to Dvaered High Command. Who knows, maybe one day I will become a Warlord..."]])

comptitle[5] = _("Obvious Pirate")
comptext[5] = _([["Hi, I'm... err... I'm an independant pilot. I'm here to take part to the challenge, and to see the best Dvaered Vendetta pilots in motion. In my job, it helps to know how they fly."]])

comptitle[6] = _("I am here to win the championship")
comptext[6] = _([["Hello," the pilot says, "I came to become the new champion of this cycle! I prepared myself since the first day I piloted a ship. Trust me, I'm nearly invincible, and my Vendetta is indestructible.
   Do you know who I am? I am the famous independant pilot who helped Dvaered High Command to destroy the FLF base in the nebula. I managed to defeat lots of FLF fighters with my ship! Another day, I will tell you my adventures, but now, I need to concentrate myself."]])
   
-- Mission details
misn_title = _("The Dvaered Championship")
misn_reward = _("From 50k to 1.6m credits, depending on your rank")
misn_desc = _("You take part to a fight contest. Try to do your best!")

-- NPC
npc_desc[1] = _("An official")
bar_desc[1] = _("This person seems to be looking for suitable combat pilots.")

npc_desc[2] = _("Pilot")
bar_desc[2] = _("This pilot looks very self-confident")

npc_desc[3] = _("Pilot")
bar_desc[3] = _("This pilot seems to work as a private combat pilot")

npc_desc[4] = _("Imperial pilot")
bar_desc[4] = _([[This man is one of the few imperial pilots who are good enough to have the right to keep their uniform while taking part to piloting challenges.]])

npc_desc[5] = _("Dvaered pilot")
bar_desc[5] = _([[This woman surely works as a Vendetta pilot.]])

npc_desc[6] = _("Strange pilot")
bar_desc[6] = _([[This man looks like a pirate, but strangely enough, the authorities seems not to worry about him.]])

-- OSD
osd_title = _("The Dvaered Championship, round %n")
final_title = _("The Dvaered Championship, final")
osd_msg[1] = _("Go to the starting point")
osd_msg[2] = _("Disable your opponent, DON'T KILL HIM")
osd_msg[3] = _("Land on %s")

--mark
mark_name = _("START")

function create ()
   
   --Change here to change the planet and the system
   sysname = "Dvaer"
   planame = "Dvaer Prime"
   missys = system.get(sysname)
   mispla = planet.get(planame)
   
   --No system claim

   portrait = {"neutral/female1", "neutral/male1", "neutral/thief1", "neutral/thief2", "neutral/thief3" }
   officialFace = portrait[rnd.rnd(1, #portrait)]
   official = misn.setNPC(npc_desc[1], officialFace)
   misn.setDesc(bar_desc[1])

end

function populate_bar() --add some random npcs

   portrait = {"neutral/female1", "neutral/male1", "neutral/thief1", "neutral/thief2", "neutral/thief3" }

   if rnd.rnd() < 0.5 then
      misn.npcAdd("competitor1", npc_desc[2], portrait[rnd.rnd(1, #portrait)], bar_desc[2])
   end
   if rnd.rnd() < 0.5 then
      misn.npcAdd("competitor2", npc_desc[3], portrait[rnd.rnd(1, #portrait)], bar_desc[3])
   end
   if rnd.rnd() < 0.5 then
      misn.npcAdd("competitor3", npc_desc[4], "empire/empire1", bar_desc[4])
   end
   if rnd.rnd() < 0.5 then
      misn.npcAdd("competitor4", npc_desc[5], "dvaered/dv_military_f1", bar_desc[5])
   end
   if rnd.rnd() < 0.5 then
      misn.npcAdd("competitor5", npc_desc[6], "pirate/pirate2", bar_desc[6])
   end
end

function competitor1()
   if player.misnDone("Destroy the FLF base!") == true then
      tk.msg(comptitle[6], comptext[6])
      else
      tk.msg(comptitle[1], comptext[1])
   end
end
function competitor2()
   tk.msg(comptitle[2], comptext[2])
end
function competitor3()
   tk.msg(comptitle[3], comptext[3])
end
function competitor4()
   tk.msg(comptitle[4], comptext[4])
end
function competitor5()
   tk.msg(comptitle[5], comptext[5])
end

function accept()
   
   level = 0
   reward = 50000
   
   if tk.yesno(title[1], text[1]) then

      misn.accept()

      osd_msg[3] = osd_msg[3]:format(planame)
      
      misn.setTitle(misn_title)
      misn.setReward(misn_reward:format(numstring(reward)))
      misn.setDesc(misn_desc)
      misn.osdCreate(misn_title, osd_msg)

      usedNames = {}   --In order not to have two pilots with the same name

      beginbattle()

   else
      tk.msg(refusetitle, refusetext)
      misn.finish(false)
   end
end

function beginbattle()

      stage = 0
      --Give a name to the competitor
      names = { _("The Nice Killer"),
                _("Longship Victory"),
                _("The Victim"),
                _("Old Fellow"),
                _("Angel of Grace"),
                _("Easy Killer"),
                _("The Unicorn"),
                _("The White Knight"),
                _("Spirit of St Jean"),
                _("Nec Mergitur") }
      reTry = true

      if usedNames == nil then  --This avoids bug in case usedName table is lost
         usedNames = {}
      end

      while reTry do  --Re-pick a name while it is in the usedNames list
         opponame = names[rnd.rnd(1, #names)]
         reTry = false
         for i, j in ipairs(usedNames) do
            if j == opponame then
               reTry = true
            end
         end
      end

      usedNames[#usedNames+1] = opponame
      
      tk.msg(title[2], text[2]:format(opponame,numstring(5-level)))

      enterhook = hook.enter("enter")

      if level ~= 0 then
         misn.npcRm(official)
      end
end

function enter()

   playerclass = ship.class(pilot.ship(player.pilot()))

   --Launchers are forbidden
   listofoutfits = player.pilot():outfits()
   haslauncher = false
   for i, j in ipairs(listofoutfits) do
      if j:type() == "Launcher" then
         haslauncher = true
      end
   end

   if system.cur() == missys and stage == 0 and playerclass == "Fighter" and not haslauncher then  --The player took off for the battle

      misn.osdActive(1)

      --Actually spawn the opponent
      ships = {}
      ships[1] = {"Hyena", "Shark"}
      ships[2] = {"Hyena", "Shark", "Shark", "Lancelot", "Soromid Reaver"}
      ships[3] = {"Shark", "Lancelot", "Lancelot", "Vendetta", "Soromid Reaver"}
      ships[4] = {"Lancelot", "Vendetta", "Vendetta", "Soromid Reaver", "Empire Lancelot", "Dvaered Vendetta"}
      ships[5] = {"Vendetta", "Empire Lancelot", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta"}

      shiplist = ships[level+1]
      oppotype = shiplist[ rnd.rnd(1,#shiplist) ]
      opponent = pilot.addRaw( oppotype, "baddie", mispla, "Thugs" )

      opponent:rmOutfit("all")
      opponent:rmOutfit("cores")

      oppotype = opponent:ship()

      --The core systems
      cores = {
         {"Tricon Zephyr Engine", "Milspec Orion 2301 Core System", "S&K Ultralight Combat Plating"},
         {"Tricon Zephyr II Engine", "Milspec Orion 3701 Core System", "S&K Light Combat Plating"}
      }

      equip_cores(opponent, equip_getCores(opponent, "small", cores))
      
      -- Equipment
      local nhigh, nmedium, nlow = oppotype:slots()

      opponent:addOutfit("Reactor Class I",nmedium)
      opponent:addOutfit("Battery",nlow)

      hvy = 0

      if oppotype == ship.get("Lancelot") or oppotype == ship.get("Empire Lancelot") or oppotype == ship.get("Soromid Reaver") then
         opponent:addOutfit("Heavy Ion Cannon")
         hvy = 1
      end
      
      opponent:addOutfit("Ion Cannon", nhigh-hvy)

      --Health
      opponent:setHealth(100,100)
      opponent:setEnergy(100)

      --
      opponent:rename(opponame)
      opponent:setHilight()
      opponent:setHostile()

      opponent:control()
      opponent:goto(mispla:pos() + vec2.new( 1000,  1500))

      --The TV and the security
      tv1 = pilot.addRaw( "Gawain", "civilian", mispla, "Dvaered" )
      tv2 = pilot.addRaw( "Gawain", "civilian", mispla, "Dvaered" )
      sec11 = pilot.addRaw( "Hyena", "dvaered", mispla, "Dvaered" )
      sec12 = pilot.addRaw( "Hyena", "dvaered", mispla, "Dvaered" )
      sec21 = pilot.addRaw( "Hyena", "dvaered", mispla, "Dvaered" )
      sec22 = pilot.addRaw( "Hyena", "dvaered", mispla, "Dvaered" )

      hooks = {}

      for i, k in ipairs({sec11, sec12, sec21, sec22}) do
         k:rmOutfit("all")
         k:addOutfit("Shredder", 3)
         k:addOutfit("Improved Stabilizer")
         k:rename("Security")
      end

      for i, k in ipairs({tv1,tv2}) do
         k:rmOutfit("all")
         k:addOutfit("Improved Stabilizer", 2)
         k:rename("Holovision")
      end

      for i, k in ipairs({tv1, sec11, sec12, tv2, sec21, sec22}) do
         hooks[i] = hook.pilot(k, "attacked", "escort_attacked")
         k:rmOutfit("cores")
         k:addOutfit("Tricon Zephyr Engine")
         k:addOutfit("Milspec Orion 2301 Core System")
         k:addOutfit("S&K Ultralight Combat Plating")
         k:setHealth(100,100)
         k:setEnergy(100)
         k:control()
         k:memory().radius = 300 --Set the radius for the follow function
      end

      -- Set the angle for the follow function
      tv1:memory().angle = 90
      sec11:memory().angle = 200
      sec12:memory().angle = 240
      tv2:memory().angle = 90
      sec21:memory().angle = 200
      sec22:memory().angle = 240

      --The escort follows the competitors
      tv1:follow(player.pilot(), true)
      sec11:follow(player.pilot(), true)
      sec12:follow(player.pilot(), true)
      tv2:follow(opponent, true)
      sec21:follow(opponent, true)
      sec22:follow(opponent, true)

      --Some hooks
      jumphook = hook.jumpout("jumpout")
      landhook = hook.land("land")

      opdehook = hook.pilot( opponent, "death", "oppo_dead" )
      opjuhook = hook.pilot( opponent, "jump", "oppo_jump" )
      pldihook = hook.pilot( player.pilot(), "disable", "player_disabled" )
      opdihook = hook.pilot( opponent, "disable", "oppo_disabled" )
      attackhook = hook.pilot( opponent, "attacked", "oppo_attacked" )

      --Adding the starting mark
      start_pos = mispla:pos() + vec2.new( -1000, -1500)
      mark = system.mrkAdd( mark_name, start_pos )
      prox = hook.timer(500, "proximity", {location = start_pos, radius = 300, funcname = "assault"})

   elseif haslauncher == true then
      tk.msg(dismisstitle, missiletext)
      misn.finish(false)
   elseif playerclass ~= "Fighter" then
      tk.msg(dismisstitle, fightertext)
      misn.finish(false)
   end
end

function oppo_attacked(pilot, attacker)  --The player tries to cheat by attacking before the signal
   if stage == 0 and attacker == player.pilot() then
      misn.finish(false)
      opponent:land("Dvaer Prime")
   end
end

function jumpout()   --The player is never allowed to go away
   tk.msg(dismisstitle, fleetext)
   misn.finish(false)
end

function assault()
   stage = 1
   misn.osdActive(2)
   opponent:attack(player.pilot())
   hook.rm(prox)
   hook.rm(attackhook)
   system.mrkRm(mark)
end

function land()

   --First remove some hooks...
   for i, j in ipairs({jumphook,landhook,opdehook,opjuhook,enterhook}) do
      hook.rm(j)
   end

   for i, j in ipairs(hooks) do
      hook.rm(j)
   end

   --Then manage the player's progress
   if stage == 2 and planet.cur() == mispla then  --player goes to next round
      tk.msg(title[3], text[3])

      populate_bar()
      official = misn.npcAdd("beginbattle", npc_desc[1], officialFace, bar_desc[1])

      elseif stage == 3 and planet.cur() == mispla then  --player will be payed

      if level == 5 then  --you are the champion
         tk.msg(title[4], text[4]:format(numstring(reward * 2^level)))
      elseif level == 4 then
         tk.msg(title[5], text[5]:format(numstring(reward * 2^level)))
      else
         tk.msg(title[6], text[6]:format(numstring(reward * 2^level)))
      end

      player.pay(reward * 2^level)
      misn.finish(true)

      elseif stage == 2 then
      tk.msg(dismisstitle, fleetext)
      misn.finish(false)
   end
end

function oppo_dead()  --The player killed his opponent
   player_wanted()
   misn.finish(false)
end

function oppo_jump()  --The opponent went away: player won
   won()
end

function player_disabled()  --player has lost
   misn.osdActive(3)
   stage = 3
   opponent:taskClear()
   opponent:land("Dvaer Prime")
   hook.rm(opdihook)
end

function oppo_disabled()  --Regular way to win
   won()
   opponent:setHostile(false)  --in case he recovers from disabling before the player landed
   hook.rm(pldihook)
   hook.rm(opdihook)
   boardhook = hook.pilot( opponent, "board", "oppo_boarded" )
end

function oppo_boarded()  --It is forbidden to board a competitor
   player_wanted()
   misn.finish(false)
end

function won()
   level = level+1
   stage = 2

   misn.osdActive(3)

   if level == 5 then  --the player is the new champion
      stage = 3
   end
end

function escort_attacked(pilot,attacker) --someone attacked the escort

   for i, k in ipairs({sec11, sec12, sec21, sec22}) do
      k:control()
      k:attack(attacker)
   end

   for i, k in ipairs({tv1,tv2}) do --The tv tries to land
      k:control()
      k:land(mispla)
   end

   if attacker == player.pilot() then
      misn.finish(false)
   end

end

function player_wanted()  --For some reason, the security wants to take the player down
   for i, k in ipairs({sec11, sec12, sec21, sec22}) do
      k:control()
      k:setHostile()
      k:attack(player.pilot())
   end

   for i, k in ipairs({tv1,tv2}) do --The tv tries to land
      k:control()
      k:setHostile()
      k:land(mispla)
   end
end

