--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The FLF Contact">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>3</priority>
  <done>The Meeting</done>
  <chance>5</chance>
  <location>Bar</location>
  <faction>Dvaered</faction>
  <faction>Empire</faction>
  <faction>Frontier</faction>
  <faction>Goddard</faction>
  <faction>Independent</faction>
  <faction>Sirius</faction>
  <faction>Soromid</faction>
  <faction>Traders Guild</faction>
  <faction>Za'lek</faction>
  <cond>not diff.isApplied( "flf_dead" )</cond>
 </avail>
 <notes>
  <campaign>Nexus show their teeth</campaign>
 </notes>
</mission>
--]]
--[[
   This is the sixth mission of the Shark's teeth campaign. The player has to take contact with the FLF.

   Stages :
   0) Way to Eiger/Surano
   1) Way back to Darkshed
--]]
local pir = require "common.pirate"
local fmt = require "format"
local shark = require "common.shark"

-- luacheck: globals board hail land (Hook functions passed by name)

-- Mission constants
local paypla, paysys = spob.getS("Darkshed")

function create ()
   --Change here to change the planets and the systems
   mem.nextsys = system.get("Arandon") -- This should be the same as the system used in sh06!
   misn.setNPC(_("Arnold Smith"), "neutral/unique/arnoldsmith.webp", _([[It looks like he has yet another job for you.]]))
end

function accept()
   mem.stage = 0

   if tk.yesno(_("Nice to see you again!"), fmt.f(_([["Hello, {player}! Are you ready to take part in another sales mission?
    "As you know, the FLF is a heavy user of our ships, but they're also heavy users of Dvaered ships, chiefly the Vendetta design. Since House Dvaered is an enemy of the FLF, we see this as an opportunity to expand our sales: we want to convince the FLF leaders to buy more Nexus ships and fewer Dvaered ships. This will be through a false contraband company so that word doesn't get out that we're supporting terrorists by selling them ships. What do you say? Can you help us once again?"]]), {player=player.name()})) then
      misn.accept()
      tk.msg(_("Good luck"), fmt.f(_([["Perfect! So, this mission is pretty simple: I want you to pass on this proposal to them." He hands you a data chip. "It's a request to meet with the FLF leaders on {sys}. If all goes well, I'll be asking you to take me there next.
    "Any FLF ship should do the job. Try hailing them and see if you get a response. If they won't talk, disable and board them so you can force them to listen. Pretty simple, really. Good luck!"]]), {sys=mem.nextsys}))

      misn.setTitle(_("The FLF Contact"))
      misn.setReward(fmt.credits(shark.rewards.sh05))
      misn.setDesc(_("Nexus Shipyards is looking to strike a better deal with the FLF."))
      misn.osdCreate(_("The FLF Contact"), {
         _("Hail any FLF ship, or disable and board one if necessary"),
         fmt.f(_("Go back to {pnt} in {sys}"), {pnt=paypla, sys=paysys}),
      })
      misn.osdActive(1)

      hook.land( "land" )
      hook.hail( "hail" )
      hook.board( "board" )
   else
      tk.msg(_("Sorry, not interested"), _([["Alright, then. I'll see if anyone else is interested."]]))
      misn.finish(false)
   end
end

function land()
   --Job is done
   if mem.stage == 1 and spob.cur() == paypla then
      tk.msg(_("Good news"), fmt.f(_([[Smith is clearly pleased with the results. "I have received word that the FLF leaders are indeed interested. Meet me at the bar whenever you're ready to take me to {sys}. And here's your payment."]]), {sys=mem.nextsys}))
      pir.reputationNormalMission(rnd.rnd(2,3))
      player.pay(shark.rewards.sh05)
      shark.addLog( _([[You helped Arnold Smith establish a contact with the FLF. He said to meet you at the bar on Alteris when you're ready to take him to Arandon.]]) )
      misn.finish(true)
   end
end

function hail( p )
   if mem.stage == 0 and p:faction() == faction.get("FLF") and not p:hostile() then
      player.commClose()
      tk.msg(_("Peaceful Resolution"), _([[The FLF ship peacefully responds to you. You explain the details of what is going on and transmit the proposal, after which you both go your separate ways.]]))
      mem.stage = 1
      misn.osdActive(2)
      mem.marker2 = misn.markerAdd(paypla, "low")
   end
end

function board( p )
   if mem.stage == 0 and p:faction() == faction.get("FLF") then
      player.unboard()
      tk.msg(_("Some Resistance Encountered"), _([[The FLF officers are clearly ready for battle, but after subduing them, you assure them that you're just here to talk. Eventually, you are able to give them a copy of the proposal and leave peacefully, for lack of a better word.]]))
      mem.stage = 1
      misn.osdActive(2)
      mem.marker2 = misn.markerAdd(paypla, "low")
   end
end
