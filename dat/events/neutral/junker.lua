--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Meet the Junker">
 <location>none</location>
 <chance>0</chance>
 <unique />
</event>
--]]
local vn = require "vn"
local ccomm = require "common.comm"
local jlib = require "common.junker"

local junker
function create ()
   -- Enter the Junker
   junker = jlib.spawn_pilot( player.pos() + vec2.newP( 2500, rnd.angle() ) )
   junker:control(true)
   junker:follow( player.pilot() )
   junker:setInvincible(true)

   hook.timer( 3, "meet_the_junker" )
   hook.enter( "done" )
end

function meet_the_junker ()
   junker:setHostile(true)
   vn.clear()
   vn.scene()
   local j = ccomm.newCharacter( vn, junker )
   vn.transition()
   vn.na(_([[An audio-only connection opens with your ship.]]))
   j(_([["You're the one who stole my pack! Give it back!"]]))
   vn.na(_([[Before you can respond, the communication closes and they charge you.]]))

   vn.run()

   junker:control(false)
   junker:setInvincible(false)
   evt.finish(true)
end
