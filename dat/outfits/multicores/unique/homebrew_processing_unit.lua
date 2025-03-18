notactive = true
local nomain=false
local nosec=false
local add_desc=require "outfits.multicores.desc"

function descextra( _p, _po )
   local desc = ""

   desc=desc.."#r"
   desc=add_desc(desc, _("Ship Mass"), naev.unit("mass"), "+10", "+10", "#r", nomain, nosec)
   desc=desc.."#g"
   desc=add_desc(desc, _("CPU max"), naev.unit("cpu"), "+10", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Capacity"), naev.unit("energy"), "+140", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Regeneration"), naev.unit("power"), "+7", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Capacity"), naev.unit("energy"), "+130", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Regeneration"), naev.unit("power"), "+4", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Detection"), naev.unit("distance"), "+15", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Ship Cooldown Time"), naev.unit("percent"), "-25", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("jump_warmup"), naev.unit("percent"), "-60", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("land_delay"), naev.unit("percent"), "-25", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("jump_delay"), naev.unit("percent"), "-25", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("ew_hide"), naev.unit("distance"), "-20", "_", "#g", nomain, nosec)

   return desc
end

function init(_p, po )
   if po:slot().tags and po:slot().tags.core then
      local cpu_max
      local energy
      local energy_regen
      local shield
      local shield_regen
      local ew_detect
      local cooldown_time
      local jump_warmup
      local land_delay
      local jump_delay
      local ew_hide
      if not po:slot().tags.secondary then
         nosec=true
         nomain=false
         cpu_max=10
         energy=140
         energy_regen=7
         shield=130
         shield_regen=4
         ew_detect=15
         cooldown_time=-25
         jump_warmup=-60
         land_delay=-25
         jump_delay=-25
         ew_hide=-20
      else
         nosec=false
         nomain=true
         cpu_max=0
         energy=0
         energy_regen=0
         shield=0
         shield_regen=0
         ew_detect=0
         cooldown_time=0
         jump_warmup=0
         land_delay=0
         jump_delay=0
         ew_hide=0
      end
      po:set( "cpu_max", cpu_max )
      po:set( "energy", energy )
      po:set( "energy_regen", energy_regen )
      po:set( "shield", shield )
      po:set( "shield_regen", shield_regen )
      po:set( "ew_detect", ew_detect )
      po:set( "cooldown_time", cooldown_time )
      po:set( "jump_warmup", jump_warmup )
      po:set( "land_delay", land_delay )
      po:set( "jump_delay", jump_delay )
      po:set( "ew_hide", ew_hide )
   else
      nosec=false
      nomain=false
   end
end
