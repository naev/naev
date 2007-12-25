-- Required control rate
control_rate = 2
-- Required "control" function
function control ()
	if ai.taskname() == "none" then
		enemy = ai.getenemy()

		if enemy ~= 0 then
			ai.pushtask(0, "attack", enemy)
		end
	end
end
-- Required "attacked" function
function attacked ( attacker )
	task = ai.taskname()
	if task ~= "attack" then

		-- now pilot fights back
		ai.pushtask(0, "attack", attacker)

	elseif task == "attack" then
			if ai.targetid() ~= attacker then
				ai.pushtask(0, "attack", attacker)
			end
	end
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
	elseif dir < 10 and dist < 300 then
		ai.shoot()
	end
end
