notactive = true

local fmt = require "format"

local nomain=false
local nosec=false

function descextra( _p, po )
   local desc = ""

   print(fmt.f("desc {po}",{po=po}))
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

   local function add_desc( name, units, base, secondary, def)
      desc = desc..fmt.f(_("\n{name}: {bas} / {sec}"), {
         name=name, units=units, bas=vu(base,units,nomain,def), sec=vu(secondary,units,nosec,def)
      })
   end

   desc=desc.."#r"
   add_desc( _("Ship Mass"), naev.unit("mass"), "+12"," +58", "#r" )
   desc=desc.."#g"
   add_desc( _("CPU max"), "", "+24"," +66" , "#g" )
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+150"," +200", "#g" )
   add_desc( _("Energy Regeneration"), "", "+8"," +7" , "#g" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+180"," +45", "#g" )
   add_desc( _("Shield Regeneration"), "", "+6"," +1" , "#g" )

   return desc
end

function init(_p, po )
   print(fmt.f("init {po}",{po=po}))
   if po:slot().tags and po:slot().tags.core then
      local mass
      local cpu_max
      local energy
      local energy_regen
      local shield
      local shield_regen
      if not po:slot().tags.secondary then
         nosec=true
         nomain=false
         mass=12
         cpu_max=24
         energy=150
         energy_regen=8
         shield=180
         shield_regen=6
      else
         nosec=false
         nomain=true
         mass=58
         cpu_max=66
         energy=200
         energy_regen=7
         shield=45
         shield_regen=1
      end
      po:set( "mass", mass )
      po:set( "cpu_max", cpu_max )
      po:set( "energy", energy )
      po:set( "energy_regen", energy_regen )
      po:set( "shield", shield )
      po:set( "shield_regen", shield_regen )
   else
      nosec=false
      nomain=false
   end
end
