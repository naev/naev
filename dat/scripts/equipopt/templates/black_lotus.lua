local eoutfits = require 'equipopt.outfits'
local pir = require 'equipopt.templates.pirate_base'
-- Don't overwrite in case is used in same env as another pirate
pir = tcopy(pir)

-- Add Black Lotus only gear
pir.outfits = eoutfits.merge{ pir.outfits, {
   "Arc Blade", "Arc Machete",
}}

-- Slightly tune equipment parameters
pir.params_overwrite = tcopy( pir.params_overwrite )
-- slightly prefer disable weapons
pir.params_overwrite.disable = 1.1
pir.params_overwrite.damage = 1.1

return pir.make_equip( pir )
