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

local mainspb, mainsys = spob.getS("Hypergate Protera")

local derelict
function create ()
   evt.finish(false)

   -- Create the random derelict
   derelict = pilot.add( "Koala", "Derelict", vec2.newP( system.cur():radius()*0.3, rnd.angle() ), p_("ship", "Derelict"), {ai="dummy", naked=true})
   derelict:disable()
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
   local c = vn.newCharacter( vni.soundonly( _("character","C") ) )
   vn.transition()

   vn.na(_([[You respond to the mysterious hail from a derelict, and strangely enough are able to open a sound-only channel.]]))
   c(_([["You aren't one of them, are you?"]]))
   vn.menu{
      {_("Who are YOU?"), "cont01_you"},
      {_("Them?"), "cont01"},
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
   vn.clear()
   vn.scene()
   vn.transition()

   vn.sfx( der.sfx.board )
   vn.na(_([[You slowly approach the location you were given with your ship avoiding the thick structural debris. After a long time of searching, you eventually find a crusty old docking port among the sprawling wreck that seemingly is in working condition. Weapon drawn and in full EVA gear you prepare to go into the rubbish.]]))
   vn.na(_([[You work your way through the crushed structure, and eventually find your way to what seems to be a working airlock. After fiddling with the controls, you are able to make your way into it and to what seemingly seems to be makeshift and surprisingly working area of the wreck. You make your way forward and eventually find yourself in what seems to be some sort of dirty cantina. You can see some ancient spoiled food and drink on the tables.]]))

   vn.scene()
   local c = vn.newCharacter( vni.soundonly( _("character","C") ) )
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
   c(_([[They cough before continuing.
"Something happened out there, something big. Is there still a universe out there?"]]))
   vn.na(_([[You explain to the best of your abilities the Incident and all the things that happen a few decacycles ago. The voice seems to go quiet, attentively listening to all your words. Eventually you finish and the room falls into silence.]]))
   c(_([[After what seems like an eternity, a wracking cough plays over the speakers.
"I always thought this couldn't be the end of it all. It's what has kept me going so long."]]))
   c(_([[They cough once again.
"But it seems like not long enough..."]]))

   vn.run()

   faction.get("Lost"):setKnown(true)

   -- Will start the mission, so we can trigger the hook
   naev.missionStart("Old Friends at Protera Husk")

   -- Trigger the mission, skipping the initial npc.
   naev.trigger("wildspace_start_misn")

   evt.finish(true)
end

function enter ()
   if system.cur() ~= mainsys then
      evt.finish(false)
   end
end
