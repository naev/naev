--[[
-- Engine Malfunction Event 2010.08.25-STH
--
-- Mimics a recalibration of the hyperdrive.
-- Synopsis:
--     Presence of the outfit undoes some of the hyperspace delays introduced by the engine malfunction events
--]]
theOutfit="Engine Calibration"

lang = naev.lang()
if lang == "es" then
    -- not translated atm
else -- default english 
    -- Text goes here. Please keep the section as organized as possible.
	theText1="<UPDATE: Beginning engine calibration>"
	theText2="<UPDATE: Testing engine for problems....>"
	theText3={"<UPDATE: No error detected>", "<UPDATE: Error detected in: "}
	--this can be added to later
	possibleErrors={"control rods", "plasma injector coil", "limiter unit"}
	theText4="<UPDATE: Attempting recalibration....>"
	theText5={"<UPDATE: Recalibration complete>", "<WARNING: Calibration failed. Repairs necessary>"}
end 

function create ()
	if player:pilot():hasOutfit("Engine Calibration")==1 then
		player.msg(theText1)
		h1=hook.timer(1000, "msg2")
	end		
end


function msg2()
	hook.rm(h1)
	player.msg(theText2)
	h2=hook.timer(3000, "msg3")
end

function msg3()
	hook.rm(h2)
	n = var.peek( "engine_malfunctionWarning" ) -- Get the value
	player:pilot():rmOutfit("Engine Calibration")
	if n==nil then
		theText=theText3[1]
		player.msg(theText)
		endEvent()
	else
		theText=theText3[2]
		i=math.random(table.getn(possibleErrors))
		theText=theText..possibleErrors[i]..">"
		player.msg(theText)
		h3=hook.timer(1000, "msg4")	
	end
end

function msg4()
	hook.rm(h3)
	player.msg(theText4)
	h4=hook.timer(3000, "msg5")
end


function msg5()
	hook.rm(h4)
	n = var.peek( "engine_malfunctionWarning" ) -- Get the value
	--very rarely, calibration will failed
	r=rnd.rnd()
	if r>0.95 then
		--this is a super fail. makes problems worse. 5% chance
		r2=rnd.rnd(1,n)
		n=n+r2
		var.push("engine_malfunctionWarning", n)
		player.msg(theText5[2])
	elseif r>0.8 then
		--this is just a fail. 20% chance
		player.msg(theText5[2])
	else
		--success! calibration has helped
		r2=rnd.rnd(1,n)
		n=n-r2
		if n<0 then
			n=0 --just make sure
		end
		var.push("engine_malfunctionWarning", n)
		player.msg(theText5[1])
	end
	endEvent()
end

function endEvent()
	hook.rm(h1)
	hook.rm(h2)
	hook.rm(h3)
	hook.rm(h4)
	evt.finish(True)
end

