notactive = true
local nomain=false
local nosec=false
local add_desc=require "outfits.multicores.desc"

function descextra( _p, _po )
   local desc = ""

   desc=desc.."#r"
   desc=add_desc(desc, _("Ship Mass"), naev.unit("mass"), "+70", "+140", "#r", nomain, nosec)
   desc=desc.."#g"
   desc=add_desc(desc, _("CPU Capacity"), naev.unit("cpu"), "+200", "+110", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Capacity"), naev.unit("energy"), "+525", "+575", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Regeneration"), naev.unit("power"), "+17", "+19", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Capacity"), naev.unit("energy"), "+330", "+70", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Regeneration"), naev.unit("power"), "+6.5", "+1", "#g", nomain, nosec)
   desc=add_desc(desc, _("Detection"), naev.unit("percent"), "+10", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Ship Cooldown Time"), naev.unit("percent"), "-25", "_", "#g", nomain, nosec)

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
      local ew_detect
      local cooldown_time
      if not po:slot().tags.secondary then
         nosec=true
         nomain=false
         mass=70
         cpu_max=200
         energy=525
         energy_regen=17
         shield=330
         shield_regen=6.5
         ew_detect=10
         cooldown_time=-25
      else
         nosec=false
         nomain=true
         mass=140
         cpu_max=110
         energy=575
         energy_regen=19
         shield=70
         shield_regen=1
         ew_detect=0
         cooldown_time=0
      end
      po:set( "mass", mass )
      po:set( "cpu_max", cpu_max )
      po:set( "energy", energy )
      po:set( "energy_regen", energy_regen )
      po:set( "shield", shield )
      po:set( "shield_regen", shield_regen )
      po:set( "ew_detect", ew_detect )
      po:set( "cooldown_time", cooldown_time )
   else
      nosec=false
      nomain=false
   end
end
