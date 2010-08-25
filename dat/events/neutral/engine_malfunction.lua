--[[
-- Engine Malfunction Event 2010.08.19-STH
--    Reworked 2010.08.23
--
-- Mimics a problem with the hyperdrive.
-- Synopsis:
--     Player starts receiving messages that time spent in hyperspace was longer than the ship's computer expeceted
--     If the player does not repair the system (visit a bar? an outfitter?) then during one jump the player ends up stuck in hyperspace
--]]

lang = naev.lang()
if lang == "es" then
    -- not translated atm
else -- default english 
    -- Text goes here. Please keep the section as organized as possible.
	warnText1="<WARNING: Hyperspace transit malfunction>"
	warnText2="<Resyncing with local system time servers....>"
	warnText3="<WARNING: Transit time significantly different than calculated>"
	warnText4="<WARNING: Estimated transit time: 1 Actual transit time: Z>"
	warnText5="<Seek assistance in repairs as soon as possible"
end 

function create ()
	--set the hooks--
	h1=hook.jumpout("onJumpout")
	h2=hook.jumpin("onJumpin")
end


function onJumpout()
	--has the player been warned before?
	n = var.peek( "engine_malfunctionWarning" ) -- Get the value
	if n==nil then 
		n=0 
	end
	--r=rnd.rnd(n,10)
	r=10--for debugging
	if r>7 then
		n = n+1
		var.push("engine_malfunctionWarning", n)
		--before jump, get the time and save it
		var.push("engine_malfunctionTime", time.get())
		--get the ship's default (average) jump time in STU
		t=player:jumpTime()
		i=math.pow(n,0.5)*5.0	--borrow a similar idea to how jump time is calculated by the game engine
		--now alter the apparent jumptime
		time.inc(i)
	else
		hook.rm(h1)
		hook.rm(h2)
	end
end

function onJumpin()
	hook.rm(h1)
	n = var.peek( "engine_malfunctionWarning" ) -- Get the value
	--if n>3 then
	--	if system.cur():name()~="ERROR: Telemetry Not Found" then
	--		theRandom=rnd.rnd()
	--		if theRandom>0.6 then	
	--			--40% chance of going to special system
	--			--I wanted this to be triggered after X# warnings, but can't seem to save the count. need to think about how more
	--			player.teleport(system.get("ERROR: Telemetry Not Found"))--go to hell. do not pass go.
	--		end
	--	end
	--end
	h3=hook.timer(1000, "msg1")
end


function msg1()
	hook.rm(h2)
	hook.rm(h3)
	if system.cur():name()=="ERROR: Telemetry Not Found" then
		endMalfunction()
	else
		player.msg(warnText1)	
		h4=hook.timer(2000, "msg2")
	end
end

function msg2()
	hook.rm(h4)
	player.msg(warnText2)
	h5=hook.timer(3000, "msg3")
end

function msg3()
	hook.rm(h5)
	player.msg(warnText3)
	h6=hook.timer(1000, "msg4")	
end

function msg4()
	hook.rm(h6)
	--the estimated jump time. 1 decimal. In STU
	estT=player:jumpTime()
	estT=math.floor(estT*10^1+0.5)/10^1
	--the actual jump time. 1 decimal. In STU
	prevT=var.peek("engine_malfunctionTime")
	presT=time.get()
	actT=presT-prevT
	actT=math.floor(actT*10^1+0.5)/10^1
	actT=time.str(actT)
	warnText4=string.gsub(warnText4, "1", estT)
	warnText4=string.gsub(warnText4, "Z", actT)
	--need to make time.str() drop leading zeros
	player.msg(warnText4)
	h7=hook.timer(3000, "msg5")
end


function msg5()
	hook.rm(h7)
	r=rnd.rnd()
	if r>0.5 then
		theFab=player:pilot():ship():fabricator()
		--could come up with all sorts of cheesy promotional phrases for here
		warnText5=warnText5..", and remember to choose only the best "..theFab.." parts>"
	else
		warnText5=warnText5..">"
	end
	player.msg(warnText5)
end

function endMalfunction()
	var.pop("engine_malfunctionTime")
	var.pop("engine_malfunctionWarning")
	evt.finish(True)
end

