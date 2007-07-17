-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
	if taskname() == "none" then
		planet = getrndplanet()
		pushtask(0, "goto", planet)
	end
end

-- Required "attacked" function
function attacked ( attacker )
	if taskname() ~= "runaway" then

		-- some messages
		num = rng(0,3)
		if num == 0 then msg = "Mayday! We are under attack!"
		elseif num == 1 then msg = "Requesting assistance.  We are under attack!"
		elseif num == 2 then msg = "Merchant vessle here under attack! Help!"
		end
		if msg then broadcast(msg) end

		-- Sir Robin bravely ran away
		pushtask(0, "runaway", attacker)
	end
end

-- runs away
function runaway ()
	target = gettargetid()
	dir = face( target, 1 )
	accel()
end

-- flies to the target
function goto ()
	target = gettarget()
	dir = face(target)
	dist = getdist( target )
	bdist = minbrakedist()
	if dir < 10 and dist > bdist then
		accel()
	elseif dir < 10 and dist < bdist then
		poptask()
		pushtask(0,"stop")
	end
end

-- brakes
function stop ()
	brake()
	if isstopped() then
		poptask()
		settimer(0, rng(3000,5000))
		pushtask(0,"land")
	end
end


-- waits
function land ()
	if timeup(0) then
		pushtask(0,"runaway",player)
	end
end

