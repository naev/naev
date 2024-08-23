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

local mainspb, mainsys = spob.getS("Hypergate Protera")

local derelict
function create ()
   evt.finish(false)

   derelict = pilot.add( "Koala", "Derelict", vec2.newP( system.cur():radius()*0.3, rnd.angle() ), p_("ship", "Derelict"), {ai="dummy", naked=true})
   derelict:disable()
   derelict:intrinsicSet( "ew_hide", 300 ) -- Much more visible
   derelict:intrinsicSet( "nebu_absorb", 100 ) -- Immune to nebula

   hook.timer( 25, "msg" )
   hook.enter( "enter" )
   hook.land( "land" )
end

function msg ()
   if not derelict:exists() then
      return
   end
   derelict:setVisplayer()
   derelict:hailPlayer()
   hook.pilot(derelict, "hail", "hail")

   player.landAllow( false, _("You do not see any place to land here.") )
end

function hail ()
   vn.clear()
   vn.scene()

   local c = vn.newCharacter( vni.soundonly( _("character","C") ) )
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

   system.markerAdd( mainspb:pos(), _("Land here") )
   player.landAllow()
end

function land ()
   vn.clear()
   vn.scene()

   local c = vn.newCharacter( vni.soundonly( _("character","C") ) )
   vn.na(_([[]]))
   c(_([[]]))

   vn.run()
end

function enter ()
   if system.cur() ~= mainsys then
      evt.finish(false)
   end
end
