--[[
-- Engine Malfunction Event 2010.08.19-STH
--
-- Mimics a problem with the hyperdrive.
-- Synopsis:
--     Player starts receiving messages that time spent in hyperspace was longer than the ship's computer expeceted
--     If the player does not repair the system (visit a bar? an outfitter?) then during one jump the player ends up stuck in hyperspace
--
-- What would make things better:
--     Right now I can't seem to store how many warnings the player has gotten. It would be nice if there were 3 or more warnings before being trapped
--     It would be great if it were possible to retrieve the ships manufacturer. Then, the warning massages could say the player should visit a <fabricator> licensed mechanic, or something
--
-- Error notes: even though the following error show up, this event appears to work. The teleport seems to break things
--		Warning: [event_runLuaFunc] Event 'Engine Malfunction' -> 'removeMalfunction': [string "dat/events/neutral/engine_malfunction.lua"]:69: No hook target was set.
--		Warning: [hook_runEvent] Hook [timer] '108' -> 'msg1' failed
--		Warning: [event_runLuaFunc] Event 'Engine Malfunction' -> 'removeMalfunction': [string "dat/events/neutral/engine_malfunction.lua"]:60: bad argument #1 to 'rmOutfit' (string expected, got nil)
--		Warning: [hook_runEvent] Hook [	] '110' -> 'removeMalfunction' failed

--]]

lang = naev.lang()
if lang == "es" then
    -- not translated atm
else -- default english 
    -- Text goes here. Please keep the section as organized as possible.
	warnText1="<WARNING: Hyperspace transit malfunction>"
	warnText2="<WARNING: Transit time significantly different than calculated>"
	warnText3="<Resyncing with local system time servers....>"
	warnText4="<Seek assistance in repairs as soon as possible>"
end 

theOutfits={"Malfunction1", "Malfunction2", "Malfunction3"}
activeItem=""

function create ()
	--set the hooks--
	h1=hook.jumpout("grantMalfunction")
	h2=hook.jumpin("removeMalfunction")
end


function grantMalfunction()
	--player is going to jump out
	--give them the bum outfit
	myID=player.pilot()
	theIndex=math.random(table.getn(theOutfits))
	theOutfit=theOutfits[theIndex]
	myID:addOutfit(theOutfit)
end

function removeMalfunction()
	hook.rm(1)
	--player has jumped in
	--take away the outfit so the never see it
	myID=player.pilot()
	--there is no way to test to see if the player/pilot has an outfit
	--so just try and take it away and too bad about the error messages
	myID:rmOutfit(theOutfit)
	if system.cur():name()~="ERROR: Telemetry Not Found" then
		theRandom=rnd.rnd()
		if theRandom>0.6 then	
			--40% chance of going to special system
			--I wanted this to be triggered after X# warnings, but can't seem to save the count. need to think about how more
			player.teleport(system.get("ERROR: Telemetry Not Found"))--go to hell. do not pass go.
		end
	end
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
	h5=hook.timer(1000, "msg3")
end

function msg3()
	hook.rm(h5)
	player.msg(warnText3)
	h6=hook.timer(5000, "msg4")
end

function msg4()
	hook.rm(h6)
	player.msg(warnText4)
end

function endMalfunction()
	evt.finish(True)
end

