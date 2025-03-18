notactive = true

local fmt = require "format"

local nomain=false
local nosec=false

function descextra( _p, po )
   local desc = ""

   local function vu( val, unit)
      if val=='_' then
         return val
      else
         return val.." "..unit
      end
   end

   local function col( s, grey, def)
      if grey then
         return "#b"..s..def
      else
         return s
      end
   end

   local function add_desc( name, units, base, secondary, def)
      desc = desc..fmt.f(_("\n{name}: {bas} {sep} {sec}"), {
         name=name, units=units,
         sep=col("/",nomain or nosec,def),
         bas=col(vu(base,units),nomain,def),
         sec=col(vu(secondary,units),nosec,def),
      })
   end

   desc=desc.."#r"
   add_desc( _("Ship Mass"), naev.unit("mass"), "+500"," +700", "#r" )
   desc=desc.."#g"
   add_desc( _("CPU max"), "", "+680"," +2120" , "#g" )
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+2300"," +1060", "#g" )
   add_desc( _("Energy Regeneration"), "", "+53"," +57" , "#g" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+800"," +100", "#g" )
   add_desc( _("Shield Regeneration"), "", "+14"," _" , "#g" )

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
