notactive = true

local fmt = require "format"

local mass
local cpu_max
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

   if po:get( "mass")==14 then
      nosec=true
   elseif po:get( "mass")==61 then
      nomain=true
   end
   desc=desc.."#r"
   add_desc( _("Ship Mass"), naev.unit("mass"), "+14"," +61", "#r", nomain,nosec)
   desc=desc.."#g"
   add_desc( _("CPU max"), "", "+18"," +52" , "#g", nomain,nosec)
   add_desc( _("Energy Regeneration"), "", "+10"," +11" , "#g", nomain,nosec)
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+200"," +50", "#g", nomain,nosec)
   add_desc( _("Shield Regeneration"), "", "+7"," +1" , "#g", nomain,nosec)

   return desc
end

function init( _p, po )
   print(po)
   if po:slot().tags and po:slot().tags.core then
      if not po:slot().tags.secondary then
         mass=14
         cpu_max=18
         energy_regen=10
         shield=200
         shield_regen=7
      else
         mass=61
         cpu_max=52
         energy_regen=11
         shield=50
         shield_regen=1
      end
   else
      po:set( "mass", mass )
      po:set( "cpu_max", cpu_max )
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

