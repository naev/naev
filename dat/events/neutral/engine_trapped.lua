--[[
-- Stuck in hyperspace
--
-- The pilot has gotten trapped in hyperspace.
-- Synopsis:
--     Player's hyperspace jump ends up in an unexpected system. There's no way to jump out
--]]

include("scripts/string_utils.lua")

allShips={"Independent Schroedinger", "Independent Hyena", "Independent Gawain", "Goddard Goddard", "Goddard Lancelot", "Trader Llama", "Trader Koala", "Trader Rhino", "Trader Mule", "Trader Quicksilver", "Proteron Kahan", "Proteron Archimedes", "Proteron Derivative", "FLF Pacifier", "FLF Vendetta", "Pirate Shark", "Pirate Admonisher", "Pirate Ancestor", "Pirate Kestrel", "Collective Drone", "Sirius Fidelity", "Sirius Preacher", "Sirius Divinity", "Sirius Dogma", "Dvaered Ancestor", "Empire Hawking", "Empire Peacemaker", "Seiryuu", "Shadowvigil Diplomat"}
--debris types 0, 1 and 2 are large enough to be seen on their own.
allDebris={"Debris0", "Debris1", "Debris2"}

lang = naev.lang()
if lang == "es" then
    -- not translated atm
else -- default english 
    -- Text goes here. Please keep the section as organized as possible.
	warnText1="<WARNING: Hyperspace transit malfunction>"
	lostText1="<WARNING: Ship has not completed transit>"
	lostText2="<WARNING: Location unknown>"
end 
	
function create ()
	clearNebula()
	enTr1=hook.jumpin("displayMsg")
	enTr2=hook.land("onLand")
end

function onLand()
	hook.rm(enTr1)
	hook.rm(enTr2)
end



function clearNebula()
	--set up the background for this odd space
	bkg.clear()
	--nebula32 makes it hard to read white text
	--local theNebula="gfx/bkg/nebula32.png"
	--local theImg=tex.open(theNebula)
	--bkg.image(theImg, 0.0, 0.0, 0.0, 1.9 )
	placeSat()
end

function placeSat()
	--make a communications sat
	theSat=pilot.add("Communications Relay", "dummy", vec2.new(0, 3000))[1]
	placeClutter()
end

function placeClutter()
	i=1
	debrisArray=var.peek("engine_trapped_debrisArray")
	if debrisArray==nil then
		--debrisArray={}
		debrisArray=""
		repeat
			--50% chance of ship vs random debris
			r=rnd.rnd()
			if r>0.1 then
				theIndex=math.random(table.getn(allDebris))
				theDebris=allDebris[theIndex]
			else
				theIndex=math.random(table.getn(allShips))
				theDebris=allShips[theIndex]
			end
			-- Create the derelict.
			angle = rnd.rnd() * 2.0 * math.pi
			dist = rnd.rnd(50,500)
			x=dist * math.sin(angle)
			y=dist * math.cos(angle)
			pos = vec2.new(x,y)			
			p = pilot.add(theDebris, "base", pos)[1]
			p:setFaction(" ")
			p:disable()
			p:rename("Debris")
			--too bad you can't save tables usig var.push
			--debrisArray[i]={theDebris,x,y}
			debrisArray=debrisArray..","..theDebris..","..x..","..y
			i=i+1
		until i>100--make 100 debris objects
		--save where each ship is so it can be recreated
		var.push("engine_trapped_debrisArray", tostring(debrisArray))
	else
		--recreate where each ship is
		debrisArray=split(debrisArray, ",")
		repeat
			theDebris=debrisArray[i]
			x=debrisArray[i+1]
			y=debrisArray[i+2]
			pos = vec2.new(x,y)
			p = pilot.add(theDebris, "base", pos)[1]
			p:setFaction(" ")
			p:disable()
			p:rename("Debris")
			i=i+3
		until i>=table.getn(debrisArray)
	end
	--shipHyperIn()
end

function displayMsg()
	clearNebula()
	--now display the oh god now messages
	player.msg(warnText1)	
	player.msg(lostText1)
	player.msg(lostText2)
end

function shipHyperIn()
	theIndex=math.random(table.getn(allShips))
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
		--this fails sometimes. Dunno why
		--Warning: [event_runLuaFunc] Event 'Lost in Hyperspace' -> 'shipHyperOut': [string "dat/events/neutral/engine_trapped.lua"]:72: attempt to call method 'exists' (a nil value)
		--Warning: [hook_runEvent] Hook [timer] '741' -> 'shipHyperOut' failed
		p:rm()
		if theSat:exists() then
			theSat:control()
			theSat:face(player.pilot())
		end
	end
	p=""
	timeToNext=rnd.rnd(10, 1000)
	hypeInTimer = hook.timer(timeToNext, "shipHyperIn")
end
		

function endEvent()
	evt.finish(True)
end
