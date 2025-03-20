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
   desc=add_desc(desc, _("Ship Mass"), naev.unit("mass"), "+90", "+180", "#r", nomain, nosec)
   desc=desc.."#g"
   desc=add_desc(desc, _("CPU Capacity"), naev.unit("cpu"), "+260", "+100", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Capacity"), unit_energy, "+750", "+850", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Regeneration"), unit_power, "+33", "+20", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Capacity"), unit_energy, "+450", "+130", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Regeneration"), unit_power, "+10", "+2", "#g", nomain, nosec)

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
         mass=90
         cpu_max=260
         energy=750
         energy_regen=33
         shield=450
         shield_regen=10
      else
         mass=180
         cpu_max=100
         energy=850
         energy_regen=20
         shield=130
         shield_regen=2
      end
      po:set( "mass", mass )
      po:set( "cpu_max", cpu_max )
      po:set( "energy", energy )
      po:set( "energy_regen", energy_regen )
      po:set( "shield", shield )
      po:set( "shield_regen", shield_regen )
   end
end
