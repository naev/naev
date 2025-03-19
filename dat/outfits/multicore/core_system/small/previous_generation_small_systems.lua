notactive = true
local nomain=false
local nosec=false
local add_desc=require "outfits.multicore.desc"

function descextra( _p, _po )
   local desc = ""

   desc=desc.."#r"
   desc=add_desc(desc, _("Ship Mass"), naev.unit("mass"), "+18", "+18", "#r", nomain, nosec)
   desc=desc.."#g"
   desc=add_desc(desc, _("CPU Capacity"), naev.unit("cpu"), "+10", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Capacity"), naev.unit("energy"), "+100", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Regeneration"), naev.unit("power"), "+5", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Capacity"), naev.unit("energy"), "+110", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Regeneration"), naev.unit("power"), "+4", "_", "#g", nomain, nosec)

   return desc
end

function init(_p, po )
   if po:slot().tags and po:slot().tags.core then
      local cpu_max
      local energy
      local energy_regen
      local shield
      local shield_regen
      if not po:slot().tags.secondary then
         nosec=true
         nomain=false
         cpu_max=10
         energy=100
         energy_regen=5
         shield=110
         shield_regen=4
      else
         nosec=false
         nomain=true
         cpu_max=0
         energy=0
         energy_regen=0
         shield=0
         shield_regen=0
      end
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
