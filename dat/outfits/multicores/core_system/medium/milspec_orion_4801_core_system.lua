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

   if po:get( "mass")==90 then
      nosec=true
   elseif po:get( "mass")==180 then
      nomain=true
   end
   desc=desc.."#r"
   add_desc( _("Ship Mass"), naev.unit("mass"), "+90"," +180", "#r", nomain,nosec)
   desc=desc.."#g"
   add_desc( _("CPU max"), "", "+260"," +100" , "#g", nomain,nosec)
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+750"," +850", "#g", nomain,nosec)
   add_desc( _("Energy Regeneration"), "", "+33"," +20" , "#g", nomain,nosec)
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+450"," +130", "#g", nomain,nosec)
   add_desc( _("Shield Regeneration"), "", "+10"," +2" , "#g", nomain,nosec)

   return desc
end

function init( _p, po )
   print(po)
   if po:slot().tags and po:slot().tags.core then
      if not po:slot().tags.secondary then
         mass=90
         cpu_max=260
         energy=750
         energy_regen=33
         shield=450
         shield_regen=10
      else
         mass=180
         cpu_max=100
         energy=850
         energy_regen=20
         shield=130
         shield_regen=2
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

