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
   desc=add_desc(desc, _("Ship Mass"), naev.unit("mass"), "+500", "+700", "#r", nomain, nosec)
   desc=desc.."#g"
   desc=add_desc(desc, _("CPU Capacity"), naev.unit("cpu"), "+680", "+2120", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Capacity"), unit_energy, "+2300", "+1060", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Regeneration"), unit_power, "+53", "+57", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Capacity"), unit_energy, "+800", "+100", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Regeneration"), unit_power, "+14", "_", "#g", nomain, nosec)

   return desc
end

function init(_p, po )
   local nomain
   local nosec
   nomain,nosec=slotflags(po)
   if nomain or nosec then
      local mass
      local cpu_max
      local energy
      local energy_regen
      local shield
      local shield_regen
      if nosec then
         nosec=true
         nomain=false
         mass=500
         cpu_max=680
         energy=2300
         energy_regen=53
         shield=800
         shield_regen=14
      else
         nosec=false
         nomain=true
         mass=700
         cpu_max=2120
         energy=1060
         energy_regen=57
         shield=100
         shield_regen=0
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
