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

   add_desc( _("CPU max"), "", "+380"," _" )
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+1760"," _" )
   add_desc( _("Energy Regeneration"), "", "+35"," _" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+500"," _" )
   add_desc( _("Shield Regeneration"), "", "+9"," _" )

   return desc
end

function init( _p, po )
   nomain=false
   nosec=false
   if not po:slot().tags.secondary then
      if po:slot().tags.core then
         nosec=true
      end
      cpu_max=380
      energy=1760
      energy_regen=35
      shield=500
      shield_regen=9
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
