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
   add_desc( _("Ship Mass"), naev.unit("mass"), "+420"," +580", "#r" )
   desc=desc.."#g"
   add_desc( _("CPU max"), "", "+440"," +1310" , "#g" )
   add_desc( _("Energy Capacity"), naev.unit("energy"), "+1860"," +940", "#g" )
   add_desc( _("Energy Regeneration"), "", "+46"," +51" , "#g" )
   add_desc( _("Shield Capacity"), naev.unit("energy"), "+650"," +100", "#g" )
   add_desc( _("Shield Regeneration"), "", "+11"," +2" , "#g" )
   add_desc( _("Detection"), "", "+10"," _" , "#g" )
   add_desc( _("Ship Cooldown Time"), "", "-25"," _" , "#g" )

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
      local ew_detect
      local cooldown_time
      if not po:slot().tags.secondary then
         nosec=true
         nomain=false
         mass=420
         cpu_max=440
         energy=1860
         energy_regen=46
         shield=650
         shield_regen=11
         ew_detect=10
         cooldown_time=-25
      else
         nosec=false
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
   else
      nosec=false
      nomain=false
   end
end
