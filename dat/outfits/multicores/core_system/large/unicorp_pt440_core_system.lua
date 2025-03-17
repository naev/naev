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
   add_desc( _("Ship Mass"), naev.unit("mass"), "+420"," +580" )
   desc=desc.."#0"
   add_desc( _("CPU max"), "", "+440"," +1310" )
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+1860"," +940" )
   add_desc( _("Energy Regeneration"), "", "+46"," +51" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+650"," +100" )
   add_desc( _("Shield Regeneration"), "", "+11"," +2" )
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
      mass=420
      cpu_max=440
      energy=1860
      energy_regen=46
      shield=650
      shield_regen=11
      ew_detect=10
      cooldown_time=-25
   else
      nomain=true
      mass=580
      cpu_max=1310
      energy=940
      energy_regen=51
      shield=100
      shield_regen=2
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
