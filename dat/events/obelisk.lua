--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Obelisk">
 <location>none</location>
 <chance>0</chance>
</event>
--]]
local audio = require 'love.audio'
local lf = require "love.filesystem"
local pp_shaders = require "pp_shaders"

local pixelcode_enter = lf.read( "glsl/love/obelisk_enter.frag" )
local pixelcode_exit = lf.read( "glsl/love/obelisk_exit.frag" )

local target, shader
local sfx = audio.newSource( 'snd/sounds/gamelan_gong.ogg' )
function create ()
   target = var.peek("obelisk_target")
   if not target then
      warn(_("Obelisk event run with no target!"))
      return
   end
   target = system.get(target)
   hook.update( "update" )

   shader = pp_shaders.newShader( pixelcode_enter )
   shader.addPPShader( shader, "gui" )
end

local timer = 0
local jumped = false
local jumptime = 2.0
function update( _dt, real_dt )
   timer = timer + real_dt
   shader:send( "u_progress", timer/jumptime )
   if timer >= jumptime then
      if not jumped then
         jumped = true
         timer = 0
         shader.rmPPShader( shader )
         shader = pp_shaders.newShader( pixelcode_exit )
         shader.addPPShader( shader, "gui" )
         hook.safe( "obelisk" )
      else
         shader.rmPPShader( shader )
         var.pop("obelisk_target")
         evt.finish()
      end
   end
end

function obelisk ()
   time.inc( time.new( 0, 0, 1000 + 2000*rnd.rnd() ) )
   player.teleport( target, true, true )
   local pp = player.pilot()
   pp:shipvarPop( "obelisk" ) -- Clear obelisk
   sfx:play()
end
