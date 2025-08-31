local pir = require 'equipopt.templates.pirate_base'
local eoutfits = require 'equipopt.outfits'

pir.outfits = eoutfits.merge{ pir.outfits, {
   "Hyena Bay", "Lancelot Bay", "Proteron Dalton Bay", "Empire Lancelot Bay",
   "Hyena Dock", "Proteron Dalton Dock",
}}

pir.params_overwrite = {
   weap = 2, -- Focus on weapons!
   max_same_stru = 1,
   max_same_util = 1,
}

pir.type = "lost"

return pir.make_equip( pir )
