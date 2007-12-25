-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
	task = ai.taskname()

	enemy = ai.getenemy()
	if enemy ~= 0 then
		ai.pushtask(0, "attack", enemy)

	elseif ai.taskname() == "none" then
		
		ai.pushtask(0, "fly")
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
		ai.broadcast("This area is under survellance, do not attempt anything funny")
	end
end

-- taunts
function taunt ( target )
	num = ai.rnd(0,4)
	if num == 0 then msg = "You dare attack me!"
	elseif num == 1 then msg = "You think that you can take me on?"
	elseif num == 2 then msg = "Die!"
	elseif num == 3 then msg = "You'll regret this!"
	end
	if msg then ai.comm(attacker, msg) end
end

-- runs away
function runaway ()
	target = ai.targetid()

	-- make sure pilot exists
	if not ai.exists(target) then
		ai.poptask()
		return
	end
		
	dir = ai.face( target, 1 )
	ai.accel()
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


	if ai.parmour() < 70 then
		ai.poptask()
		ai.pushtask(0, "runaway", target)
	elseif dir < 10 and dist > 300 then
		ai.accel()
	elseif dir < 10 and dist < 300 then
		ai.shoot()
	end
end

-- flies to the player
function fly ()
	target = player
	dir = ai.face(target)
	dist = ai.dist( ai.pos(target) )
	if dir < 10 and dist > 300 then
		ai.accel()
	end
end

