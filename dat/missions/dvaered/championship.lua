--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Dvaered Championship">
 <unique />
 <priority>4</priority>
 <chance>50</chance>
 <location>Bar</location>
 <spob>Dvaer Prime</spob>
 <notes>
   <tier>3</tier>
 </notes>
</mission>
 --]]
--[[

   The player takes part into a challenge between fighter pilots in Dvaered space, he has to defeat 5 opponents to win.

   Stages :
   0) Way to positions
   1) Begin of the fight
   2) Player won
   3) End of competition

--]]

--Needed scripts
local fmt = require "format"
require "proximity"
local portrait = require "portrait"
local bioship = require "bioship"
local ai_setup = require "ai.core.setup"

-- NPC
mem.npc_portrait = {}
mem.npc_portrait[2] = portrait.get()
mem.npc_portrait[3] = portrait.get()
mem.npc_portrait[4] = portrait.get("Empire")
mem.npc_portrait[5] = portrait.get("Dvaered")
mem.npc_portrait[6] = portrait.get("Pirate")

-- Non-persistent state
local hooks = {}
local usedNames = {}   --In order not to have two pilots with the same name
local opponent, sec11, sec12, sec21, sec22, tv1, tv2

--Change here to change the planet and the system
local mispla, missys = spob.getS("Dvaer Prime")

local beginbattle, land_everyone, player_wanted, won -- Forward-declared functions

function create ()
   if not misn.claim ( missys ) then
      misn.finish(false)
   end

   mem.officialFace = portrait.getMil( "Dvaered" )
   mem.official = misn.setNPC(_("An official"), mem.officialFace, _("This person seems to be looking for suitable combat pilots."))
end

local function populate_bar() --add some random npcs
   if rnd.rnd() < 0.5 then
      misn.npcAdd("competitor1", _("Pilot"), mem.npc_portrait[2], _("This pilot looks very self-confident"))
   end
   if rnd.rnd() < 0.5 then
      misn.npcAdd("competitor2", _("Pilot"), mem.npc_portrait[3], _("This pilot seems to work as a private combat pilot"))
   end
   if rnd.rnd() < 0.5 then
      misn.npcAdd("competitor3", _("Imperial pilot"), mem.npc_portrait[4], _([[This pilot is is clearly from the Empire.]]))
   end
   if rnd.rnd() < 0.5 then
      misn.npcAdd("competitor4", _("Dvaered pilot"), mem.npc_portrait[5], _([[This pilot surely works as a Vendetta pilot.]]))
   end
   if rnd.rnd() < 0.5 then
      misn.npcAdd("competitor5", _("Strange pilot"), mem.npc_portrait[6], _([[This pilot looks like a pirate, but strangely enough, the authorities don't seem worried.]]))
   end
end

function competitor1()
   if player.misnDone("Destroy the FLF base!") == true then
      tk.msg(_("I am here to win the championship"), _([["I am here to claim my place as this cycle's champion! I've prepared myself since the first day I piloted a ship. Trust me, I'm nearly invincible and my Vendetta is indestructible.
    "Do you know who I am? I am the famous independent pilot who helped Dvaered High Command destroy the FLF base in the nebula! I managed to defeat lots of FLF fighters with my ship! I will tell you my adventures some other time, but for now, I need to concentrate."]]))
   else
      tk.msg(_("I am here to win the championship"), _([["Hello! I am here to claim my place as this cycle's champion! I've prepared myself since the first day I piloted a ship. Trust me, I'm nearly invincible and my Vendetta is indestructible.
    "Did you know that the Vendetta is the best fighter in this part of the galaxy? It's the reason why every pilot who's won this championship had one."]]))
   end
end
function competitor2()
   tk.msg(_("Hello"), _([["Are you here for the Dvaered dogfight championship? I am a competitor. I fly a Shark, so I don't hope to win lots of rounds... But I still enjoy the battle. Every cycle, a Dvaered pilot wins. Do you know why? It's because the rules of the championship advantage heavy armoured, well armed fighters, like the Vendetta. Imperial pilots are used to electronic warfare with guided missiles and stealth ships. Dvaered pilots, on the other hand, only understand basic dogfighting."]]))
end
function competitor3()
   tk.msg(_("Imperial Pilot"), _([["What a pity. I am the best in my squad. I trained cycles to be able to take down these pitiful Vendettas with my missiles before they even see my Lancelot on their radar. But in this championship, only armour and firepower are useful."]]))
end
function competitor4()
   tk.msg(_("Dvaered Pilot"), _([["Nice to see you. I am a Vendetta pilot. I hope I win this time! For us, being the champion here means that you become member of the senior staff, which makes you closer to Dvaered High Command! Who knows? Maybe one day I will become a Warlord."]]))
end
function competitor5()
   tk.msg(_("Obvious Pirate"), _([["Hi, I'm... err... I'm an independent pilot. I'm here to take part in the challenge and see the best Dvaered Vendetta pilots in action. It helps to know how they fly in my job."]]))
end

function accept()
   mem.level = 0
   mem.reward = 50e3

   if tk.yesno(_("Do you want to take part to a challenge?"), _([["Hello, I'm a member of the staff of the Dvaered dogfight challenge. Here are the rules: you need to take off with a fighter-class ship and to join your starting mark. After that, you will try to disable your opponent. Don't kill them; the security staff won't forgive that. It is forbidden to use missiles, so you won't be allowed to have those equipped while taking off. It's also forbidden to board the opponent's ship and to attack him before the signal is given. You are not allowed to land on any planet or jump away during the championship.
    We are looking for pilots. Are you in?"]])) then

      misn.accept()

      misn.setTitle(_("The Dvaered Championship"))
      misn.setReward(_("From 50k to 1.6m credits, depending on your rank"))
      misn.setDesc(_("You are taking part in a fight contest. Try to do your best!"))
      misn.osdCreate(_("The Dvaered Championship"), {
         _("Go to the starting point"),
         _("Disable your opponent; DO NOT KILL"),
         fmt.f(_("Land on {pnt}"), {pnt=mispla}),
      })

      beginbattle()

   else
      tk.msg(_("Sorry, not interested"), _([["That's your choice. Goodbye, then."]]))
      return
   end
end

function abort () -- Everyone lands in case the player aborts
   land_everyone()
end

function beginbattle()
      mem.stage = 0
      --Give a name to the competitor
      local names = {
	        _("The Nice Killer"),
                _("Longship Victory"),
                _("The Victim"),
                _("Old Fellow"),
                _("Angel of Grace"),
                _("Easy Killer"),
                _("The Unicorn"),
                _("The White Knight"),
                _("Spirit of St Jean"),
                _("Nec Mergitur") }
      local reTry = true

      while reTry do  --Re-pick a name while it is in the usedNames list
         mem.opponame = names[rnd.rnd(1, #names)]
         reTry = false
         for i, j in ipairs(usedNames) do
            if j == mem.opponame then
               reTry = true
            end
         end
      end

      usedNames[#usedNames+1] = mem.opponame

      tk.msg(_("Let's go"), fmt.f(_([[For this round, your opponent is {plt}. Remember: use a fighter with no launchers. You still have to defeat {n} opponents to win.]]), {plt=mem.opponame, n=fmt.number(5-mem.level)}))

      mem.enterhook = hook.enter("enter")

      if mem.level ~= 0 then
         misn.npcRm(mem.official)
      end
end

function enter()
   mem.playerclass = ship.class(pilot.ship(player.pilot()))

   --Launchers are forbidden
   local listofoutfits = player.pilot():outfitsList()
   local haslauncher = false
   for i, j in ipairs(listofoutfits) do
      if j:type() == "Launcher" then
         haslauncher = true
      end
   end

   if system.cur() == missys and mem.stage == 0 and mem.playerclass == "Fighter" and not haslauncher then  --The player took off for the battle

      misn.osdActive(1)

      --Actually spawn the opponent
      local ships = {}
      ships[1] = {"Hyena", "Shark"}
      ships[2] = {"Hyena", "Shark", "Shark", "Lancelot", "Soromid Reaver"}
      ships[3] = {"Shark", "Lancelot", "Lancelot", "Vendetta", "Soromid Reaver"}
      ships[4] = {"Lancelot", "Vendetta", "Vendetta", "Soromid Reaver", "Empire Lancelot", "Dvaered Vendetta"}
      ships[5] = {"Vendetta", "Empire Lancelot", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta"}

      local shiplist = ships[mem.level+1]
      local oppotype = shiplist[ rnd.rnd(1,#shiplist) ]
      opponent = pilot.add( oppotype, "Mercenary", mispla, mem.opponame, {ai="baddie", naked=true} )

      oppotype = opponent:ship()

      --The core systems
      if oppotype == ship.get("Hyena") or oppotype == ship.get("Shark") then
         opponent:outfitAdd("Tricon Zephyr Engine")
         opponent:outfitAdd("Milspec Orion 2301 Core System")
         opponent:outfitAdd("S&K Ultralight Combat Plating")
      elseif oppotype == ship.get("Soromid Reaver") then
         bioship.simulate( opponent, bioship.maxstage( opponent ) )
      else
         opponent:outfitAdd("Tricon Zephyr II Engine")
         opponent:outfitAdd("Milspec Orion 3701 Core System")
         opponent:outfitAdd("S&K Light Combat Plating")
      end

      -- Equipment
      local nhigh, _nmedium, nlow = oppotype:slots()

      -- TODO: decide if the "Faraday Tempest Coating" is a good idea
      opponent:outfitAdd("Battery I",nlow)

      local hvy = 0
      if oppotype == ship.get("Lancelot") or oppotype == ship.get("Empire Lancelot") then
         opponent:outfitAdd("Heavy Ion Cannon")
         hvy = 1
      end

      opponent:outfitAdd("Ion Cannon", nhigh-hvy)

      --Health
      opponent:setHealth(100,100)
      opponent:setEnergy(100)

      --
      opponent:setHilight()
      opponent:setHostile()

      opponent:control()
      opponent:moveto(mispla:pos() + vec2.new( 1000,  1500))
      ai_setup.setup(opponent)

      --The TV and the security
      tv1 = pilot.add( "Gawain", "Dvaered", mispla, _("Holovision"), {ai="civilian"} )
      tv2 = pilot.add( "Gawain", "Dvaered", mispla, _("Holovision"), {ai="civilian"} )
      sec11 = pilot.add( "Hyena", "Dvaered", mispla, _("Security") )
      sec12 = pilot.add( "Hyena", "Dvaered", mispla, _("Security") )
      sec21 = pilot.add( "Hyena", "Dvaered", mispla, _("Security") )
      sec22 = pilot.add( "Hyena", "Dvaered", mispla, _("Security") )

      hooks = {}

      for i, k in ipairs({sec11, sec12, sec21, sec22}) do
         k:outfitRm("all")
         k:outfitAdd("Gauss Gun", 3)
         k:outfitAdd("Improved Stabilizer")
      end

      for i, k in ipairs({tv1,tv2}) do
         k:outfitRm("all")
         k:outfitAdd("Improved Stabilizer", 2)
      end

      for i, k in ipairs({tv1, sec11, sec12, tv2, sec21, sec22}) do
         hooks[i] = hook.pilot(k, "attacked", "escort_attacked")
         k:outfitRm("cores")
         k:outfitAdd("Tricon Zephyr Engine")
         k:outfitAdd("Milspec Orion 2301 Core System")
         k:outfitAdd("S&K Ultralight Combat Plating")
         k:setHealth(100,100)
         k:setEnergy(100)
         k:control()
         k:memory().radius = 300 --Set the radius for the follow function
      end

      -- Set the angle for the follow function
      tv1:memory().angle = math.rad(90)
      sec11:memory().angle = math.rad(200)
      sec12:memory().angle = math.rad(240)
      tv2:memory().angle = math.rad(90)
      sec21:memory().angle = math.rad(200)
      sec22:memory().angle = math.rad(240)

      --The escort follows the competitors
      tv1:follow(player.pilot(), true)
      sec11:follow(player.pilot(), true)
      sec12:follow(player.pilot(), true)
      tv2:follow(opponent, true)
      sec21:follow(opponent, true)
      sec22:follow(opponent, true)

      --Some hooks
      mem.jumphook = hook.jumpout("jumpout")
      mem.landhook = hook.land("land")

      mem.opdehook = hook.pilot( opponent, "death", "oppo_dead" )
      mem.opjuhook = hook.pilot( opponent, "jump", "oppo_jump" )
      mem.pldihook = hook.pilot( player.pilot(), "disable", "player_disabled" )
      mem.opdihook = hook.pilot( opponent, "disable", "oppo_disabled" )
      mem.attackhook = hook.pilot( opponent, "attacked", "oppo_attacked" )

      --Adding the starting mark
      local start_pos = mispla:pos() + vec2.new( -1000, -1500)
      mem.mark = system.markerAdd( start_pos, _("START") )
      mem.prox = hook.timer(0.5, "proximity", {location = start_pos, radius = 300, funcname = "assault"})

   elseif haslauncher == true then
      tk.msg(_("You are dismissed"), _("You weren't allowed to use missiles!"))
      misn.finish(false)
   elseif mem.playerclass ~= "Fighter" then
      tk.msg(_("You are dismissed"), _("You had to use a fighter!"))
      misn.finish(false)
   end
end

function land_everyone()
   for i, k in ipairs({tv1, sec11, sec12, tv2, sec21, sec22, opponent}) do
      k:control()
      k:land("Dvaer Prime", true)
   end
end

function oppo_attacked(_pilot, attacker)  --The player tries to cheat by attacking before the signal
   if mem.stage == 0 and (attacker and attacker:withPlayer()) then
      land_everyone()
      tk.msg(_("You are dismissed"), _("You weren't supposed to attack before the signal."))
      system.markerRm(mem.mark)
      misn.finish(false)
   end
end

function jumpout()   --The player is never allowed to go away
   tk.msg(_("You are dismissed"), _("You weren't supposed to go away."))
   misn.finish(false)
end

function assault()
   mem.stage = 1
   misn.osdActive(2)
   opponent:attack(player.pilot()) -- Probably fine to just attack player
   hook.rm(mem.prox)
   hook.rm(mem.attackhook)
   system.markerRm(mem.mark)
end

function land()

   if mem.stage == 2 and spob.cur() == mispla then  --player goes to next round
      --Manage the player's progress
      tk.msg(_("You won this round"), _([["Congratulations," the staff says to you. "Come back when you are ready for the next round!"]]))

      populate_bar()
      mem.official = misn.npcAdd("cleanNbegin", _("An official"), mem.officialFace, _("This person seems to be looking for suitable combat pilots."))

      elseif mem.stage == 3 and spob.cur() == mispla then  --player will be payed

      if mem.level == 5 then  --you are the champion
         tk.msg(_("You are the new champion"), fmt.f(_([[Congratulations! The staff pays you {credits}.]]), {credits=fmt.credits(mem.reward * 2^mem.level)}))
      elseif mem.level == 4 then
         tk.msg(_("You are the vice-champion"), fmt.f(_([[Congratulations! The staff pays you {credits}.]]), {credits=fmt.credits(mem.reward * 2^mem.level)}))
      else
         tk.msg(_("Thanks for playing"), fmt.f(_([[The staff pays you {credits}.]]), {credits=fmt.credits(mem.reward * 2^mem.level)}))
      end

      player.pay(mem.reward * 2^mem.level)
      misn.finish(true)

      elseif mem.stage == 2 then
      tk.msg(_("You are dismissed"), _("You weren't supposed to go away."))
      misn.finish(false)
   end
end

function cleanNbegin()
   -- Remove some hooks and begin a new battle
   for i, j in ipairs({mem.jumphook,mem.landhook,mem.opdehook,mem.opjuhook,mem.enterhook}) do
      hook.rm(j)
   end

   for i, j in ipairs(hooks) do
      hook.rm(j)
   end
   beginbattle()
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
   mem.stage = 3
   opponent:taskClear()
   opponent:land("Dvaer Prime",true)
   hook.rm(mem.opdihook)
end

function oppo_disabled()  --Regular way to win
   won()
   opponent:setHostile(false)  --in case he recovers from disabling before the player landed
   hook.rm(mem.pldihook)
   hook.rm(mem.opdihook)
   mem.boardhook = hook.pilot( opponent, "board", "oppo_boarded" )
end

function oppo_boarded()  --It is forbidden to board a competitor
   player_wanted()
   misn.finish(false)
end

function won()
   mem.level = mem.level+1
   mem.stage = 2

   misn.osdActive(3)

   if mem.level == 5 then  --the player is the new champion
      mem.stage = 3
   end
end

function escort_attacked(_plt, attacker) --someone attacked the escort
   for i, k in ipairs({sec11, sec12, sec21, sec22}) do
      k:control()
      k:attack(attacker)
   end

   for i, k in ipairs({tv1,tv2}) do --The tv tries to land
      k:control()
      k:land(mispla)
   end

   if attacker == player.pilot() or attacker:leader() == player.pilot() then
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
