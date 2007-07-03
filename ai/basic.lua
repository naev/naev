
-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
	say("I'm going to kill you!")
	pushtask(0,"attack")
end

function attack ()
	target = 0
	dir = face(target)
	dist = getdist(getpos(target))

	if dir < 10 and dist > 100 then
		accel()
	elseif dir < 10 and dist < 100 then
		shoot()
	end
end
