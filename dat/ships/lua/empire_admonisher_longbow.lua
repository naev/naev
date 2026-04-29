update_dt = 1

function init( p )
   mem.val = 0
   if p then
      update( p )
   end
end

function update( p )
   local ew_detect = p:shipstat("ew_detect")
   if mem.val ~= ew_detect then
      p:shippropSet( "weapon_range", ew_detect )
      mem.val = ew_detect
   end
end
