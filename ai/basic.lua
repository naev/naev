-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
	pushtask(0, "fly")
end

-- Required "attacked" function
function attacked ( attacker )
	task = taskname()
	if task ~= "attack" and task ~= "runaway" then

		-- some taunts
		if attacker == player then
			msg = rng(0,4)
			if msg == 0 then say("You dare attack me!")
			elseif msg == 1 then say("You think that you can take me on?")
			elseif msg == 2 then say("Die!")
			elseif msg == 3 then say("You'll regret this!")
			end
		end

		-- now pilot fights back
		pushtask(0, "attack", attacker)
	end
end

-- runs away
function runaway ()
	target = gettargetid()
	dir = face( target, 1 )
	accel()
	dist = getdist( getpos(target) )
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

