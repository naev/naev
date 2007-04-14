

function follow ()
	face(1, 1)
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
