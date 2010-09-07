--[[
-- Stuck in hyperspace
--
-- The pilot has gotten trapped in hyperspace.
-- Synopsis:
--     Player's hyperspace jump ends up in an unexpected system. There's no way to jump out
--]]

--variables saved:
--   enTr_MsgStatus
--   enTr_SatStatus
--   enTr_ClutterStatus
--   enTr_NebulaStatus

include("scripts/string_utils.lua")
include("dat/events/neutral/derelict.lua")--take advantage of boarding messages that already exist




theSat=nil
allShips={"Independent Schroedinger", "Independent Hyena", "Independent Gawain", "Goddard Goddard", "Goddard Lancelot", "Trader Llama", "Trader Koala", "Trader Rhino", "Trader Mule", "Trader Quicksilver", "Proteron Kahan", "Proteron Archimedes", "Proteron Derivative", "FLF Pacifier", "FLF Vendetta", "Pirate Shark", "Pirate Admonisher", "Pirate Ancestor", "Pirate Kestrel", "Collective Drone", "Sirius Fidelity", "Sirius Preacher", "Sirius Divinity", "Sirius Dogma", "Dvaered Ancestor", "Empire Hawking", "Empire Peacemaker", "Seiryuu", "Shadowvigil Diplomat"}
--debris types 0, 1 and 2 are large enough to be seen on their own. Include the generic cargo image
allDebris={"Debris0", "Debris1", "Debris2", "Debris6"}
--maybe make a lua file listing all outfits and throw it into scripts/?
goodItems={"Battery", "Battery II", "Battery III", "Reactor Class I", "Reactor Class II", "Reactor Class III", "Shield Capacitor", "Shield Capacitor II", "Shield Capacitor III", "Shield Capacitor IV", "Shield Booster", "Droid Repair Crew", "Engine Reroute", "Auxiliary Processing Unit I", "Auxiliary Processing Unit II", "Auxiliary Processing Unit III", "Fuel Pod", "Improved Power Regulator", "Improved Refrigeration Cycle"}


lang = naev.lang()
if lang == "es" then
    -- not translated atm
else -- default english 
    -- Text goes here. Please keep the section as organized as possible.
	warnText1="<WARNING: Hyperspace transit malfunction>"
	lostText1="<WARNING: Ship has not completed transit>"
	lostText2="<WARNING: Location unknown>"
	btext={}
	btext[1] = [[As soon as you affix your boarding clamp to the derelect ship, it triggers an explosion, severely damaging your ship!]]
	btext[2] = [[The docking clamp cycles through integrety tests and an electrical short on the derelect triggers an explosion!]]
	btext[3] = [[You barely nudge the derelect and it explodes!]]

end 
	
function create ()
	--I'm pretty sure that hijacking a hyperspace jump with a teleport is seen as entering a system twice.
	clearNebula()
	placeSat()
	placeClutter()
	shipHyperIn()
	displayMsg()
	enTrLand=hook.land("onLand")
end

function onLand()
	var.pop("enTr_SatStatus")
	var.pop("enTr_ClutterStatus")
	var.pop("enTr_NebulaStatus")
	hook.rm(enTrLand)
	--this would start a mission
end



function clearNebula()
	--nebula status:
	--  1==cleared
	--  0==uncleared
	test=var.peek("enTr_NebulaStatus")
	if test==0 or test==nil then
		bkg.clear()
		var.push("enTr_NebulaStatus",1)
	end
end

function placeSat()
	--make a communications sat
	--sat status:
	--   1==placed
	--   0==unplaced
	--   -1==killed
	test=var.peek("enTr_SatStatus")
	if test==0 or test==nil then
		theSat=pilot.add("Communications Relay", "dummy", vec2.new(0, 3000))[1]
		--faction should be " "
		hook.pilot(theSat, "death", "killSat")
		var.push("enTr_SatStatus",1)
	end
end

function killSat()
	var.push("enTr_SatStatus", -1)
end

function placeClutter()
	--make clutter
	--clutter status:
	--  1==placed
	--  0==unplaced
	test=var.peek("enTr_ClutterStatus")
	if test==0 or test==nil then
		debrisArray=var.peek("enTr_debrisArray")
		i=1
		if debrisArray==nil then
			maxRings=2
			minDist=200
			theWidth=200
			shipChance=0.01
			--shipChance=1.0
			theGap=200
			ringNum=1
			debrisArray=""
			maxDist=minDist+theWidth
			repeat
				repeat
					r=rnd.rnd()
					if r>shipChance then
						theIndex=math.random(table.getn(allDebris))
						theDebris=allDebris[theIndex]
					else
						theIndex=math.random(table.getn(allShips))
						theDebris=allShips[theIndex]
					end
					angle = rnd.rnd() * 2.0 * math.pi
					dist = rnd.rnd(minDist, maxDist)
					x=dist * math.sin(angle)
					y=(dist-(dist/2)) * math.cos(angle+1)--  Quick and dirty way to creat the debris as an ellipsoid ring
					pos = vec2.new(x,y)			
					p = pilot.add(theDebris, "base", pos)[1]
					p:setFaction(" ")
					p:setCrew(0)
					p:disable()
					--hook.pilot(p, "board", "board", p)
					p:rename("Debris")
					debrisArray=debrisArray..","..theDebris..","..x..","..y
					i=i+1
				until i>100--make 100 debris objects
				minDist=maxDist+theGap
				maxDist=minDist+theWidth
				shipChance=shipChance+0.2
				ringNum=ringNum+1
				i=1
			until ringNum>maxRings

			--save where each ship is so it can be recreated
			var.push("enTr_debrisArray", tostring(debrisArray))
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
				p:setCrew(0)
				p:disable()
				--hook.pilot(p, "board", "board", p)
				p:rename("Debris")
				i=i+3
			until i>=table.getn(debrisArray)
		end
		var.push("enTr_ClutterStatus",1)
	end
end

function displayMsg()
	--msg status:
	--  1==displayed
	--  0==uncleared
	test=var.peek("enTr_MsgStatus")
	if test==0 or test==nil then
		--now display the oh god now messages
		player.msg(warnText1)	
		player.msg(lostText1)
		player.msg(lostText2)
		var.push("enTr_MsgStatus", 1)
	end
end

function shipHyperIn()
	--simulates ships passing through hyperspace very quickly
	theIndex=math.random(table.getn(allShips))
	theShip=allShips[theIndex]
	angle = rnd.rnd() * 2 * math.pi
	dist  = rnd.rnd(0, 3000)
	pos   = vec2.new( dist * math.cos(angle), dist * math.sin(angle) )
	p     = pilot.add(theShip, "base", pos)[1]
	p:control()
	--if theSat:exists() then
	if theSat~=nil then
		if pilot.exists(theSat) then
			theSat:control()
			theSat:face(p)
		end
	end
	p:face(vec2.new(10000,10000))
	p:setVel(vec2.new(100000,100000))
	hypeOutTimer=hook.timer(500, "shipHyperOut", p)
end

function shipHyperOut(p)
	if p:exists() then 
		p:rm()
		if theSat~=nil then
			if pilot.exists(theSat) then
				theSat:control()
				theSat:face(player.pilot())
			end
		end
	end
	timeToNext=rnd.rnd(1000, 5000)
	hypeInTimer = hook.timer(timeToNext, "shipHyperIn")
end


function board(thePilot)
	if thePilot:ship():baseType()~="Debris" then
		--this is a derelict ship
		r=rnd.rnd()
		player.unboard()
		if r>=0.75 then
			--this is a straight up good event
			--fuel? outfit? credits?
			theWeaponSlots,theUtilitySlots,theStructureSlots=thePilot:ship():slots()
			i=0
			whatGot=""
			repeat 
				theItem=goodItems[rnd.rnd(1,#goodItems)]
				print(theItem)
				player.addOutfit(theItem)
				whatGot=whatGot..", "..theItem
				i=i+1
			until i>=theUtilitySlots
			tk.msg("", "You'd get something good here")
			tk.msg("", whatGot)
		elseif r>=0.50 then
			--this is a bad event
			--boom
			i=rnd.rnd(1,#btext)
			tk.msg("", btext[i])
			thePilot:setHealth(0,0)
			modHealth()--don't want to kill the pilot
		elseif r>=0.25 then
			--this is a mixed event
			--fuel? outfit? credits? then boom
			tk.msg("", "You'd get something good here")
			tk.msg("", "But oh no! Boom!")
			thePilot:setHealth(0,0)
			modHealth()--don't want to kill the pilot
		else
			--this is a neutral event
			i=rnd.rnd(1,#ntext)
			tk.msg("", ntext[i])
		end
		--maybe pilot logs to show how crazy Dr. Talbot is?
			end
end

function modHealth()
	myShield,myArmor,dis=player:pilot():health()
	newShield=rnd.rnd(0,myShield)
	newArmor=rnd.rnd(30,myArmor)
	if newArmor<=30 then
		newArmor=30
	end
	player.pilot():setHealth(newArmor, newShield)
end

function neturalEvent()
	i=rnd.rnd(1, #ntext)
	theText=ntext[i]
	tk.msg("",theText)
end

function endEvent()
	var.pop("enTr_SatStatus")
	var.pop("enTr_ClutterStatus")
	var.pop("enTr_NebulaStatus")
	evt.finish(True)
end
