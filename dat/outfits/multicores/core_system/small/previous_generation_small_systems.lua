notactive = true

local fmt = require "format"

local nomain=false
local nosec=false

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

   add_desc( _("CPU max"), "", "+10"," _" )
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+100"," _" )
   add_desc( _("Energy Regeneration"), "", "+5"," _" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+110"," _" )
   add_desc( _("Shield Regeneration"), "", "+4"," _" )

   return desc
end

function init( _p, po )
   nomain=false
   nosec=false
   if not po:slot().tags.secondary then
      if po:slot().tags.core then
         nosec=true
      end
      cpu_max=10
      energy=100
      energy_regen=5
      shield=110
      shield_regen=4
   else
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
end
