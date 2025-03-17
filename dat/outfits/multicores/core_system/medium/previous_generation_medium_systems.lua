notactive = true

local fmt = require "format"

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

   if po:get( "cpu_max")==180 then
      nosec=true
   elseif po:get( "cpu_max")==0 then
      nomain=true
   end
   add_desc( _("CPU max"), "", "+180"," _" , "#g", nomain,nosec)
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+580"," _", "#g", nomain,nosec)
   add_desc( _("Energy Regeneration"), "", "+23"," _" , "#g", nomain,nosec)
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+310"," _", "#g", nomain,nosec)
   add_desc( _("Shield Regeneration"), "", "+7"," _" , "#g", nomain,nosec)

   return desc
end

function init( _p, po )
   print(po)
   if po:slot().tags and po:slot().tags.core then
      if not po:slot().tags.secondary then
         cpu_max=180
         energy=580
         energy_regen=23
         shield=310
         shield_regen=7
      else
         cpu_max=0
         energy=0
         energy_regen=0
         shield=0
         shield_regen=0
      end
   else
      po:set( "cpu_max", cpu_max )
      po:set( "energy", energy )
      po:set( "energy_regen", energy_regen )
      po:set( "shield", shield )
      po:set( "shield_regen", shield_regen )
      po:set( "cpu_max", -128 )
   end
end

function onremove( p, po)
   init(p,po)
end

function onadd( p, po )
   init(p,po)
end

