notactive = true
local add_desc=require "outfits.multicore.desc"
local slotflags=require "outfits.multicore.slotflags"

function descextra( _p, _o, po )
   local desc = ""
   local nosec
   local nomain

   nomain,nosec=slotflags(po)

   local unit_energy= naev.unit("energy")
   local unit_power= naev.unit("power")
   desc=desc.."#r"
   desc=add_desc(desc, _("Ship Mass"), naev.unit("mass"), "+18", "+18", "#r", nomain, nosec)
   desc=desc.."#g"
   desc=add_desc(desc, _("CPU Capacity"), naev.unit("cpu"), "+10", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Capacity"), unit_energy, "+100", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Regeneration"), unit_power, "+5", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Capacity"), unit_energy, "+110", "_", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Regeneration"), unit_power, "+4", "_", "#g", nomain, nosec)

   return desc
end

function init(_p, po )
   local nomain
   local nosec
   nomain,nosec=slotflags(po)
   if nomain or nosec then
      local cpu_max
      local energy
      local energy_regen
      local shield
      local shield_regen
      if nosec then
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
