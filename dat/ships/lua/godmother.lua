local fmt = require "format"

function descextra( _p, _s )
   return "#o"..fmt.f(_("Bombers deployed by this ship get +{dmg}% launcher damage and fire rate and +{ammo}% launcher ammunition capacity."), {
      dmg   = 10,
      ammo  = 50,
   } ).."#0"
end
