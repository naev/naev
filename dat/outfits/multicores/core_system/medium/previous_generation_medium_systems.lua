notactive = true
local nomain=false
local nosec=false
local add_desc=require "outfits.multicores.desc"

function descextra( _p, _po )
   local desc = ""

   desc=desc.."#r"
   desc=add_desc(desc, _("Ship Mass"), naev.unit("mass"), "+100", "+100", "#r", nomain, nosec)
   desc=desc.."#g"
   desc=add_desc(desc, _("CPU max"), "", "+180", "_" , "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Capacity"), naev.unit("energy"), "+580", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Regeneration"), naev.unit("power"), "+23", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Capacity"), naev.unit("energy"), "+310", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Regeneration"), naev.unit("power"), "+7", "_", "#g", nomain, nosec)

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
         cpu_max=180
         energy=580
         energy_regen=23
         shield=310
         shield_regen=7
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
