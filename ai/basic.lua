

function follow ()
	target = 1
	dir = face(target)
	dist = getdist(getpos(target))
	if dir < 10 and dist > 100 then
		accel()
	end
end


function goto ()

	v = gettarget()
	face(v)

	d = getdist(v)
	if d < minbrakedist()*1.05 then
		poptask()
	else
		accel(1)
	end
end


function control ()
	pushtask(0,"follow");
	--pushtask(0,"goto", createvect(1000,0));
end
