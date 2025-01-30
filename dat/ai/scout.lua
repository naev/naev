require 'ai.core.core'
require 'ai.core.control.scout'

function create ()
   create_pre()
   mem.atk_skill  = 0.75 + 0.25*rnd.sigma()
   create_post()
end
