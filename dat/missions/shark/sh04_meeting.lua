--[[

    This is the fifth mission of the Shark's teeth campaign. The player has to go to a planet in Za'lek space.

    Stages :
    0) Way to Za'lek system
    1) Way back to Darkshed
	
	TODO : I'm not really happy with the drone's behaviour : it's quite too obvious
	
--]]

--needed scripts
include "proximity.lua"

lang = naev.lang()
if lang == "es" then
else -- default english
    title = {}
    text = {}
    osd_msg = {}
    npc_desc = {}
    bar_desc = {}
    
    title[1] = "Travel"
    text[1] = [["Ok, are you ready for the travel to Curie in Kohler?"]]
	
    refusetitle = "Sorry, not interested"
    refusetext = [["Ok, so come back when you are ready," Smith says.]]

    title[2] = "Time to go"
    text[2] = [["So let's go"]]
	
    title[3] = "End of mission"
    text[3] = [[Smith gets out of your ship and looks at you, smiling: "You know, it's like that in our kind of jobs. Sometimes it works and sometimes it fails. It's not our fault.
	Anyway, here is your pay, and goodbye."]]

    title[4] = "The meeting"
    text[4] = [[As you land, you see a group of people that were waiting for your ship. Smith hails them and says you to wait in the ship while he goes to a private part of the bar.
	A few STP later, he comes back and explains you that he didn't manage to get the support of the councillor, what means that the Frontier will not buy the Sharks.
	"Anyway," he says, "bring me back to Alteris in one piece and I will pay you."]]

    title[5] = "What is going on?"
    text[5] = [[That drone was behaving strangely and now, it is attacking you. As you wonder what to do, you hear a comm from a remote Za'lek ship : "Attention please, it seems some of our drones have been hacked. If a drone is attacking you and you aren't wanted by the authorities, you have exceptionnaly the autorization to destroy it."
	"Incredible, "Smith says, "they have managed to hire a Za'lek military engineer who has hacked some drones in order to make them attack our ship! That's strong." He says, admiring.]]
	
    -- Mission details
    misn_title = "The Meeting"
    misn_reward = "50 000 credits"
    misn_desc = "Nexus Shipyard asks you to take part in a secret meeting"
   
    -- NPC
    npc_desc[1] = "Arnold Smith"
    bar_desc[1] = [[What wouldn't this guy do to sell Sharks?]]
	
    -- OSD
    osd_title = "The Meeting"
    osd_msg[1] = "Go to Kohler and land on Curie"
    osd_msg[2] = "Bring Smith back to Darkshed in Alteris"

end

function create ()

    --Change here to change the planets and the systems
    mispla = planet.get("Curie")
    missys = system.get("Kohler")
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
    proba = 0.3  --the probability of ambushes will change
    firstambush = true  --In the first ambush, there will be a little surprise text
	
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
    --The player is landing on the mission planet
    if stage == 0 and planet.cur() == mispla then
	tk.msg(title[4], text[4])
	stage = 1
	misn.osdActive(2)
        misn.markerRm(marker)
	marker2 = misn.markerAdd(paysys, "low")
    end
	
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
    --This timer will ensure that the hacked drones don't reveal themselves during the jumping
    enable = false
    hook.timer(5000,"enabling")
    -- Ambush !
    if system.cur():presence(faction.get("Za'lek")) > 50 then  -- Only in Za'lek space
        if stage == 0 and rnd.rnd() < proba then
	    ambush()
            --proba = proba - 0.1 is done in ambush only if the drones are revealed
	else
	    --the probality of an ambush grows up when you cross a system without meeting any ennemy
	    proba = proba + 0.2
	end
    end
end

function ambush()
    badguy = {}
    number = {1,2,3,4}
    tnumber = {1,2,3,4,5,6,7,8,9,10,11,12}
	
    for i in ipairs(number) do
	badguy[i] = pilot.add( "Za'lek Light Drone", nil, 0 )[1]		
    end
	
    for i in ipairs(number) do
	j = i+4
	badguy[j] = pilot.add( "Za'lek Heavy Drone", nil, 0 )[1]
    end

    for i in ipairs(number) do
	j = i+8
	badguy[j] = pilot.add( "Za'lek Bomber Drone", nil, 0 )[1]
    end
	
    for i in ipairs(tnumber) do
	--Makes the drones follow the player
	badguy[j]:control()
	badguy[j]:follow(player.pilot())
		
	--as the player approaches, the drones reveal to be bad guys!
	hook.timer(500, "proximity", {anchor = badguy[j], radius = 1000, funcname = "reveal"})
    end
	
end

function abort()
    misn.cargoRm(smith)
    misn.finish(false)
end

function reveal()  --transforms the spawn drones into baddies
    if enable == true then  --only if this happends a few time after the jumping/taking off
	for i in ipairs(tnumber) do
	    badguy[i]:rename("Hacked Drone")
            badguy[i]:setHostile()
	    badguy[i]:setFaction("Mercenary")
	    badguy[i]:control(false)
	end
	if firstambush == true then
	    --Surprise message
            tk.msg(title[5], text[5])
	    firstambush = false
	end
	proba = proba - 0.1  --processing the probability change
    end
end

function enabling()
    enable = true
end
