--[[

   This is the sixth mission of the Shark's teeth campaign. The player has to take contact with the FLF.

   Stages :
   0) Way to Eiger/Surano
   1) Way back to Darkshed

--]]

include "numstring.lua"

title = {}
text = {}
osd_msg = {}
npc_desc = {}
bar_desc = {}

title[1] = _("Nice to see you again!")
text[1] = _([["Hello, %s! Are you ready to take part in another sales mission?
    "As you know, the FLF is a heavy user of our ships, but they're also heavy users of Dvaered ships, chiefly the Vendetta design. Since House Dvaered is an enemy of the FLF, we see this as an opportunity to expand our sales: we want to convince the FLF leaders to buy more Nexus ships and fewer Dvaered ships. This will be through a false contraband company so that word doesn't get out that we're supporting terrorists by selling them ships. What do you say? Can you help us once again?"]])

refusetitle = _("Sorry, not interested")
refusetext = _([["Alright, then. I'll see if anyone else is interested."]])

title[2] = _("Good luck")
text[2] = _([["Perfect! So, this mission is pretty simple: I want you to pass on this proposal to them." He hands you a data chip. "It's a request to meet with the FLF leaders on %s. If all goes well, I'll be asking you to take me there next.
    "Any FLF ship should do the job. Try hailing them and see if you get a response. If they won't talk, disable and board them so you can force them to listen. Pretty simple, really. Good luck!"]])

title[3] = _("Good news")
text[3] = _([[Smith is clearly pleased with the results. "I have received word that the FLF leaders are indeed interested. Meet me at the bar whenever you're ready to take me to %s. And here's your payment."]])

title[4] = _("Peaceful Resolution")
text[4] = _([[The FLF ship peacefully responds to you. You explain the details of what is going on and transmit the proposal, after which you both go your separate ways.]])

title[5] = _("Some Resistance Encountered")
text[5] = _([[The FLF officers are clearly ready for battle, but after subduing them, you assure them that you're just here to talk. Eventually, you are able to give them a copy of the proposal and leave peacefully, for lack of a better word.]])

-- Mission details
misn_title = _("The FLF Contact")
misn_reward = _("%s credits")
misn_desc = _("Nexus Shipyards is looking to strike a better deal with the FLF.")

-- NPC
npc_desc[1] = _("Arnold Smith")
bar_desc[1] = _([[It looks like he has yet another job for you.]])

-- OSD
osd_title = _("The FLF Contact")
osd_msg[1] = _("Hail any FLF ship, or disable and board one if necessary")
osd_msg[2] = _("Go back to %s in %s")

function create ()

   --Change here to change the planets and the systems
   paypla, paysys = planet.get("Darkshed")
   nextsys = system.get("Arandon") -- This should be the same as the system used in sh06!

   osd_msg[2] = osd_msg[2]:format(paypla:name(), paysys:name())
   paysys = system.get(paysys:name())
   paypla = planet.get(paypla:name())

   misn.setNPC(npc_desc[1], "neutral/male1")
   misn.setDesc(bar_desc[1])
end

function accept()

   stage = 0
   reward = 1000000

   if tk.yesno(title[1], text[1]:format(player.name())) then
      misn.accept()
      tk.msg(title[2], text[2]:format(nextsys:name()))

      misn.setTitle(misn_title)
      misn.setReward(misn_reward:format(numstring(reward)))
      misn.setDesc(misn_desc)
      osd = misn.osdCreate(osd_title, osd_msg)
      misn.osdActive(1)

      hook.land( "land" )
      hook.hail( "hail" )
      hook.board( "board" )
   else
      tk.msg(refusetitle, refusetext)
      misn.finish(false)
   end
end

function land()
   --Job is done
   if stage == 1 and planet.cur() == paypla then
      tk.msg(title[3], text[3]:format(nextsys:name()))
      player.pay(reward)
      misn.finish(true)
   end
end

function hail( p )
   if stage == 0 and p:faction():name() == "FLF" and not p:hostile() then
      player.commClose()
      tk.msg(title[4], text[4])
      stage = 1
      misn.osdActive(2)
      marker2 = misn.markerAdd(paysys, "low")
   end
end

function board( p )
   if stage == 0 and p:faction():name() == "FLF" then
      player.unboard()
      tk.msg(title[5], text[5])
      stage = 1
      misn.osdActive(2)
      marker2 = misn.markerAdd(paysys, "low")
   end
end
