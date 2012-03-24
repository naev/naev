--Preliminary draft of a new event where the player meets one of the Touched, who tries to convert him
--Sudarshan S <ssoxygen@users.sf.net>

include("scripts/fleethelper.lua")

commtitle="The preaching begins..."
commtext=[[A Sirian appears on your viewscreen. He seems different than most Sirii you've met. He regards you with a neutral yet intense gaze.
"Man is cruel and deceptive," he says. "You deserve more than you shall ever get from humanity. Your only hope is to follow the Holy One, Sirichana. He shall guide you to peace and wisdom. He is the sole refuge for humans like you and me. You MUST follow him!"

You feel a brief but overpowering urge to follow him, but it passes and your head clears. The Sirian ship makes no further attempt to communicate with you.]]

althoughEnemy={
"%s, although you are an enemy of House Sirius, I shall not attack unless provoked, for I abhor violence!",
"%s, although you are an enemy of House Sirius, I shall not attack unless provoked, for I believe mercy is a great Truth!",
"%s, although you are an enemy of House Sirius, I shall not attack unless provoked, for you too are Sirichana's child!"
}

friend={
"%s, I foresee in you a great Sirian citizen, and I look forward to your friendship!",
"%s, I foresee a bright future for you, illuminated by Sirichana's light!",
"%s, may Sirichana's light illuminate your path!"
}

followSirichana={
"You shall all follow Sirichana henceforth!",
"Sirichana shall lead you to peace and wisdom!",
"Sirichana is the Father of you all!",
"Sirichana's grace shall liberate you!",
"May Sirichana's light shine on you henceforth!"
}

praiseSirichana={
"We shall all follow Sirichana now!",
"We have been liberated from our evil ways!",
"No more shall we tread the path of evil!",
"We see the True path now!",
"No more shall we commit sins!"
}

attackerPunished={
"Serves you right for attacking a Touched!",
"Fry in hell, demon!",
"May you suffer eternal torment!",
"Your doom is Sirichana's curse!"
}

attackersDead={
"All the attackers are dead!",
"We can resume our Quest now!",
"The glory of Sirichana remains unblemished!",
"All heretics have been destroyed!"
}

whatHappened={
"Do you think everyone can be brainwashed?",
"You shall convert no more of us!",
"Some of us shall not be converted, fool!",
"You'll never convert me!",
"I shall never be converted!"
}

presence={
"You feel an overwhelming presence nearby!",
"Something compels you to stop",
"You are jerked awake by a mysterious but compelling urge",
"You feel... Touched... by a magical power"
}

startCombat={
"Die, heretics!",
"Those who insult the Sirichana shall die!",
"You've commited an unpardonable sin!",
"Hell awaits, fools!"
}

preacherDead={
"Oh no! The Touched One is dead!",
"Sirichana save our souls!",
"We shall never forget You, O Touched One!",
"We swear eternal revenge!"
}

urge={
"You feel an overwhelming urge to hear him out!",
"A mysterious force forces you to listen!",
"You feel compelled to listen!"
}

dyingMessage={
"With my dying breath, I curse you!",
"Sirichana speed you to hell!",
"Sirichana, I did my best!"
}

dead={
"The Reverance is dead!",
"Someone killed the preacher!"
}

--initialize the event
function create()
	curr=system.cur() --save the current system
	
	v = var.peek( "si_convert" ) -- Get the value
	if v == nil then -- Doesn't exist, so create
		var.push( "si_convert", 1 )
	else
		var.push( "si_convert", v+1 )
	end
	
	--start the fun when the player jumps
	hook.jumpin("funStartsSoon")
	hook.land("cleanup") --oops he landed
end

--Start the real mission after a short delay
function funStartsSoon()
	playerP=player.pilot() --save player's pilot
	rep=faction.playerStanding(faction.get("Sirius"))
	hook.timer(5000, "theFunBegins") --for effect, so that we can see them jumping in!
end

--the preaching's about to begin!
function theFunBegins()
	if rep < 0 then
		local dist = vec2.dist(jump.get(system.cur(),curr):pos(),player.pos()) --please note the order of system.cur() and curr matters!
		if dist < 6000 then
			hook.timer(5000,"theFunBegins") --wait some more time
			return
		end
	end
	--summon a preacher from the jump point and highlight him and take control and focus on him
	preacher=pilot.addRaw("Sirius Reverance", "sirius_norun", curr, "Sirius")[1]
	preacher:setHilight()
	preacher:setVisplayer()
	preacher:control()
	preacher:broadcast(followSirichana[rnd.rnd(1,#followSirichana)],true)
	preacher:hailPlayer()
	playerP:setInvincible()
	
	--set needed hooks
	hook.pilot(preacher,"attacked","violence")
	hook.pilot(preacher,"death","badCleanup")
	hook.pilot(preacher,"land","landCleanup")
	hook.pilot(preacher,"jump","jumpCleanup")
	hook.jumpout("cleanup")
	
	camera.set(preacher, true)
	player.cinematics(true,{gui=true, abort=presence[rnd.rnd(1,#presence)]})
	
	--you're hooked till you hear him out!
	playerP:control()
	player.msg(urge[rnd.rnd(1,#urge)])
	
	--create a random band of converted pirate followers
	local followerShips = {"Pirate Kestrel", "Pirate Admonisher", "Pirate Shark", "Pirate Vendetta", "Pirate Rhino"} --the types of followers allowed
	followers = {}
	local numships = rnd.rnd(2, 6) -- This is the total number of converted follower ships.
	
	for num=1, numships, 1 do
		followers[num] = followerShips[rnd.rnd(1, #followerShips)] -- Pick a follower ship at random.
	end
	
	followers = addRawShips(followers, "sirius_norun", curr, "Sirius") -- The table now contains pilots, not ship names.
	for _,j in ipairs(followers) do
		j:rename("Converted "..j:name())
	end
	
	--pick a random converted pirate and have him praise the Sirichana
	praiser=followers[rnd.rnd(1,#followers)]
	
	--add some sirian escorts too
	local sirianFollowers = {"Sirius Fidelity","Sirius Shaman"} --the types of followers allowed
	local sirianFollowerList = {}
	
	numships = rnd.rnd(2, 6) -- This is the total number of sirian escort ships.
	for num=1, numships, 1 do
		sirianFollowerList[num] = sirianFollowers[rnd.rnd(1, #sirianFollowers)] -- Pick a follower ship at random.
	end
	
	sirianFollowers = addRawShips(sirianFollowerList, "sirius_norun", curr, "Sirius") -- The table now contains pilots, not ship names.
	
	for _, j in ipairs(sirianFollowers) do
		followers[#followers + 1] = j
	end
	
	--set up a table to store attackers
	attackers={}
	
	--make these followers follow the Touched one
	--if Sirius is an enemy still keep these guys neutral... at first
	for _, j in ipairs(followers) do
		j:setFriendly()
		j:control()
		j:follow(preacher)
		hook.pilot(j,"attacked","violence")
	end
	preacher:setFriendly()
	
	--pick a random follower and have him praise Sirichana, after a delay
	hook.timer(4000,"praise")
	
	--have the preacher say something cool
	hook.timer(8000,"preacherSpeak")
	
	--add some normal pirates for fun :)
	hook.timer(12500,"pirateSpawn")
	
	--hook up timers for releasing cinematics (and you of course :P)
	hook.timer(17500,"release")
	
	--hook up timer for re-hailing player
	hailHook=hook.date(time.create(0, 0, 1000), "reHail") --hail every 1000 STU till player answers
	
	--when hailed, the preacher preaches to you
	hook.pilot(preacher, "hail", "hail")
end

function preacherSpeak()
	camera.set(preacher,true)
	if rep < 0 then
		preacher:comm(string.format(althoughEnemy[rnd.rnd(1,#althoughEnemy)],player.name()), true)
	else
		preacher:comm(string.format(friend[rnd.rnd(1,#friend)],player.name()), true)
	end
end

--re-hail the player
function reHail()
	if preacher:exists() then
		preacher:hailPlayer()
	end
end

--random praise for the Sirichana
function praise()
	camera.set(praiser,true)
	praiser:broadcast(praiseSirichana[rnd.rnd(1,#praiseSirichana)],true)
end

--spawn some enemy pirates for fun :P
--to add even more fun have them say something cool
function pirateSpawn()
	local numships=rnd.rnd(2,5)
	local curiousNumber=rnd.rnd(1,numships)
	local shiptype={"Pirate Shark","Pirate Vendetta"}
	local thepilot
	for num=1,numships,1 do
		thepilot = pilot.add(shiptype[rnd.rnd(1,#shiptype)], nil, curr)[1]
		if num==curiousNumber then
			thepilot:broadcast(whatHappened[rnd.rnd(1,#whatHappened)],true)
			camera.set(thepilot,true)
		end
		thepilot:control()
		thepilot:attack(followers[rnd.rnd(1,#followers)])
	end
end

--called when a new attack happens
function violence(attacked,attacker)
	if #attackers == 0 then --we have to change the group to battle mode
		attacked:broadcast(startCombat[rnd.rnd(1,#startCombat)],true)
		preacher:control(false)
		for _, j in ipairs(followers) do
			if j:exists() then
				j:control(false)
			end
		end
	end
	local found=false
	for _,j in ipairs(attackers) do
		if j==attacker then
			found=true
			break
		end
	end
	if not found then --new attacker
		attackers[#attackers+1]=attacker
		hook.pilot(attacker,"exploded","anotherdead")
		hook.pilot(attacker,"land","anotherdead")
		hook.pilot(attacker,"jump","anotherdead")
	end
end

--another enemy is dead
function anotherdead(enemy, attacker)

	if attacker==nil then --in case the pilot was blown up by an explosion
		attacker=preacher
	end

	if attacker:exists() then --in case the attacker was parallely killed
		attacker:broadcast(attackerPunished[rnd.rnd(1,#attackerPunished)],true)
	end

	--find and remove the enemy
	for i,j in ipairs(attackers) do
		if j==enemy then
			table.remove(attackers,i)
			break
		end
	end

	if #attackers == 0 then --last one was killed, restore idle mode
		attacker:broadcast(attackersDead[rnd.rnd(1,#attackersDead)],true)
		restoreControl()
	end
end

--finds and set a new target for the preacher, when he is outta battle mode
function getPreacherTarget()
	local sirius=faction.get( "Sirius" )

	--look for nearby landable sirian planet to land
	for key, planet in ipairs( system.cur():planets() ) do
		if planet:faction()==sirius and planet:services()["land"]  then
			target = planet
			break
		end
	end
	
	--if no landable sirian planets found, jump to random system
	--TODO: prevent jump back through the entry point
	if target then
		preacher:land(target)
	else
		preacher:hyperspace()
	end
end

--restores control to the idle mode
function restoreControl()
	preacher:control()
	for _, j in ipairs(followers) do
		if j:exists() then
			j:control()
			j:follow(preacher)
		end
	end
	getPreacherTarget()
end

--releases the player after the cutscene
function release()
	camera.set(nil, true)
	player.cinematics(false)
	playerP:setInvincible(false)
	playerP:control(false)
	--if the attacks have already started, we shouldn't set a target yet
	if #attackers==0 then
		getPreacherTarget()
	end
end

--when hailed back, show the message
function hail()
	tk.msg(commtitle,commtext)
	player.commClose()
	hook.rm(hailHook) --no more hailing
end

--everything is done
function cleanup()
	player.pilot():setInvincible(false)
	evt.finish()
end

--oops, it seems the preacher died. End gracefully
function badCleanup()
	playerP:setInvincible(false)
	player.msg(dead[rnd.rnd(1,#dead)])
	preacher:broadcast(dyingMessage[rnd.rnd(1,#dyingMessage)])
	local survivors={}
	for _,j in ipairs(followers) do
		if j:exists() then
			j:control(false)
			survivors[#survivors+1]=j
		end
	end
	if #survivors > 0 then
		follower=survivors[rnd.rnd(1,#survivors)]
		follower:broadcast(preacherDead[rnd.rnd(1,#preacherDead)],true)
	end
	evt.finish()
end

--the preacher has landed. Land all his followers too
function landCleanup()
	playerP:setInvincible(false)
	for _,j in ipairs(followers) do
		if j:exists() then
			j:taskClear()
			j:land(target)
		end
	end
	evt.finish()
end

--the preacher has jumped. Jump all his followers too
function jumpCleanup()
	playerP:setInvincible(false)
	for _,j in ipairs(followers) do
		if j:exists() then
			j:taskClear()
			j:hyperspace(target,true) --attack back as they move away?
		end
	end
	evt.finish()
end
