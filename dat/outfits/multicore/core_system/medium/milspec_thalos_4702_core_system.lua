notactive = true
local nomain=false
local nosec=false
local add_desc=require "outfits.multicore.desc"

function descextra( _p, _po )
   local desc = ""

   desc=desc.."#r"
   desc=add_desc(desc, _("Ship Mass"), naev.unit("mass"), "+80", "+170", "#r", nomain, nosec)
   desc=desc.."#g"
   desc=add_desc(desc, _("CPU Capacity"), naev.unit("cpu"), "+300", "+120", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Capacity"), naev.unit("energy"), "+675", "+725", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Regeneration"), naev.unit("power"), "+23", "+20", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Capacity"), naev.unit("energy"), "+400", "+100", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Regeneration"), naev.unit("power"), "+8", "+1", "#g", nomain, nosec)

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
         mass=80
         cpu_max=300
         energy=675
         energy_regen=23
         shield=400
         shield_regen=8
      else
         nosec=false
         nomain=true
         mass=170
         cpu_max=120
         energy=725
         energy_regen=20
         shield=100
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
