--[[
-- Message in a bottle
--
-- Synopsis:
--     A communication's relay satelite let's the player that he is not alone in the void
--]]

lang = naev.lang()
if lang == "es" then
    -- not translated atm
else -- default english 
    -- Text goes here. Please keep the section as organized as possible.
	warnText1="<WARNING: Hyperspace transit malfunction>"
	lostText1="<WARNING: Ship has not completed transit>"
	lostText2="<LOCATION: UNKNOWN>"
	theHails={"This is Doctor Talbot Mallory!", "Do you read me?", "<playership>, please respond.", "Is there anyone there?", "Is there anyone there?", "Hello?", "I don't even know if this equipment is still functioning....", "I'm the sole survivor of the Empire science vessel, '<shipname>'.", "Oh god, please respond...."}
end 


function create ()
	pauseHook=hook.timer(5000, "hailPlayer")
end

function hailPlayer()
	--pick two hair phrases
	repeat
		theIndex1=math.random(table.getn(theHails))
		theIndex2=math.random(table.getn(theHails))
	until theIndex1~=theIndex2
	aHail1=theHails[theIndex1]
	aHail2=theHails[theIndex2]
	theHail=(aHail1.."  "..aHail2)
	theHail=string.gsub(theHail, "<playership>", player.ship())
	theHail=string.gsub(theHail, "<shipname>", "Sharanya")
	--the satelite was made by a different event script, so we need to try and grab it's pilot ref
	allShips=pilot.get({faction.get(" ")})
	theSat=""
	numShips=table.getn(allShips)
	i=1
	repeat
		if allShips[i]:name()=="Communications Relay" then
			theSat=allShips[i]
		end
		i=i+1
	until i>numShips
	if theSat=="" then
		--the satelite does not exist
		endEvent()
	end
    theSat:broadcast(theHail)
	theDelay=rnd.rnd(10000,15000)
    repeatHailTimer = hook.timer(theDelay, "hailPlayer")
end

function endEvent()
	evt.finish(True)
end
