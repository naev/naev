--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Wormhole">
 <trigger>none</trigger>
 <chance>0</chance>
</event>
--]]
local audio = require 'love.audio'

-- luacheck: globals wormhole (Hook functions passed by name)

local target
local sfx = audio.weSource( 'snd/sounds/wormhole.ogg' )
function create ()
   target = var.peek("wormhole_target")
   if not target then
      warn(_("Wormhole event run with no target!"))
      return
   end
   hook.safe( "wormhole" )
   sfx:play()
   -- TODO nice animation, probably sync with shaders
end

function wormhole ()
   player.teleport( target )
   var.pop("wormhole_target")
   evt.finish()
end
