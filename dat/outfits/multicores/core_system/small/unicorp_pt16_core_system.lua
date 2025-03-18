notactive = true
local nomain=false
local nosec=false
local add_desc=require "outfits.multicores.desc"

function descextra( _p, _po )
   local desc = ""

   desc=desc.."#r"
   desc=add_desc(desc, _("Ship Mass"), naev.unit("mass"), "+8", "+57", "#r", nomain, nosec)
   desc=desc.."#g"
   desc=add_desc(desc, _("CPU max"), "", "+16", "+52" , "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Capacity"), naev.unit("energy"), "+150", "+75", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Regeneration"), naev.unit("power"), "+7", "+3", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Capacity"), naev.unit("energy"), "+150", "+30", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Regeneration"), naev.unit("power"), "+4.5", "+1", "#g", nomain, nosec)
   desc=add_desc(desc, _("Detection"), "", "+10", "_" , "#g", nomain, nosec)
   desc=add_desc(desc, _("Ship Cooldown Time"), "", "-25", "_" , "#g", nomain, nosec)

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
         mass=8
         cpu_max=16
         energy=150
         energy_regen=7
         shield=150
         shield_regen=4.5
         ew_detect=10
         cooldown_time=-25
      else
         nosec=false
         nomain=true
         mass=57
         cpu_max=52
         energy=75
         energy_regen=3
         shield=30
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
