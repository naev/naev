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
		pushtask(0, "attack", attacker)
	end
end

-- runs away
function runaway ()
	target = gettargetid()
	dir = face( target, 1 )
	accel()
	dist = getdist( getpos(target) )

	if dist > 800 then
		say("So longer sucker!")
	end
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

