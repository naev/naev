-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
	task = taskname()
	if task ~= "attack" and task ~= "runaway" then

		-- if getenemy() is 0 then there is no enemy around
		enemy = getenemy()
		if enemy ~= 0 then

			-- taunts!
			num = rng(0,4)
			if num == 0 then msg = "Prepare to be boarded!"
			elseif num == 1 then msg = "Yohoho!"
			elseif num == 2 then msg = "What's a ship like you doing in a place like this?"
			end
			comm(enemy, msg)

			-- make hostile to the enemy (mainly for player)
			hostile(enemy)

			-- proceed to the attack
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


-- runs away from the target
function runaway ()
	target = gettargetid()
	dir = face( target, 1 )
	accel()
end

-- attacks the target
function attack ()
	target = gettargetid()
	dir = face( target )
	dist = getdist( getpos(target) )

	-- must know when to run away
	if parmor() < 70 then
		poptask()
		pushtask(0, "runaway", target)

	-- should try to hurt the target
	elseif dir < 10 and dist > 300 then
		accel()
	elseif dir < 10 and dist < 300 then
		shoot()
	end
end

-- flies to the player, pointless until hyperspace is implemented
function fly ()
	target = player
	dir = face(target)
	dist = getdist( getpos(target) )
	if dir < 10 and dist > 300 then
		accel()
	end
end

