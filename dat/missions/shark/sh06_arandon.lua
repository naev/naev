--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="A Journey To Arandon">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>30</priority>
  <done>The FLF Contact</done>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Darkshed</planet>
  <cond>not diff.isApplied( "flf_dead" )</cond>
 </avail>
 <notes>
  <campaign>Nexus show their teeth</campaign>
 </notes>
</mission>
--]]
--[[
   This is the seventh mission of the Shark's teeth campaign. The player has to meet the FLF in Arandon.

   Stages :
   0) Way to Arandon
   1) Way back to Darkshed
--]]
local pir = require "common.pirate"
local fmt = require "format"
local shark = require "common.shark"

--Change here to change the planets and the systems
local missys = system.get("Arandon")
local paypla, paysys = planet.getS("Darkshed")

function create ()
   if not misn.claim(missys) then
      misn.finish(false)
   end

   misn.setNPC(_("Arnold Smith"), "neutral/unique/arnoldsmith.webp", _([[He's waiting for you.]]))
end

function accept()

   stage = 0
   reward = 750e3

   if tk.yesno(_("Let's go"), _([["Is your ship ready for the dangers of the Nebula?"]])) then
      misn.accept()
      tk.msg(_("Go"), _([[Smith once again steps in your ship in order to go to a meeting.]]))

      misn.setTitle(fmt.f(_("A Journey To {sys}"), {sys=missys}))
      misn.setReward(fmt.credits(reward))
      misn.setDesc(fmt.f(_("You are to transport Arnold Smith to {sys} so that he can talk about a deal."), {sys=missys}))
      misn.osdCreate(fmt.f(_("A Journey To {sys}"), {sys=missys}), {
         fmt.f(_("Go to {sys} and wait for the FLF ship, then hail and board it."), {sys=missys}),
         fmt.f(_("Go back to {pnt} in {sys}"), {pnt=paypla, sys=paysys}),
      })
      misn.osdActive(1)

      marker = misn.markerAdd(missys, "low")

      local c = misn.cargoNew( N_("Smith"), N_("Arnold Smith of Nexus Shipyards.") )
      smith = misn.cargoAdd( c, 0 )

      landhook = hook.land("land")
      enterhook = hook.enter("enter")
      else
      tk.msg(_("...Or not"), _([["Come back when you are ready."]]))
      misn.finish(false)
   end
end

function land()
   --Job is done
   if stage == 1 and planet.cur() == paypla then
      if misn.cargoRm(smith) then
         tk.msg(_("Well done!"), _([[Smith thanks you for the job well done. "Here is your pay," he says. "I will be in the bar if I have another task for you."]]))
         pir.reputationNormalMission(rnd.rnd(2,3))
         player.pay(reward)
         misn.osdDestroy()
         hook.rm(enterhook)
         hook.rm(landhook)
         shark.addLog( _([[You transported Arnold Smith to a meeting with someone from the FLF. He said that he had good results.]]) )
         misn.finish(true)
      end
   end
end

function enter()
   --Entering in Arandon in order to find the FLF Pacifier
   if system.cur() == missys then
      --Lets unspawn everybody (if any)
      pilot.clear()
      pilot.toggleSpawn(false)

      --Waiting to spawn the FLF in order to let the player's shield decrease
      hook.timer(2.0,"wait_msg")
      hook.timer(10.0,"flf_people")

   end
end

function wait_msg ()
   -- Prevents the player from being impatient
   tk.msg( _("Let's wait"), _([["Mm. It looks like the others have not arrived yet." Smith says. "Just wait close to the jump point, they should arrive soon."]]) )
end

function flf_people ()
   pacifier = pilot.add( "Pacifier", "FLF", system.get("Doeston") )
   pacifier:memory().aggressive = false
   pacifier:setFriendly( true )
   pacifier:setInvincible( true )
   hook.pilot( pacifier, "hail", "hail_pacifier" )
   hook.pilot( pacifier, "death", "dead" )
   hook.pilot( pacifier, "jump", "failed" )
end

function hail_pacifier()
   --hailing the pacifier
   tk.msg(_("Hail"), _([[The Pacifier commander answers you and stops his ship, waiting to be boarded.]]))
   pacifier:control()
   pacifier:brake()
   pacifier:setActiveBoard(true)
   pacifier:hookClear()
   hook.pilot( pacifier, "death", "dead" )
   hook.pilot(pacifier, "board", "board")
   player.commClose()
end

function board()
   --boarding the pacifier
   tk.msg(_("The Meeting"), fmt.f(_([[As you board, Arnold Smith insists on entering the FLF's ship alone. A few periods later, he comes back looking satisfied. It seems this time luck is on his side. He mentions that he had good results with a smile on his face before directing you to take him back to {sys}.]]), {sys=paysys}))
   player.unboard()
   pacifier:control(false)
   pacifier:setActiveBoard(false)
   stage = 1
   misn.osdActive(2)
   misn.markerRm(marker)
   marker2 = misn.markerAdd(paysys, "low")
end

function dead()  --Actually, I don't know how it could happened...
   misn.finish(false)
end

function failed()
   misn.finish(false)
end

function abort()
   misn.cargoRm(smith)
   misn.finish(false)
end
