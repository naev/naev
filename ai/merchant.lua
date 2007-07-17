-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
	if taskname() == "none" then
		local planet = getrndplanet()
		pushtask(0, "goto", planet)
	end
end

-- Required "attacked" function
function attacked ( attacker )
	if taskname() ~= "runaway" then

		-- some messages
		if attacker == player then
			local msg = rng(0,4)
			if msg == 0 then say("Why don't you pick on someone your own size")
			elseif msg == 1 then say("We are just a merchant vessle")
			elseif msg == 2 then say("Don't kill us please")
			end
		end

		-- Sir Robin bravely ran away
		pushtask(0, "runaway", attacker)
	end
end

-- runs away
function runaway ()
	local target = gettargetid()
	local dir = face( target, 1 )
	accel()
end

-- flies to the target
function goto ()
	local target = gettarget()
	local dir = face(target)
	local dist = getdist( target )
	local bdist = minbrakedist()
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

