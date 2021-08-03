require 'ai.pirate'

mem.armour_run    = 60
mem.careful       = false
mem.vulnrambo     = 1.3
mem.vulnattack    = 0.8
mem.vulnabort     = 0.4

create_pirate = create

function create ()
   create_pirate()

   -- Kill more than pirates
   mem.atk_kill = (rnd.rnd() < 0.5)
   mem.stealth  = false
end
