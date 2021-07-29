require 'ai.pirate'

create_pirate = create

function create ()
   create_pirate()

   -- Kill more than pirates
   mem.atk_kill = (rnd.rnd() < 0.5)
   mem.stealth  = false
end
