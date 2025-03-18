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

   add_desc( _("CPU max"), "", "+380"," _" , "#g" )
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+1760"," _", "#g" )
   add_desc( _("Energy Regeneration"), "", "+35"," _" , "#g" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+500"," _", "#g" )
   add_desc( _("Shield Regeneration"), "", "+9"," _" , "#g" )

   return desc
end

function init(_p, po )
   print(fmt.f("init {po}",{po=po}))
   if po:slot().tags and po:slot().tags.core then
      local cpu_max
      local energy
      local energy_regen
      local shield
      local shield_regen
      if not po:slot().tags.secondary then
         nosec=true
         nomain=false
         cpu_max=380
         energy=1760
         energy_regen=35
         shield=500
         shield_regen=9
      else
         nosec=false
         nomain=true
         cpu_max=0
         energy=0
         energy_regen=0
         shield=0
         shield_regen=0
      end
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
