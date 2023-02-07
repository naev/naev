--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Wormhole">
 <location>none</location>
 <chance>0</chance>
</event>
--]]
local audio = require 'love.audio'
local lf = require "love.filesystem"
local pp_shaders = require "pp_shaders"

local pixelcode = lf.read( "glsl/love/wormhole_travel.frag" )

local target, shader, r
local sfx = audio.newSource( 'snd/sounds/wormhole.ogg' )
function create ()

   target = var.peek("wormhole_target")
   if not target then
      warn(_("Wormhole event run with no target!"))
      return
   end
   hook.update( "update" )
   sfx:play()

   shader = pp_shaders.newShader( pixelcode )
   shader.addPPShader( shader, "final" )
   r = -rnd.rnd()*1000
end

local timer = 0
local jumped = false
local jumptime = 2.0
function update( _dt, real_dt )
   timer = timer + real_dt
   shader:send( "u_time", timer+r )
   shader:send( "u_progress", math.min(timer/jumptime,1.0) )
   if timer >= jumptime then
      if not jumped then
         jumped = true
         r = r + timer
         timer = 0
         shader:send( "u_progress", 0 ) -- avoids visual artefact
         shader:send( "u_invert", 1 )
         hook.safe( "wormhole" )
      else
         shader.rmPPShader( shader )
         var.pop("wormhole_target")
         evt.finish()
      end
   end
end

function wormhole ()
   player.teleport( target )
   local pp = player.pilot()
   pp:setPos( pp:pos() + vec2.newP( 100+100*rnd.rnd(), rnd.angle() ) )
   pp:shipvarPop( "wormhole" ) -- Clear wormhole
   pp:effectAdd("Wormhole Exit")

   -- Increment time
   time.inc( time.new( 0, 0, 1000 + 2000*rnd.rnd() ) )
end
