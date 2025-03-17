notactive = true

local fmt = require "format"

local nomain=false
local nosec=false

local mass
local cpu_max
local energy
local energy_regen
local shield
local shield_regen

function descextra( _p, _o )
   local desc = ""

   local function _vu( val, unit)
      if val=='_' then
         return val
      else
         return val.." "..unit
      end
   end

   local function vu( val, unit, grey)
      local res=_vu(val,unit)
      if grey then
         return "#b"..res.."#0"
      else
         return res
      end
   end

   local function add_desc( name, units, base, secondary )
      desc = desc..fmt.f(_("\n{name}: {bas} / {sec}"), {
         name=name, units=units, bas=vu(base,units,nomain), sec=vu(secondary,units,nosec)
      })
   end

   desc=desc.."#r"
   add_desc( _("Ship Mass"), naev.unit("mass"), "+80"," +170" )
   desc=desc.."#0"
   add_desc( _("CPU max"), "", "+300"," +120" )
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+675"," +725" )
   add_desc( _("Energy Regeneration"), "", "+23"," +20" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+400"," +100" )
   add_desc( _("Shield Regeneration"), "", "+8"," +1" )

   return desc
end

function init( _p, po )
   nomain=false
   nosec=false
   if not po:slot().tags.secondary then
      if po:slot().tags.core then
         nosec=true
      end
      mass=80
      cpu_max=300
      energy=675
      energy_regen=23
      shield=400
      shield_regen=8
   else
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
end
