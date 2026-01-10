-- Need to set effect_name before requiring
local fmt = require "format"

-- luacheck: globals bonus_mod effect_name damage_mod base_duration

bonus_mod = bonus_mod or 1
effect_name = effect_name or "Plasma Burn"
damage_mod = damage_mod or 1
base_duration = base_duration or 5

local damage
local onload_old = onload
function onload( o )
   local s     = o:specificstats()
   damage      = s.damage * damage_mod
   if onload_old then
      onload_old( o )
   end
end

local mapping = {
   ["Corrosion I"] = "corrosion_i",
   ["Paralyzing Plasma"] = "paralyzing",
   ["Crippling Plasma"] = "crippling",
   ["Corrosion II"] = "corrosion_ii",
}
function init( p, _po )
   if not p or not p:exists() then return end
   for k,o in ipairs(p:outfitsList("intrinsic")) do
      local m = mapping[ o:nameRaw() ]
      if m then
         mem[m] = true
      end
   end
end

function descextra( p )
   local dmg = damage
   local dur = base_duration
   local cor = ""

   if p and p:exists() then
      local map = {}
      for k,o in ipairs(p:outfitsList("intrinsic")) do
         local m = mapping[ o:nameRaw() ]
         if m then
            map[m] = true
         end
      end

      if map.corrosion_ii then
         cor = _(" [Corrosion II]")
         dmg = dmg + dmg * bonus_mod
         dur = dur + dur * bonus_mod
      elseif map.corrosion_i then
         cor = _(" [Corrosion I]")
         dmg = dmg + 0.5 * dmg * bonus_mod
         dur = dur + 0.5 * dur * bonus_mod
      end
      dmg = dmg * dur / 5 -- Effect is computed to do 1 damage over 5 seconds, so have to normalize

      if map.crippling then -- Learned at 3
         return "#p"..fmt.f(_("Plasma burns deal an extra {damage:.1f} of damage over {duration} seconds on the target{corrosion}, while reducing the targets turn, speed, and accel by 25% [Paralyzing Plasma], and lowering fire rate by 20% [Crippling Plasma]."),{damage=dmg, duration=dur, corrosion=cor}).."#0"
      elseif map.paralyzing then -- Learned at 2
         return "#p"..fmt.f(_("Plasma burns deal an extra {damage:.1f} of damage over {duration} seconds on the target{corrosion}, while reducing the targets turn, speed, and accel by 25% [Paralyzing Plasma]."),{damage=dmg, duration=dur, corrosion=cor}).."#0"
      end
   end

   return "#p"..fmt.f(_("Plasma burns deal an extra {damage:.1f} of damage over {duration} seconds on the target{corrosion}."),{damage=dmg, duration=dur, corrosion=cor}).."#0"
end

function onimpact( p, target, _pos, _vel, _o, armour, shield )
   local dmg = armour + shield
   local dur = base_duration

   if mem.corrosion_ii then
      dur = dur + dur * bonus_mod
      target:effectAdd( effect_name, dur, dmg, p )
      target:effectAdd( "Paralyzing Plasma", dur, nil, p )
      target:effectAdd( "Crippling Plasma", dur, nil, p )
   elseif mem.corrosion_i then
      dur = dur + 0.5 * dur * bonus_mod
      target:effectAdd( effect_name, dur, dmg, p )
      if mem.paralyzing then
         target:effectAdd( "Paralyzing Plasma", dur, nil, p )
      end
      if mem.crippling then
         target:effectAdd( "Crippling Plasma", dur, nil, p )
      end
   else
      target:effectAdd( effect_name, dur, dmg, p )
   end
end
