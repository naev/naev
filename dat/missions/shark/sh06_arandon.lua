--[[

   This is the seventh mission of the Shark's teeth campaign. The player has to meet the FLF in Arandon.

   Stages :
   0) Way to Arandon
   1) Way back to Darkshed

--]]

include "numstring.lua"

title = {}
text = {}
osd_msg = {}
npc_desc = {}
bar_desc = {}

title[1] = _("Let's go")
text[1] = _([["Is your ship ready for the dangers of the Nebula?"]])

refusetitle = _("...Or not")
refusetext = _([["Come back when you are ready."]])

title[2] = _("Go")
text[2] = _([[Smith once again steps in your ship in order to go to a meeting.]])

title[3] = _("Well done!")
text[3] = _([[Smith thanks you for the job well done. "Here is your pay," he says. "I will be in the bar if I have another task for you."]])

title[4] = _("The Meeting")
text[4] = _([[As you board, Arnold Smith insists on entering the FLF's ship alone. A few hours later, he comes back, satisfied. It seems this time luck is on his side. He mentions that he had good results with a smile on his face before directing you to take him back to %s.]])

title[5] = _("Hail")
text[5] = _([[The Pacifier commander answers you and stops his ship, waiting to be boarded.]])

-- Mission details
misn_title = _("A Journey To %s")
misn_reward = _("%s credits")
misn_desc = _("You are to transport Arnold Smith to %s so that he can talk about a deal.")

-- NPC
npc_desc[1] = _("Arnold Smith")
bar_desc[1] = _([[He's waiting for you.]])

-- OSD
osd_title = _("A Journey To %s")
osd_msg[1] = _("Go to %s and wait for the FLF ship, then hail and board it.")
osd_msg[2] = _("Go back to %s in %s")

function create ()

   --Change here to change the planets and the systems
   missys = system.get("Arandon")
   pplname = "Darkshed"
   psyname = "Alteris"
   paysys = system.get(psyname)
   paypla = planet.get(pplname)

   if not misn.claim(missys) then
      misn.finish(false)
   end

   misn.setNPC(npc_desc[1], "neutral/male1")
   misn.setDesc(bar_desc[1])
end

function accept()

   stage = 0
   reward = 750000

   if tk.yesno(title[1], text[1]) then
      misn.accept()
      tk.msg(title[2], text[2])

      osd_msg[1] = osd_msg[1]:format(missys:name())
      osd_msg[2] = osd_msg[2]:format(paypla:name(), paysys:name())

      misn.setTitle(misn_title:format(missys:name()))
      misn.setReward(misn_reward:format(numstring(reward)))
      misn.setDesc(misn_desc:format(missys:name()))
      osd = misn.osdCreate(osd_title:format(missys:name()), osd_msg)
      misn.osdActive(1)

      marker = misn.markerAdd(missys, "low")

      smith = misn.cargoAdd("Person", 0)  --Adding the cargo

      landhook = hook.land("land")
      enterhook = hook.enter("enter")
      else
      tk.msg(refusetitle, refusetext)
      misn.finish(false)
   end
end

function land()
   --Job is done
   if stage == 1 and planet.cur() == paypla then
      if misn.cargoRm(smith) then
         tk.msg(title[3], text[3])
         player.pay(reward)
         misn.osdDestroy(osd)
         hook.rm(enterhook)
         hook.rm(landhook)
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
      hook.timer(10000,"flf_people")

   end
end

function flf_people()
   pacifier = pilot.add( "FLF Pacifier", nil, "Doeston" )[1]
   pacifier:setFriendly()
   pacifier:setInvincible(true)
   hook.pilot(pacifier, "hail", "hail_pacifier")
   hook.pilot( pacifier, "death", "dead" )
   hook.pilot( pacifier, "jump", "jump" )
end

function hail_pacifier()
   --hailing the pacifier
   tk.msg(title[5], text[5])
   pacifier:control()
   pacifier:brake()
   pacifier:setActiveBoard(true)
   hook.pilot(pacifier, "board", "board")
end

function board()
   --boarding the pacifier
   tk.msg(title[4], text[4]:format(paysys:name()))
   player.unboard()
   pacifier:control(false)
   pacifier:setActiveBoard(false)
   stage = 1
   misn.osdActive(2)
   misn.markerRm(marker)
   marker2 = misn.markerAdd(paysys, "low")
end

function dead()  --Actually, I don't know how it could happend...
   misn.finish(false)
end

function jump()
   misn.finish(false)
end

function abort()
   misn.cargoRm(smith)
   misn.finish(false)
end
