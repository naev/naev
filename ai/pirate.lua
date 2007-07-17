-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
	if taskname() ~= "attack" then
		enemy = getenemy()
		if enemy ~= -1 then
			pushtask(0, "attack", enemy)
		else
			pushtask(0, "fly")
		end
	end
end

-- Required "attacked" function
function attacked ( attacker )
	task = taskname()
	if task ~= "attack" and task ~= "runaway" then
		taunt()
		pushtask(0, "attack", attacker)
	elseif task == "attack" then
		if gettargetid() ~= attacker then
			pushtask(0, "attack", attacker)
		end
	end
end


function taunt ()
		-- some taunts
		num = rng(0,4)
		if num == 0 then msg = "You dare attack me!"
		elseif num == 1 then msg = "You think that you can take me on?"
		elseif num == 2 then msg = "Die!"
		elseif num == 3 then msg = "You'll regret this!"
		end
		if msg then comm(attacker, msg) end
end


-- runs away
function runaway ()
	target = gettargetid()
	dir = face( target, 1 )
	accel()
end

-- attacks
function attack ()
	target = gettargetid()
	dir = face( target )
	dist = getdist( getpos(target) )

	if parmor() < 70 then
		poptask()
		pushtask(0, "runaway", target)
	elseif dir < 10 and dist > 300 then
		accel()
	elseif dir < 10 and dist < 300 then
		shoot()
	end
end

-- flies to the player
function fly ()
	target = 0
	dir = face(target)
	dist = getdist( getpos(target) )
	if dir < 10 and dist > 300 then
		accel()
	end
end

