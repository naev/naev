notactive = true
local nomain=false
local nosec=false
local add_desc=require "outfits.multicores.desc"

function descextra( _p, _po )
   local desc = ""

   desc=desc.."#r"
   desc=add_desc(desc, _("Ship Mass"), naev.unit("mass"), "+540", "+760", "#r", nomain, nosec)
   desc=desc.."#g"
   desc=add_desc(desc, _("CPU max"), naev.unit("cpu"), "+540", "+1660", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Capacity"), naev.unit("energy"), "+2460", "+1380", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Regeneration"), naev.unit("power"), "+66", "+74", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Capacity"), naev.unit("energy"), "+850", "+250", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Regeneration"), naev.unit("power"), "+15", "+3", "#g", nomain, nosec)

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
         mass=540
         cpu_max=540
         energy=2460
         energy_regen=66
         shield=850
         shield_regen=15
      else
         nosec=false
         nomain=true
         mass=760
         cpu_max=1660
         energy=1380
         energy_regen=74
         shield=250
         shield_regen=3
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
