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
   add_desc( _("Ship Mass"), naev.unit("mass"), "+540"," +760" )
   desc=desc.."#0"
   add_desc( _("CPU max"), "", "+540"," +1660" )
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+2460"," +1380" )
   add_desc( _("Energy Regeneration"), "", "+66"," +74" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+850"," +250" )
   add_desc( _("Shield Regeneration"), "", "+15"," +3" )

   return desc
end

function init( _p, po )
   nomain=false
   nosec=false
   if not po:slot().tags.secondary then
      if po:slot().tags.core then
         nosec=true
      end
      mass=540
      cpu_max=540
      energy=2460
      energy_regen=66
      shield=850
      shield_regen=15
   else
      nomain=true
      mass=760
      cpu_max=1660
      energy=1380
      energy_regen=74
      shield=250
      shield_regen=3
   end
   po:set( "mass", mass )
   po:set( "cpu_max", cpu_max )
   po:set( "energy", energy )
   po:set( "energy_regen", energy_regen )
   po:set( "shield", shield )
   po:set( "shield_regen", shield_regen )
end
