require 'ai.pirate'

mem.armour_run    = 40
mem.careful       = false
mem.vulnrambo     = 2.0
mem.vulnattack    = 3.0
mem.vulnabort     = 5.0
mem.stealth       = false

local create_pirate = create
function create ()
   create_pirate()
   -- Override the pirate settings
   mem.stealth    = (rnd.rnd() < 0.5) -- Will drop out of stealth more
   mem.atk_kill   = (rnd.rnd() < 0.5) -- Kill more than pirates
   mem.simplecombat = (rnd.rnd() < 0.5) -- Not always dodging
   mem.formation  = nil -- No formation
end
