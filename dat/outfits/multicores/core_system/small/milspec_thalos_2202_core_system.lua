notactive = true

local fmt = require "format"

local mass
local cpu_max
local energy
local energy_regen
local shield
local shield_regen

function descextra( _p, po )
   local desc = ""

   local function _vu( val, unit)
      if val=='_' then
         return val
      else
         return val.." "..unit
      end
   end

   local function vu( val, unit, grey, def)
      local res=_vu(val,unit)
      if grey then
         return "#b"..res..def
      else
         return res
      end
   end

   local function add_desc( name, units, base, secondary, def,no_main,no_sec)
      desc = desc..fmt.f(_("\n{name}: {bas} / {sec}"), {
         name=name, units=units, bas=vu(base,units,no_main,def), sec=vu(secondary,units,no_sec,def)
      })
   end

   local nomain=false
   local nosec=false

   if po:get( "mass")==12 then
      nosec=true
   elseif po:get( "mass")==58 then
      nomain=true
   end
   desc=desc.."#r"
   add_desc( _("Ship Mass"), naev.unit("mass"), "+12"," +58", "#r", nomain,nosec)
   desc=desc.."#g"
   add_desc( _("CPU max"), "", "+24"," +66" , "#g", nomain,nosec)
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+150"," +200", "#g", nomain,nosec)
   add_desc( _("Energy Regeneration"), "", "+8"," +7" , "#g", nomain,nosec)
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+180"," +45", "#g", nomain,nosec)
   add_desc( _("Shield Regeneration"), "", "+6"," +1" , "#g", nomain,nosec)

   return desc
end

function init( _p, po )
   print(po)
   if po:slot().tags and po:slot().tags.core then
      if not po:slot().tags.secondary then
         mass=12
         cpu_max=24
         energy=150
         energy_regen=8
         shield=180
         shield_regen=6
      else
         mass=58
         cpu_max=66
         energy=200
         energy_regen=7
         shield=45
         shield_regen=1
      end
   else
      po:set( "mass", mass )
      po:set( "cpu_max", cpu_max )
      po:set( "energy", energy )
      po:set( "energy_regen", energy_regen )
      po:set( "shield", shield )
      po:set( "shield_regen", shield_regen )
      po:set( "mass", -128 )
   end
end

function onremove( p, po)
   init(p,po)
end

function onadd( p, po )
   init(p,po)
end

