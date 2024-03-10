--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="A Journey To Arandon"><!-- Actually Behar though... -->
 <unique />
 <priority>3</priority>
 <done>The FLF Contact</done>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Darkshed</spob>
 <cond>not diff.isApplied( "flf_dead" )</cond>
 <notes>
  <campaign>Nexus show their teeth</campaign>
 </notes>
</mission>
--]]
--[[
   This is the seventh mission of the Shark's teeth campaign. The player has to meet the FLF in Behar.

   Stages :
   0) Way to Behar
   1) Way back to Darkshed
--]]
local pir = require "common.pirate"
local fmt = require "format"
local shark = require "common.shark"
local vn = require "vn"
local vntk = require "vntk"
local ccomm = require "common.comm"

local pacifier -- Non-persistent state

--Change here to change the planets and the systems
local missys = system.get("Behar")
local paypla, paysys = spob.getS("Darkshed")

function create ()
   if not misn.claim(missys) then
      misn.finish(false)
   end

   misn.setNPC(shark.arnold.name, shark.arnold.portrait, _([[He's waiting for you.]]))
end

function accept()
   mem.stage = 0
   local accepted = false

   vn.clear()
   vn.scene()
   local arnold = vn.newCharacter( shark.vn_arnold() )
   vn.transition( shark.arnold.transition )

   arnold(_([["Is your ship ready for the dangers of the Nebula?"]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   arnold(_([["Come back when you are ready."]]))
   vn.done( shark.arnold.transition )

   vn.label("accept")
   vn.func( function () accepted = true end )
   arnold(_([[Smith once again steps in your ship in order to go to a meeting.]]))

   vn.done( shark.arnold.transition )
   vn.run()

   if not accepted then return end

   misn.accept()

   misn.setTitle(fmt.f(_("A Journey To {sys}"), {sys=missys}))
   misn.setReward(shark.rewards.sh06)
   misn.setDesc(fmt.f(_("You are to transport Arnold Smith to {sys} so that he can talk about a deal."), {sys=missys}))
   misn.osdCreate(fmt.f(_("A Journey To {sys}"), {sys=missys}), {
      fmt.f(_("Go to {sys} and wait for the FLF ship, then hail and board it."), {sys=missys}),
      fmt.f(_("Go back to {pnt} in {sys}"), {pnt=paypla, sys=paysys}),
   })
   misn.osdActive(1)

   mem.marker = misn.markerAdd(missys, "low")

   local c = commodity.new( N_("Smith"), N_("Arnold Smith of Nexus Shipyards.") )
   mem.smith = misn.cargoAdd( c, 0 )

   hook.land("land")
   hook.enter("enter")
end

function land()
   --Job is done
   if mem.stage == 1 and spob.cur() == paypla then
      vn.clear()
      vn.scene()
      local arnold = vn.newCharacter( shark.vn_arnold() )
      vn.transition( shark.arnold.transition )
      arnold(_([[Smith thanks you for the job well done. "Here is your pay," he says. "I will be in the bar if I have another task for you."]]))
      vn.func( function ()
         pir.reputationNormalMission(rnd.rnd(2,3))
         player.pay(shark.rewards.sh06)
      end )
      vn.sfxVictory()
      vn.na(fmt.reward(shark.rewards.sh06))
      vn.run()
      shark.addLog( _([[You transported Arnold Smith to a meeting with someone from the FLF. He said that he had good results.]]) )
      misn.finish(true)
   end
end

function enter()
   -- Entering in Behar in order to find the FLF Pacifier
   if system.cur() ~= missys then
      return
   end

   --Lets unspawn everybody (if any)
   pilot.clear()
   pilot.toggleSpawn(false)

   --Waiting to spawn the FLF in order to let the player's shield decrease
   hook.timer(2.0,"wait_msg")
   hook.timer(10.0,"flf_people")
end

function wait_msg ()
   -- Prevents the player from being impatient
   vntk.msg( _("Let's wait"), _([["Mm. It looks like the others have not arrived yet." Smith says. "Just wait close to the jump point, they should arrive soon."]]) )
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
   vn.clear()
   vn.scene()
   local p = ccomm.newCharacter( vn, pacifier )
   vn.transition()
   p(_([[The Pacifier commander answers you and stops his ship, waiting to be boarded.]]))
   vn.run()

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
   vntk.msg(_("The Meeting"), fmt.f(_([[As you board, Arnold Smith insists on entering the FLF's ship alone. A few periods later, he comes back looking satisfied. It seems this time luck is on his side. He mentions that he had good results with a smile on his face before directing you to take him back to {sys}.]]), {sys=paysys}))
   player.unboard()
   pacifier:control(false)
   pacifier:setActiveBoard(false)
   mem.stage = 1
   misn.osdActive(2)
   misn.markerRm(mem.marker)
   mem.marker2 = misn.markerAdd(paypla, "low")
end

function dead()  --Actually, I don't know how it could happened...
   misn.finish(false)
end

function failed()
   misn.finish(false)
end
