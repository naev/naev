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

   if po:get( "mass")==540 then
      nosec=true
   elseif po:get( "mass")==760 then
      nomain=true
   end
   desc=desc.."#r"
   add_desc( _("Ship Mass"), naev.unit("mass"), "+540"," +760", "#r", nomain,nosec)
   desc=desc.."#g"
   add_desc( _("CPU max"), "", "+540"," +1660" , "#g", nomain,nosec)
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+2460"," +1380", "#g", nomain,nosec)
   add_desc( _("Energy Regeneration"), "", "+66"," +74" , "#g", nomain,nosec)
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+850"," +250", "#g", nomain,nosec)
   add_desc( _("Shield Regeneration"), "", "+15"," +3" , "#g", nomain,nosec)

   return desc
end

function init( _p, po )
   print(po)
   if po:slot().tags and po:slot().tags.core then
      if not po:slot().tags.secondary then
         mass=540
         cpu_max=540
         energy=2460
         energy_regen=66
         shield=850
         shield_regen=15
      else
         mass=760
         cpu_max=1660
         energy=1380
         energy_regen=74
         shield=250
         shield_regen=3
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

