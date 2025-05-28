update_dt = 1

local val
function init( p )
   val = 0
   update( p )
end

function update( p )
   local ew_detect = p:shipstat("ew_detect")
   if val ~= ew_detect then
      p:shippropSet( "weapon_range", ew_detect )
      val = ew_detect
   end
end
