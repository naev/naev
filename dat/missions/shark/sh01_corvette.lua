--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Sharkman Is Back">
 <unique />
 <priority>3</priority>
 <done>A Shark Bites</done>
 <chance>10</chance>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <faction>Empire</faction>
 <faction>Frontier</faction>
 <faction>Goddard</faction>
 <faction>Independent</faction>
 <faction>Sirius</faction>
 <faction>Soromid</faction>
 <faction>Traders Society</faction>
 <faction>Za'lek</faction>
 <notes>
  <campaign>Nexus show their teeth</campaign>
 </notes>
</mission>
--]]
--[[
   This is the second mission of the Shark's teeth campaign. The player has to take part to a fake battle.

   Stages :
   0) Way to Toaxis
   1) Battle
   2) Going to Darkshed
--]]
local pir = require "common.pirate"
local fmt = require "format"
local lmisn = require "lmisn"
local shark = require "common.shark"
local vn = require "vn"

local sharkboy -- Non-persistent state

--Change here to change the planet and the system
local battlesys = system.get("Toaxis")
local paypla, paysys = spob.getS("Darkshed")
--System neighbouring Toaxis with zero pirate presence due to a "Virtual Pirate Unpresence" asset
local escapesys = system.get("Ingot")

function create ()
   if not misn.claim(battlesys) then
      misn.finish(false)
   end

   misn.setNPC( shark.arnold.name, shark.arnold.portrait, shark.arnold.description )
end

function accept()
   mem.stage = 0
   local accepted = false

   vn.clear()
   vn.scene()
   local arnold = vn.newCharacter( shark.vn_arnold() )
   vn.transition( shark.arnold.transition )

   arnold(_([["I have another job for you. The Baron was unfortunately not as impressed as we hoped. So we need a better demonstration, and we think we know what to do: we're going to demonstrate that the Lancelot, our higher-end fighter design, is more than capable of defeating Destroyer-class ships.]]))
   arnold(_([["Now, one small problem we face is that pirates almost never use Destroyer-class ships; they tend to stick to fighters, corvettes, and cruisers. More importantly, actually sending a fighter after a Destroyer is exceedingly dangerous, even if we could find a pirate piloting one. So we have another plan: we want someone to pilot a Destroyer-class ship and just let another pilot disable them with ion cannons.]]))
   arnold(_([["What do you say? Are you interested?"]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   arnold(_([["OK, that's alright."]]))
   vn.done( shark.arnold.transition )

   vn.label("accept")
   vn.func( function () accepted = true end )
   arnold(fmt.f(_([["Great! Go and meet our pilot in {battlesys}. After the job is done, meet me on {pnt} in the {sys} system."]]),
      {battlesys=battlesys, pnt=paypla, sys=paysys}))

   vn.done( shark.arnold.transition )
   vn.run()

   if not accepted then return end

   misn.accept()

   misn.setTitle(_("Sharkman is back"))
   misn.setReward(fmt.credits(shark.rewards.sh01/2))
   misn.setDesc(_("Nexus Shipyards wants you to fake a loss against a Lancelot while piloting a Destroyer-class ship."))
   misn.osdCreate(_("Sharkman Is Back"), {
      fmt.f(_("Jump in {sys} with a Destroyer-class ship and let the Lancelot disable you"), {sys=battlesys}),
      fmt.f(_("Go to {pnt} in {sys} to collect your pay"), {pnt=paypla, sys=paysys}),
   })
   misn.osdActive(1)

   mem.marker = misn.markerAdd(battlesys, "low")

   hook.jumpout("jumpout")
   hook.land("land")
   hook.enter("enter")
end

function jumpout()
   if mem.stage == 1 then --player trying to escape
      lmisn.fail( _("You ran away.") )
   end
end

function land()
   if mem.stage == 1 then --player trying to escape
      lmisn.fail( _("You ran away.") )
   end
   if mem.stage == 2 and spob.cur() == paypla then
      vn.clear()
      vn.scene()
      local arnold = vn.newCharacter( shark.vn_arnold() )
      vn.transition( shark.arnold.transition )
      arnold(_([[As you land, you see Arnold Smith waiting for you. He explains that the Baron was so impressed by the battle that he signed an updated contract with Nexus Shipyards, solidifying Nexus as the primary supplier of ships for his fleet. As a reward, they give you twice the sum of credits they promised to you.]]))
      vn.func( function ()
         pir.reputationNormalMission(rnd.rnd(2,3))
         player.pay(shark.rewards.sh01)
      end )
      vn.sfxVictory()
      vn.na(fmt.reward( shark.rewards.sh01 ))
      vn.done( shark.arnold.transition )
      vn.run()

      shark.addLog( _([[You helped Nexus Shipyards fake a demonstration by allowing a Lancelot to disable your Destroyer class ship.]]) )
      misn.finish(true)
   end
end

function enter()
   local playersize = player.pilot():ship():size()
   --Jumping in Toaxis for the battle with a Destroyer class ship
   if system.cur() == battlesys and mem.stage == 0 and playersize == 4 then
      pilot.clear()
      pilot.toggleSpawn( false )

      hook.timer(2.0,"lets_go")
   end
end

function lets_go()
   -- spawns the Shark
   sharkboy = pilot.add( "Lancelot", "Mercenary", system.get("Zacron"), nil, {ai="baddie_norun"} )
   sharkboy:setHostile(true)
   sharkboy:setHilight(true)

   mem.stage = 1

   mem.shark_dead_hook = hook.pilot( sharkboy, "death", "shark_dead" )
   mem.disabled_hook = hook.pilot( player.pilot(), "disable", "disabled" )
end

function shark_dead()  --you killed the shark
   lmisn.fail( _("You destroyed the Lancelot.") )
end

function disabled(pilot, attacker)
   if attacker == sharkboy then
      mem.stage = 2
      misn.osdActive(2)
      misn.markerRm(mem.marker)
      mem.marker2 = misn.markerAdd(paypla, "low")
      pilot.toggleSpawn( true )
   end
   sharkboy:control(true)
   -- making sure the shark doesn't continue attacking the player
   sharkboy:hyperspace(escapesys, true)
   sharkboy:setNoDeath(true)
end
