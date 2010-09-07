--[[
-- Message in a bottle
--
-- Synopsis:
--     A communication's relay satelite let's the player that he is not alone in the void
--]]

--variables saved
--   enHh_landed
--	 enHh_meeting

lang = naev.lang()
if lang == "es" then
    -- not translated atm
else -- default english 
    -- Text goes here. Please keep the section as organized as possible.
	warnText1="<WARNING: Hyperspace transit malfunction>"
	lostText1="<WARNING: Ship has not completed transit>"
	lostText2="<LOCATION: UNKNOWN>"
	theHails={"This is Doctor Talbot Mallory!", "Do you read me?", "<playership>, please respond.", "Is there anyone there?", "Is there anyone there?", "Hello?", "I don't even know if this equipment is still functioning....", "I'm the sole survivor of the Empire science vessel, 'Sharanya'.", "Oh god, please respond...."}
	thePlea="I wasn't even sure you were receiving me...this satellite wasn't designed for signaling ships. Please, my name is Doctor Talbot Mallory. I'm the last survivor of the Empire science vessel 'Sharanya'. You are the first ship that's come through since I was trapped here. Please, land on the planetoid at the center of the debris field. There's so much I have to tell you...."
	theLanding="The planetoid is barely dense enough to maintain a roughly spherical shape. As you approach, it's easy to see one of the older class of Science Vessels on the surface. Surrounding it are a series of other craft, all interconnected by docking clamps. A frantic flashing of lights signals where you should land and, setting down, a docking clamp extends from the hull of a scarred Gawain-class ship."
	theMeeting1="Within moments after the docking mechanisms completed their integrity tests, Dr. Mallory bursts into your ship. Her appearance is disheveled--her uniform worn and patched. When she talks, it is hesitant and halting, as though she is out of practice, but her body is never at rest; even when sitting, she rocks forward and backward. This is clearly someone that has undergone trauma."
	theMeeting2="Over the next hour and a half, you are able to piece together what happened to Doctor Mallory and where you are."
	theMeeting3="'Five years ago, the 'Sharanya' was returning to Sol from a…a deep space mission. We'd been gone for some time, checking up on an engineering project. As we made the final jump to the Sol system, something happened. The ship shuddered and dropped out of hyperspace and alarms began blaring. There were no stars and we were surrounded by debris that must have been in hyperspace with us. Other ships, pieces of ships, dust, rock. There was no way to avoid collisions. There was a massive amount of radiation hitting the shields. It was pandemonium. Rather than lose the entire ship, the captain rerouted the shielding to protect the engineering section and the bridge. The shields failed on most decks and most of the crew died...some were exposed to vacuum as the hull was punctured by debris after the shields were rerouted. Others died from radiation poisoning. We had no idea what happened.'"
	theMeeting4="You explain about the destruction of Sol five years ago and Doctor Talbot turns ashen. 'Sol and the core worlds are gone? What the hell could have.... That explains the debris and why our transit into Sol was cut short."
	theMeeting5="The 'Sharanya' was so damaged it could barely maneuver, and we'd lost most of the crew. We couldn't contact anyone else aboard other ships.... I guess they were killed. The rest of the crew and myself were able to gather some basic supplies from the other ships that came through with us, but with so few people we could never repair the ship. But then there was an engines overload. I was in the hanger bay, getting ready to launch a new type of automated construction device.... I guess the additional bay shielding saved me. The rest of the crew just...died. That was four years ago."
	theMeeting6="I was alone, and the ship was crippled. I managed to scavenge the parts to construct a sensor array to try and track the trajectories of the ships I could detect passing through hyperspace. It took...years...but I thought we identified a point where we could escape. By then, the Sharanya's hull integrity was so compromised I didn't think it would survive an attempt to leave. I don't know if physics is different here, but some of the dust, rocks and ships that got stuck with us had formed this planetoid. The rest of the debris formed a series of rings around it."	
	theMeeting7="I think this place we're in is a sort of side pocket in space-time. Imagine that the universe has a type of surface tension and that jump points are places where that surface energy is lower. That's what let's ships use hyperspace: the amount of energy needed to 'break the surface' is lower at those points. But if something catastrophic happened in Sol and all that energy were to hit a jump point, maybe a bubble was created. We're stuck in…in a drop of water on the surface of a pond. Under the right conditions, the surface tension of each keeps them from merging. We're in a little pocket, but I think I know how to get out...but I need your help."
	theMeeting8="I managed to slave the nav computers of some of the other ships to the Sharanya, and set them down on the planetoid, but I just don't have the equipment I need! It's all out there, in the rings. On board those derelict ships and drifting in the rings."
	theMeeting9="It's taken me years, but I know how to collapse this bubble we're in. The sensor array has let me track hundreds of thousands of ships passing through hyperspace. I know where this place intersects with the rest of the universe, and I think I know who to get us out. Eventually this bubble will cohere to the rest of the universe, but that could take decades! And who can say whether an already damaged ship could survive the transition? But I need your help. Your ship is still functional for now. Systems break down quickly here, but right now your ship is our best hope. I need you to gather parts and fuel from derelict ships and from the debris rings. Will you help? Together, we can leave this place."

end 

theSat=""

function create ()
	--make a communications sat
	--sat status:
	--    1==placed
	--    0==unplaced
	--   -1==killed
	test=var.peek("enTr_SatStatus")
	if test==1 and var.peek("enHh_landed")~=1 then
		--the satelite was made by a different event script, so we need to try and grab it's pilot ref
		allShips=pilot.get({faction.get(" ")})
		theSat=getSatID()
		print(theSat)
		if theSat==nil then
			--the satelite does not exist
			endEvent()
		else
			hailHook=hook.pilot(theSat, "hail", "hail")
			pauseHook=hook.timer(5000, "hailPlayer", theSat)
		end
	end
	hook.land("land")--when the player lands, there's no more need for hails
end

function getSatID()
	--the satelite was made by a different event script, so we need to try and grab it's pilot ref
	allShips=pilot.get({faction.get(" ")})
	theSat=nil
	numShips=table.getn(allShips)
	i=1
	if allShips[i]~=nil then
		repeat
			if allShips[i]:name()=="Communications Relay" then
				theSat=allShips[i]
				break
			end
			i=i+1
		until i>table.getn(allShips)
	end
	return theSat
end

function hailPlayer(theSat)
	if theSat:exists()==true then
		--pick two hail phrases
		repeat
			theIndex1=math.random(table.getn(theHails))
			theIndex2=math.random(table.getn(theHails))
		until theIndex1~=theIndex2
		aHail1=theHails[theIndex1]
		aHail2=theHails[theIndex2]
		theHail=(aHail1.."  "..aHail2)
		theHail=string.gsub(theHail, "<playership>", player.ship())
		theSat:broadcast(theHail)
		theDelay=rnd.rnd(10000,15000)
		repeatHailTimer = hook.timer(theDelay, "hailPlayer", theSat)
	end
end

function hail()
	if tk.yesno("Intersystem Relay Communication", "Communication Interlink Requested. Do you wish to proceed with real-time communication?") then		
		tk.msg("", thePlea)
		theSat=getSatID()
		theSat:changeAI("base")
		hook.rm(repeatHailTimer)
		hook.rm(pauseHook)
	end
end

function land()
	--maybe I should cut these up into single sentences in a list and show them in a random order. That would definitely be crazy.
	tk.msg("", theLanding)
	tk.msg("", theMeeting1)
	tk.msg("", theMeeting2)
	tk.msg("", theMeeting3)
	tk.msg("", theMeeting4)
	tk.msg("", theMeeting5)
	tk.msg("", theMeeting6)
	tk.msg("", theMeeting7)
	tk.msg("", theMeeting8)
	num, chosen=tk.choice("", theMeeting9, "Agree to help", "Politely decline")
	if num==1 then
		var.push("enHh_meeting", 1)--agreed to help
		--start the mission to help the doctor
		evt.misnStart("Super Looting1")
	else
		--var.push("enHh_meeting", 2)--did not agree to help
		--start the mission where the the player is allowed to go an explore
		tk.msg("", "Eventually the player will be allowed to flail about in space until they come back and agree to help")
		var.push("enHh_meeting", 1)--agreed to help
		--start the mission to help the doctor
		evt.misnStart("Super Looting1")
	end
	var.push("enHh_landed", 1)
	endEvent()
end

function endEvent()
	var.push("enHh_landed", 1)
	evt.finish(True)
end
