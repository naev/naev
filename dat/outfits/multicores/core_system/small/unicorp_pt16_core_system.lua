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
local ew_detect
local cooldown_time

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
   add_desc( _("Ship Mass"), naev.unit("mass"), "+8"," +57" )
   desc=desc.."#0"
   add_desc( _("CPU max"), "", "+16"," +52" )
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+150"," +75" )
   add_desc( _("Energy Regeneration"), "", "+7"," +3" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+150"," +30" )
   add_desc( _("Shield Regeneration"), "", "+4.5"," +1" )
   add_desc( _("Detection"), "", "+10"," _" )
   add_desc( _("Ship Cooldown Time"), "", "-25"," _" )

   return desc
end

function init( _p, po )
   nomain=false
   nosec=false
   if not po:slot().tags.secondary then
      if po:slot().tags.core then
         nosec=true
      end
      mass=8
      cpu_max=16
      energy=150
      energy_regen=7
      shield=150
      shield_regen=4.5
      ew_detect=10
      cooldown_time=-25
   else
      nomain=true
      mass=57
      cpu_max=52
      energy=75
      energy_regen=3
      shield=30
      shield_regen=1
      ew_detect=0
      cooldown_time=0
   end
   po:set( "mass", mass )
   po:set( "cpu_max", cpu_max )
   po:set( "energy", energy )
   po:set( "energy_regen", energy_regen )
   po:set( "shield", shield )
   po:set( "shield_regen", shield_regen )
   po:set( "ew_detect", ew_detect )
   po:set( "cooldown_time", cooldown_time )
end
