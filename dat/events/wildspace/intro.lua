--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Welcome to Wild Space">
 <location>enter</location>
 <unique />
 <chance>100</chance>
 <system>Nirtos</system>
</event>
--]]
local vn = require "vn"
local vni = require "vnimage"
local der = require "common.derelict"
local ws = require "common.wildspace"
local fmt = require "format"
local fcts = require "factions"

local mainspb, mainsys = spob.getS("Hypergate Protera")

local derelict
function create ()
   -- Create the random derelict
   derelict = pilot.add( "Koala", "Derelict", vec2.newP( system.cur():radius()*0.3, rnd.angle() ), p_("ship", "Derelict"), {ai="dummy", naked=true})
   derelict:setDisable()
   derelict:intrinsicSet( "ew_hide", 300 ) -- Much more visible
   derelict:intrinsicSet( "nebu_absorb", 100 ) -- Immune to nebula

   -- Don't let the player land untiil the hail, we could do a diff, but it's more complex and likely not worth it
   player.landAllow( false, _("You do not see any place to land here.") )

   -- Set up our hooks
   hook.timer( 25, "msg" )
   hook.enter( "enter" )
   hook.land( "land" )
end

function msg ()
   -- Shouldn't be dead, but just in case
   if not derelict:exists() then
      return
   end
   derelict:setVisplayer()
   derelict:hailPlayer()
   hook.pilot(derelict, "hail", "hail")
end

function hail ()
   vn.clear()
   vn.scene()
   local c = vn.newCharacter( vni.soundonly( p_("character","C") ) )
   vn.transition()

   vn.na(_([[You respond to the mysterious hail from a derelict, and strangely enough are able to open a sound-only channel.]]))
   c(_([["You aren't one of them, are you?"]]))
   vn.menu{
      {_([["Who are YOU?"]]), "cont01_you"},
      {_([["Them?"]]), "cont01"},
   }

   vn.label("cont01_you")
   c(_([[You hear a loud cough.
"Not one of them at least."]]))
   vn.label("cont01")
   c(_([["What are you doing out there? You'll get shredded out there! Come land over here."]]))
   vn.na(_([[You hear some coughing as the connection is cut. It looks like they sent you some coordinates though.]]))

   vn.run()

   -- Not a mission so use a system marker
   system.markerAdd( mainspb:pos(), _("Land here") )
   player.landAllow()
end

function land ()
   local accept = false
   vn.clear()
   vn.scene()
   vn.transition()

   vn.sfx( der.sfx.board )
   vn.na(_([[You slowly approach the location you were given with your ship avoiding the thick structural debris. After a long time of searching, you eventually find a crusty old docking port among the sprawling wreck that seemingly is in working condition. Weapon drawn and in full EVA gear you prepare to go into the rubbish.]]))
   vn.na(_([[You work your way through the crushed structure, and eventually find your way to what seems to be a working airlock. After fiddling with the controls, you are able to make your way into it and to what seemingly seems to be makeshift and surprisingly working area of the wreck. You make your way forward and eventually find yourself in what seems to be some sort of dirty cantina. You can see some ancient spoiled food and drink on the tables.]]))

   vn.scene()
   local c = vn.newCharacter( vni.soundonly( p_("character","C") ) )
   vn.transition()
   c(_([[Suddenly a loudspeaker screeches on, and you hear some coughing followed by a familiar voice.
"I was right! You aren't one of them. How the hell did you make it here? I thought I was the only normal one left."]]))
   vn.menu{
      {_([["What is going on here?"]]),"cont01_what"},
      {_([["Who are them?"]]),"cont01_who"},
   }

   vn.label("cont01_what")
   c(_([["I was hoping you could tell me! One moment I'm jumping through hyperspace, then when I drop out it's a damn mess! Missing planets, the few remaining are completely damaged, death and destruction everywhere!"]]))
   vn.jump("cont01")

   vn.label("cont01_who")
   c(_([["You've seen them, haven't you? All those ships out there, attacking everything on sight. They even kill each other all the time! It's a dog-eat-dog world out there..."]]))
   vn.jump("cont01")

   vn.label("cont01")
   c(_([[They cough quite severely, before abruptly changing tone.
"DAMNIT! I'LL AVENGE THEM ALL!"
You hear a dull thud echo through the hull.]]))
   c(_([[They recompose before continuing in a small voice.
"Sorry about that. Something happened out there, something big, right? Is there still a universe out there?"]]))
   vn.na(_([[You explain to the best of your abilities the Incident and all the things that happen a few decacycles ago. The voice seems to go quiet, attentively listening to all your words. Eventually you finish and the room falls into silence.]]))
   c(_([[After what seems like an eternity, a wracking cough plays over the speakers.
"I always thought this couldn't be the end of it all. It's what has kept me going so long."]]))
   c(_([[They cough once again.
"But it seems like not long enough..."]]))
   c(_([["I know we just met, but I want to ask a favour of you. You see, it wasn't this bad, as it is now."]]))
   c(_([[They give a small cough.
"After the... the Incident, me and my crewmates, we thought we could pull through. We created a colony with the survivors on husk of what was left of the once glorious Protera. However, the worst was yet to come."]]))
   c(_([["At first we thought it was some sort of pathogen, maybe due to all the debris, however, even with careful quarantine and elimination, it still spread. People would... change."]]))
   c(_([["It started subtle, violent outbreaks, slight twitching, aversion to bathing, then it got worse. Eventually, they would lash out, attack everyone and everything in some sort of primal irrational rage. Having lost all their remaining humanity, we called the brutes the Lost."]]))
   c(_([["Paranoia set in and many were purged, but it didn't stop. Eventually, the survivors split into separate enclaves, all fortified and wary against strangers. It never stopped progressing though..."]]))
   c(_([["Almost everyone I knew was lost, and in the end, me and my roommate Ben, we tried to get out of there, the only..."
They pause to give a wracking cough, it takes them a while before they can continue.]]))
   c(_([["I'm rambling now, just like an old man... It wasn't supposed to end like this..."]]))
   c(_([["Ah yes, the favour. Ben, he left some important items behind. I would like you to bring them to me."]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Maybe Later]]), "decline"},
   }

   vn.label("decline")
   c(_([["I will be waiting, if you change your mind..."
To the tune of coughing the speaker goes silent.]]))
   vn.done()

   vn.label("accept")
   vn.func( function () accept = true end )
   c(_([[They cough.
"Here, take the coordinates. Be careful."]]))
   vn.run()

   fcts.setKnown( faction.get("Lost"), true )

   ws.log(fmt.f(_([[You found a survivor in the {sys} system that led you to a ruined hypergate. You exchanged information with them, and they told you about the Lost that inhabit Wild Space. They seem to have something they want you to do.]]),
      {sys=mainsys}))

   -- Still finish event, but now the mission will have to be started "normally"
   if not accept then
      evt.finish(true)
      return
   end

   -- Will start the mission, and use variable to skip accepting stage
   var.push("wildspace_start_misn", true)
   naev.missionStart("Old Friends at Protera Husk")

   -- Done here
   evt.finish(true)
end

function enter ()
   -- Clean up and restart if player changes system
   if system.cur() ~= mainsys then
      evt.finish(false)
   end
end
