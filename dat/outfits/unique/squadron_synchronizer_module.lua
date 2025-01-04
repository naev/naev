require "outfits.unique.poi_data"
local fmt = require "format"

function descextra( _p, _o )
   return fmt.f(_("Increases Fleet Capacity by {amt}. Does not need to be equipped."), {amt=25})
end
