max_regen = 30

-- Init function run on creation
function init( p, po )
   po:clear()
   po:state("off")
   local nebu_dens, nebu_vol = system.cur():nebula()
   mem.nebu_vol = nebu_vol
   mem.force_off = false
end

function update( p, po )
   if mem.nebu_vol <= 0 or mem.force_off then
      return
   end
   po:state("on")
   local regen = math.min( max_regen, mem.nebu_vol )
   po:set( "shield_regen_malus", -regen )
   po:set( "energy_loss", regen )
end


function ontoggle( p, po, on )
   if mem.nebu_vol <= 0 then
      return
   end
   if on then
      po:state("on")
   else
      po:state("off")
      mem.force_off = true
   end
end
