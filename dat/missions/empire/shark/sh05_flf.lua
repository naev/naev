--[[

    This is the sixth mission of the Shark's teeth campaign. The player has to take contact with the FLF.

    Stages :
    0) Way to Sindbad/Surano
    1) Way back to Darkshed
	
--]]

lang = naev.lang()
if lang == "es" then
else -- default english
    title = {}
    text = {}
    osd_msg = {}
    npc_desc = {}
    bar_desc = {}
    
    title[1] = "Nice to see you again!"
    text[1] = [[As you sit down at his table, Arnold Smith welcomes you with his trademark smile: "Hello, old boy, are you ready to take part to another sales project?" He sobers. "This time, it's serious game. I really need your help to get in touch with somebody who could be very interested in the Shark.
	The problem is that this person is very difficult to spot." You ask him if he wants you to sell imperial fighters to a pirate, but he answers: "No, it isn't precisely that. In fact, we are looking for a contact with the FLF. Do you think that you can do it?"]]
	
    refusetitle = "Sorry, not interested"
    refusetext = [["Oh, I see, so, I guess, I'll have to find somebody else..." Smith says.]]

    title[2] = "Good luck"
    text[2] = [["Ok, that's good news for me. I don't know if you know people in there, and I don't want to know. If you don't, I suggest you to try to hail a lone FLF ship. I think you could encounter some in Surano.
	Maybe try to disable a FLF ship, board it and explain to its crew what we want. If you succeed, set up a meeting in Arandon. I don't think anyone will disturb us there.
	Ah, and don't mention the company for which we are working."]]
	
    title[3] = "Good news"
    text[3] = [[As you land, you see Smith coming towards you. When you finish explaining to him that you managed to make contact with the FLF, he seems happy and pays you your fee.
	"Now, meet me in the bar when you are ready to bring me to Arandon."]]

    title[4] = "The FLF crew"
    text[4] = [[As you board the ship, the FLF crew seems to be ready to die at their posts. Instead of using blasters to cook them all, you start to explain them that you have been sent by a friend of the FLF who wants to sell ships to them. You write on a papersheet all the information they need to give to their officers, and leave the ship."]]

    title[5] = "The FLF executive"
    text[4] = [[As you board the ship, the FLF crew seems to be ready to die at their posts. Instead of using blasters to cook them all, you start to explain them that you have been sent by a friend of the FLF who wants to sell ships to them
    text[5] = [[After a short hesitation, you approach the man, who seems at first to not want to talk to you. After you explain that you were sent by a company that wants to sell fighters to the FLF, he looks at you, surprised. You explain everything about the meeting in Arandon and he answers you: "Well, I will consult my superiors. If we don't come, you will know that we are not interested."]]
	
    -- Mission details
    misn_title = "The FLF Contact"
    misn_reward = "200 000 credits"
    misn_desc = "Nexus Shipyard asks you to take contact with the FLF"
   
    -- NPC
    npc_desc[1] = "Arnold Smith"
    bar_desc[1] = [[It seems Nexus Shipyards is still looking for Shark customers.]]
    npc_desc[2] = "FLF executive"
    bar_desc[2] = [[This guy looks important]]
	
    -- OSD
    osd_title = "The FLF Contact"
    osd_msg[1] = "Placeholder"
    osd_sindbad = "Go to Surano, disable and board a FLF ship or go to Sindbad and speak to a FLF officer."
    osd_nosindbad = "Go to Surano, disable and board a FLF ship"
    osd_msg[2] = "Go back to Darkshed in Alteris"

end

function create ()

    --Change here to change the planets and the systems
    missys = system.get("Surano")
    pplname = "Darkshed"
    psyname = "Alteris"
	
    --Does the player have access to Sindbad
    if diff.isApplied( "FLF_base") then
	sindbad = planet.get("Sindbad")
	osd_msg[1] = osd_sindbad
    else
	osd_msg[1] = osd_nosindbad
    end
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
    reward = 200000
	
    if tk.yesno(title[1], text[1]) then
        misn.accept()
        tk.msg(title[2], text[2])

        misn.setTitle(misn_title)
        misn.setReward(misn_reward)
        misn.setDesc(misn_desc)
        misn.osdCreate(misn_title, osd_msg)
        marker = misn.markerAdd(missys, "low")
        
	landhook = hook.land("land")
	enterhook = hook.enter("enter")
    else
        tk.msg(refusetitle, refusetext)
        misn.finish(false)
    end
end

function land()
    --The player is landing on Sindbad
    if stage == 0 and planet.cur() == sindbad then
	flfguy = misn.npcAdd("talktoguy", npc_desc[2], "neutral/thief1", bar_desc[2])
    end
	
    --Job is done
    if stage == 1 and planet.cur() == paypla then
	tk.msg(title[3], text[3])
        player.pay(reward)
        misn.finish(true)
    end
end

function talktoguy()
    tk.msg(title[5], text[5])
    stage = 1
    misn.osdActive(2)
    misn.markerRm(marker)
    marker2 = misn.markerAdd(paysys, "low")
end

function enter()
    --Entering in Surano in order to find and disable a FLF Lancelot
    if system.cur() == missys then
	--Lets unspawn everybody
	pilot.clear()
	pilot.toggleSpawn(false)
		
	lancelot = pilot.add( "FLF Lancelot", nil, 0 )[1]
	hook.pilot(lancelot, "board", "discuss")
    end
end

function discuss()
    tk.msg(title[4], text[4])
    stage = 1
    misn.osdActive(2)
    misn.markerRm(marker)
    marker2 = misn.markerAdd(paysys, "low")
end
