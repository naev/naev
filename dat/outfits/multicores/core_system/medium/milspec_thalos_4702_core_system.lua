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
   add_desc( _("Ship Mass"), naev.unit("mass"), "+80"," +170", "#r" )
   desc=desc.."#g"
   add_desc( _("CPU max"), "", "+300"," +120" , "#g" )
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+675"," +725", "#g" )
   add_desc( _("Energy Regeneration"), "", "+23"," +20" , "#g" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+400"," +100", "#g" )
   add_desc( _("Shield Regeneration"), "", "+8"," +1" , "#g" )

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
