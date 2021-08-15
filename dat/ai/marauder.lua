require 'ai.pirate'

mem.armour_run    = 60
mem.careful       = false
mem.vulnrambo     = 2.0
mem.vulnattack    = 3.0
mem.vulnabort     = 5.0

create_pirate = create

function create ()
   create_pirate()

   -- Kill more than pirates
   mem.atk_kill = (rnd.rnd() < 0.5)
   mem.stealth  = false
end
