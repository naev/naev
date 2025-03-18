notactive = true
local nomain=false
local nosec=false
local add_desc=require "outfits.multicores.desc"

function descextra( _p, _po )
   local desc = ""

   desc=desc.."#r"
   desc=add_desc(desc, _("Ship Mass"), naev.unit("mass"), "+420"," +580", "#r", nomain, nosec)
   desc=desc.."#g"
   desc=add_desc(desc, _("CPU max"), "", "+440"," +1310" , "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Capacity"), naev.unit("energy"), "+1860"," +940", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Regeneration"), "", "+46"," +51" , "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Capacity"), naev.unit("energy"), "+650"," +100", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Regeneration"), "", "+11"," +2" , "#g", nomain, nosec)
   desc=add_desc(desc, _("Detection"), "", "+10"," _" , "#g", nomain, nosec)
   desc=add_desc(desc, _("Ship Cooldown Time"), "", "-25"," _" , "#g", nomain, nosec)

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
         mass=420
         cpu_max=440
         energy=1860
         energy_regen=46
         shield=650
         shield_regen=11
         ew_detect=10
         cooldown_time=-25
      else
         nosec=false
         nomain=true
         mass=580
         cpu_max=1310
         energy=940
         energy_regen=51
         shield=100
         shield_regen=2
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
