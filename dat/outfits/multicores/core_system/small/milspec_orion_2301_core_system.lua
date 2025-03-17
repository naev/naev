notactive = true

local fmt = require "format"

local nomain=false
local nosec=false

local mass
local cpu_max
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
   add_desc( _("Ship Mass"), naev.unit("mass"), "+14"," +61" )
   desc=desc.."#0"
   add_desc( _("CPU max"), "", "+18"," +52" )
   add_desc( _("Energy Regeneration"), "", "+10"," +11" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+200"," +50" )
   add_desc( _("Shield Regeneration"), "", "+7"," +1" )

   return desc
end

function init( _p, po )
   nomain=false
   nosec=false
   if not po:slot().tags.secondary then
      if po:slot().tags.core then
         nosec=true
      end
      mass=14
      cpu_max=18
      energy_regen=10
      shield=200
      shield_regen=7
   else
      nomain=true
      mass=61
      cpu_max=52
      energy_regen=11
      shield=50
      shield_regen=1
   end
   po:set( "mass", mass )
   po:set( "cpu_max", cpu_max )
   po:set( "energy_regen", energy_regen )
   po:set( "shield", shield )
   po:set( "shield_regen", shield_regen )
end
