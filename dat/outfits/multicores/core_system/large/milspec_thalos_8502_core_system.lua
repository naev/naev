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
   add_desc( _("Ship Mass"), naev.unit("mass"), "+500"," +700" )
   desc=desc.."#0"
   add_desc( _("CPU max"), "", "+680"," +2120" )
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+2300"," +1060" )
   add_desc( _("Energy Regeneration"), "", "+53"," +57" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+800"," +100" )
   add_desc( _("Shield Regeneration"), "", "+14"," _" )

   return desc
end

function init( _p, po )
   nomain=false
   nosec=false
   if not po:slot().tags.secondary then
      if po:slot().tags.core then
         nosec=true
      end
      mass=500
      cpu_max=680
      energy=2300
      energy_regen=53
      shield=800
      shield_regen=14
   else
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
end
