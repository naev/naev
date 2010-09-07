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
	warnText2="<Resyncing with local system time servers....>"--this should probably only appear in settled systems
	warnText3="<WARNING: Transit time significantly different than calculated>"
	warnText4="<WARNING: Estimated transit time: 1 Actual transit time: Z>"
	warnText5="<Seek assistance in repairs as soon as possible"

end 

malfunctionHappened=0

function create ()
	--set the hooks--
	enMalJumpOut=hook.jumpout("onJumpout")
	enMalJumpIn=hook.jumpin("onJumpin")
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
		malfunctionHappened=1
	end
	hook.rm(enMalJumpOut)
end

function onJumpin()
	if malfunctionHappened==1 then
		malfunctionHappened=0
		aMalfunction()
	end
end

function aMalfunction()
	n = var.peek( "engine_malfunctionWarning" ) -- Get the value
	if system.cur():name()~="ERROR: Telemetry Not Found" then
		if n>3 then
			theRandom=rnd.rnd()
			if theRandom>0.6 then	
				--40% chance of going to special system
				--I wanted this to be triggered after X# warnings, but can't seem to save the count. need to think about how more
				player.teleport(system.get("ERROR: Telemetry Not Found"))--go to hell. do not pass go.
				var.pop("engine_malfunctionTime")
				var.pop("engine_malfunctionWarning")
				hook.rm(enMalJumpOut)
				hook.rm(enMalJumpIn)
				--endEvent()
			end
		else
			--gug. time delays on messages are such a pain in tha butt
			player.msg(warnText1)
			--player.msg(warnText2)
			--the estimated jump time. 1 decimal. In STU
			estT=estJumpTime()
			--the actual jump time. 1 decimal. In STU
			actT=actJumpTime()
			--do some replacements
			warnText4=string.gsub(warnText4, "1", estT)
			warnText4=string.gsub(warnText4, "Z", actT)
			--player.msg(warnText3)
			player.msg(warnText4)
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
	end
	hook.rm(enMalJumpIn)
end

function actJumpTime()
	--the actual jump time. 1 decimal. In STU
	prevT=var.peek("engine_malfunctionTime")
	presT=time.get()
	actT=presT-prevT
	actT=math.floor(actT*10^1+0.5)/10^1
	actT=time.str(actT)
	return actT
end


function estJumpTime()
	--the estimated jump time. 1 decimal. In STU
	estT=player:jumpTime()
	estT=math.floor(estT*10^1+0.5)/10^1
	return estT
end

