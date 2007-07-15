-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
	if taskname() == "none" then
		pushtask(0, "fly")
	end
end

-- Required "attacked" function
function attacked ( attacker )
	if taskname() ~= "runaway" then

		-- some messages
		if attacker == player then
			msg = rng(0,4)
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
	target = gettargetid()
	dir = face( target, 1 )
	accel()
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

