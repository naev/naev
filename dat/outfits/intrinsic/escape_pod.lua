local escapepod = require "outfits.lib.escape_pod"

function init ()
   mem.launched = false
end

function ondeath( p )
   if not mem.launched then
      escapepod.launch( p )
      mem.launched = true
   end
end
