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

   add_desc( _("CPU max"), "", "+180"," _" )
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+580"," _" )
   add_desc( _("Energy Regeneration"), "", "+23"," _" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+310"," _" )
   add_desc( _("Shield Regeneration"), "", "+7"," _" )

   return desc
end

function init( _p, po )
   nomain=false
   nosec=false
   if not po:slot().tags.secondary then
      if po:slot().tags.core then
         nosec=true
      end
      cpu_max=180
      energy=580
      energy_regen=23
      shield=310
      shield_regen=7
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
