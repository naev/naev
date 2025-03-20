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
   desc=add_desc(desc, _("Ship Mass"), naev.unit("mass"), "+80", "+170", "#r", nomain, nosec)
   desc=desc.."#g"
   desc=add_desc(desc, _("CPU Capacity"), naev.unit("cpu"), "+300", "+120", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Capacity"), unit_energy, "+675", "+725", "#g", nomain, nosec)
   desc=add_desc(desc, _("Energy Regeneration"), unit_power, "+23", "+20", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Capacity"), unit_energy, "+400", "+100", "#g", nomain, nosec)
   desc=add_desc(desc, _("Shield Regeneration"), unit_power, "+8", "+1", "#g", nomain, nosec)

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
         mass=80
         cpu_max=300
         energy=675
         energy_regen=23
         shield=400
         shield_regen=8
      else
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
   end
end
