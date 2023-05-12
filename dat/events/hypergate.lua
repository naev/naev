--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Hypergate">
 <location>none</location>
 <chance>0</chance>
</event>
--]]
local audio = require 'love.audio'
local lf = require "love.filesystem"
local pp_shaders = require "pp_shaders"

local pixelcode = lf.read( "glsl/love/hypergate_travel.frag" )


local target, shader, prevtex
local sfx = audio.newSource( 'snd/sounds/hypergate.ogg' )
function create ()

   target = var.peek("hypergate_target")
   if not target then
      warn(_("Hypergate event run with no target!"))
      return
   end

   hook.safe( "hypergate" )
end

function hypergate ()
   -- Teleport right away because we do some tricks.
   -- Note that this might interfere with missions that run code right away when entering a system with hooks
   player.teleport( target )
   local pp = player.pilot()
   pp:setPos( pp:pos() + vec2.newP( 100+100*rnd.rnd(), rnd.angle() ) )

   hook.update( "update" )
   sfx:play()

   -- Get colour
   local col = naev.cache().hypergate_colour or { 0, 0.8, 0.6 }

   prevtex = gfx.screenshot():getTex()
   shader = pp_shaders.newShader( pixelcode )
   shader:send( "u_prevtex", prevtex )
   shader:send( "u_colour", col )
   shader.addPPShader( shader, "final" )
end

local timer = 0
local jumptime = 4.0
function update( _dt, real_dt )
   timer = timer + real_dt
   shader:send( "u_progress", math.min(timer/jumptime,1.0) )
   if timer >= jumptime  then
      shader.rmPPShader( shader )
      var.pop("hypergate_target")
      evt.finish()
   end
end
