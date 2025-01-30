require 'ai.pirate'

mem.armour_run    = 40
mem.careful       = false
mem.vulnrambo     = 1.3
mem.vulnattack    = 1.5
mem.vulnabort     = 2.1
mem.stealth       = false

local create_pirate = create
function create ()
   create_pirate()
   -- Override the pirate settings
   mem.stealth    = (rnd.rnd() < 0.5) -- Will drop out of stealth more
   mem.atk_kill   = (rnd.rnd() < 0.5) -- Kill more than pirates
   mem.atk_skill  = 0.5+0.25*rnd.sigma() -- Not too skilled
   mem.formation  = nil -- No formation
end
