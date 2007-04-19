
-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
	pushtask(0,"follow");
end

function follow ()
	target = 1
	dir = face(target)
	dist = getdist(getpos(target))

	if dir < 10 and dist > 100 then
		accel( dist/100-1 )
	end
end
