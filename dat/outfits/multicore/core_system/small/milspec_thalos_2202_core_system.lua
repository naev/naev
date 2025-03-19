notactive = true
local nomain=false
local nosec=false
local add_desc=require "outfits.multicore.desc"

function descextra( _p, _po )
   local desc = ""

   desc=desc.."#r"
   desc=add_desc(desc, _("Ship Mass"), naev.unit("mass"), "+12", "+58", "#r", nomain, nosec)
   desc=desc.."#g"
   desc=add_desc(desc, _("CPU Capacity"), naev.unit("cpu"), "+24", "+66", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Capacity"), naev.unit("energy"), "+150", "+200", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Regeneration"), naev.unit("power"), "+8", "+7", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Capacity"), naev.unit("energy"), "+180", "+45", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Regeneration"), naev.unit("power"), "+6", "+1", "#g", nomain, nosec)

   return desc
end

function init(_p, po )
   if po:slot().tags and po:slot().tags.core then
      local mass
      local cpu_max
      local energy
      local energy_regen
      local shield
      local shield_regen
      if not po:slot().tags.secondary then
         nosec=true
         nomain=false
         mass=12
         cpu_max=24
         energy=150
         energy_regen=8
         shield=180
         shield_regen=6
      else
         nosec=false
         nomain=true
         mass=58
         cpu_max=66
         energy=200
         energy_regen=7
         shield=45
         shield_regen=1
      end
      po:set( "mass", mass )
      po:set( "cpu_max", cpu_max )
      po:set( "energy", energy )
      po:set( "energy_regen", energy_regen )
      po:set( "shield", shield )
      po:set( "shield_regen", shield_regen )
   else
      nosec=false
      nomain=false
   end
end
