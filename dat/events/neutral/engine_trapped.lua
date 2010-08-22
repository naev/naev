--[[
-- Stuck in hyperspace
--
-- The pilot has gotten trapped in hyperspace.
-- Synopsis:
--     Player's hyperspace jump ends up in an unexpected system. There's no way to jump out
--]]

lang = naev.lang()
if lang == "es" then
    -- not translated atm
else -- default english 
    -- Text goes here. Please keep the section as organized as possible.
	warnText1="<WARNING: Hyperspace transit malfunction>"
	lostText1="<WARNING: Ship has not completed transit>"
	lostText2="<LOCATION: UNKNOWN>"
end 

function create ()
	--teleporting in seems to result in entering the system twice, if you hijack a hyperspace
	--really, this is getting solo pilots from fleet.xml
	--too bad there isn't a way to make this list in an automatic way. 
	theShips={"Independent Schroedinger", "Independent Hyena", "Independent Gawain", "Goddard Goddard", "Goddard Lancelot", "Trader Llama", "Trader Koala", "Trader Rhino", "Trader Mule", "Trader Quicksilver", "Proteron Kahan", "Proteron Archimedes", "Proteron Derivative", "FLF Pacifier", "FLF Vendetta", "Pirate Shark", "Pirate Admonisher", "Pirate Ancestor", "Pirate Kestrel", "Collective Drone", "Sirius Fidelity", "Sirius Preacher", "Sirius Divinity", "Sirius Dogma", "Dvaered Ancestor", "Empire Hawking", "Empire Peacemaker", "Seiryuu", "Shadowvigil Diplomat"}
	--it'd be nice to have exotic ships not seen in game. These would be antique ships and probes
	--maybe also alien?
	i=1	
	repeat
		theIndex=math.random(table.getn(theShips))
		theShip=theShips[theIndex]
		-- Create the derelict.
		angle = rnd.rnd() * 2 * math.pi
		dist  = rnd.rnd(0, 4000)
		pos   = vec2.new( dist * math.cos(angle), dist * math.sin(angle) )
		p     = pilot.add(theShip, "base", pos)[1]
		p:setFaction(" ")
		p:disable()
		i=i+1
	until i>50--make 50 derelicts
	--make a communications sat
	theSat=pilot.add("Communications Relay", "base", vec2.new(0, 3000))[1]
	--set up the background for this odd space
	bkg.clear()
	--nebula32 makes it hard to read white text
	--local theNebula="gfx/bkg/nebula32.png"
	--local theImg=tex.open(theNebula)
	--bkg.image(theImg, 0.0, 0.0, 0.0, 1.9 )
	--now display the oh god now messages
	player.msg(warnText1)	
	player.msg(lostText1)
	player.msg(lostText2)
	--evt.finish(True)--calling evt.finish(
	shipHyperIn()
end

function shipHyperIn()
	theIndex=math.random(table.getn(theShips))
	theShip=theShips[theIndex]
	angle = rnd.rnd() * 2 * math.pi
	dist  = rnd.rnd(0, 3000)
	pos   = vec2.new( dist * math.cos(angle), dist * math.sin(angle) )
	p     = pilot.add(theShip, "base", pos)[1]
	p:control()
	if theSat:exists() then
		theSat:control()
		theSat:face(p)
	end
	p:face(vec2.new(10000,10000))
	p:setVel(vec2.new(100000,100000))
	hypeOutTimer=hook.timer(500, "shipHyperOut")
end

function shipHyperOut()
	if p:exists() then 
		p:rm()
		if theSat:exists() then
			theSat:control()
			theSat:face(player.pilot())
		end
	end
	p=""
	timeToNext=rnd.rnd(1, 1000)
	hypeInTimer = hook.timer(timeToNext, "shipHyperIn")
end
		

function endEvent()
	evt.finish(True)
end
