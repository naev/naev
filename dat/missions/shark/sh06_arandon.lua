--[[

    This is the seventh mission of the Shark's teeth campaign. The player has to meet the FLF in Arandon.

    Stages :
    0) Way to Arnadon
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
    
    title[1] = "Let's go"
    text[1] = [["Is your ship ready for the dangers of the Nebula?"]]
	
    refusetitle = "Sorry, not interested"
    refusetext = [["Come back when you are ready."]]

    title[2] = "Go"
    text[2] = [[Smith once again steps in your ship in order to go to a meeting.]]
	
    title[3] = "Well done!"
    text[3] = [["Here is your pay," says Smith. I will be in the bar if I have an other task for you.]]

    title[4] = "The Meeting"
    text[4] = [[As you board it, Arnold Smith insists to step alone in the FLF's ship. A few hours later, he comes back, satisfied. It seems, this time luck is on your side. "They will buy us tons of this damn "Shark", all we have to do now is to fix a few details, so let's go back to Alteris," he says happily.
	You unboards, wondering what kind of details he could be thinking about...]]
	
    title[5] = "Hail"
    text[5] = [[As you hail him, the Pacifier commander answers you and stops his ship, waiting to be boarded]]
	
    -- Mission details
    misn_title = "A Journey To Arandon"
    misn_reward = "50 000 credits"
    misn_desc = "Nexus Shipyard asks you to take contact with the FLF"
   
    -- NPC
    npc_desc[1] = "Arnold Smith"
    bar_desc[1] = [[It's fun to see how this guy's dishonesty has led him to help the most idelaistic group in the galaxy.]]
	
    -- OSD
	osd_title = "A Journey To Arandon"
    osd_msg[1] = "Go to Arandon and whait for the FLF ship, then hail and board it."
	osd_msg[2] = "Go back to Darkshed in Alteris"

end

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
    reward = 50000
	
    if tk.yesno(title[1], text[1]) then
        misn.accept()
        tk.msg(title[2], text[2])

        misn.setTitle(misn_title)
        misn.setReward(misn_reward)
        misn.setDesc(misn_desc)
        misn.osdCreate(misn_title, osd_msg)
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
	pacifier = pilot.add( "FLF Pacifier", nil, 0 )[1]
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
    tk.msg(title[4], text[4])
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