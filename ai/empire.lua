-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
	task = ai.taskname()

	enemy = ai.getenemy()
	if enemy ~= 0 then
		ai.pushtask(0, "attack", enemy)

	elseif task == "none" then
		planet = ai.rndplanet()
		-- planet must exist
		if planet == nil then
			ai.pushtask(0, "hyperspace")
		else
			ai.pushtask(0, "goto", planet)
		end
	end
end

-- Required "attacked" function
function attacked ( attacker )
	task = ai.taskname()
	if task ~= "attack" and task ~= "runaway" then

		-- some taunting
		taunt( attacker )

		-- now pilot fights back
		ai.pushtask(0, "attack", attacker)

	elseif task == "attack" then
			if ai.targetid() ~= attacker then
				ai.pushtask(0, "attack", attacker)
			end
	end
end

function create ()
	if ai.rnd(0,2)==0 then
		ai.broadcast("The Empire is watching you.")
	end
end

-- taunts
function taunt ( target )
	num = ai.rnd(0,4)
	if num == 0 then msg = "You dare attack me!"
	elseif num == 1 then msg = "You are no match for the Empire!"
	elseif num == 2 then msg = "The Empire will have your head!"
	elseif num == 3 then msg = "You'll regret this!"
	end
	if msg then ai.comm(attacker, msg) end
end

-- attacks
function attack ()
	target = ai.targetid()

	-- make sure pilot exists
	if not ai.exists(target) then
		ai.poptask()
		return
	end

	dir = ai.face( target )
	dist = ai.dist( ai.pos(target) )
	second = ai.secondary()

	if ai.secondary() == "Launcher" then
		ai.settarget(target)
		ai.shoot(2)
	end


	if dir < 10 and dist > 300 then
		ai.accel()
	elseif (dir < 10 or ai.hasturrets()) and dist < 300 then
		ai.shoot()
	end
end



-- flies to the target
function goto ()
	target = ai.target()
	dir = ai.face(target)
	dist = ai.dist( target )
	bdist = ai.minbrakedist()
	if dir < 10 and dist > bdist then
		ai.accel()
	elseif dir < 10 and dist < bdist then
		ai.poptask()
		ai.pushtask(0,"stop")
	end
end

-- brakes
function stop ()
	if ai.isstopped() then
		ai.stop()
		ai.poptask()
		ai.settimer(0, ai.rnd(8000,15000))
		ai.pushtask(0,"land")
	else
		ai.brake()
	end
end

-- waits
function land ()
	if ai.timeup(0) then
		ai.pushtask(0,"hyperspace")
	end
end

-- goes hyperspace
function hyperspace ()
	dir = ai.face(-1) -- face away from (0,0)
	if (dir < 10) then
		ai.accel()
	end
end

