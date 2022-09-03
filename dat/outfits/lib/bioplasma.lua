-- Need to set effect_name before requiring
-- luacheck: globals effect_name
local fmt = require "format"

local damage, penetration, isturret
function onload( o )
   local s     = o:specificstats()
   damage      = s.damage
   penetration = s.penetration
   isturret    = s.isturret
end

local mapping = {
   ["Corrosion I"] = "corrosion_i",
   ["Paralyzing Plasma"] = "paralyzing",
   ["Crippling Plasma"] = "crippling",
   ["Corrosion II"] = "corrosion_ii",
}
function init( p, _po )
   for k,o in ipairs(p:outfitsList("intrinsic")) do
      local m = mapping[ o:nameRaw() ]
      if m then
         mem[m] = true
      end
   end
end

function descextra( p )
   local dmg = damage
   local dur = 5
   local cor = ""

   if p then
      local map = {}
      for k,o in ipairs(p:outfitsList("intrinsic")) do
         local m = mapping[ o:nameRaw() ]
         if m then
            map[m] = true
         end
      end

      if map.corrosion_ii then
         cor = _(" [Corrosion II]")
         dmg = dmg * 2
         dur = 10
      elseif map.corrosion_i then
         cor = _(" [Corrosion I]")
         dmg = dmg * 1.5
         dur = 7.5
      end
      if map.crippling then -- Learned at 3
         return fmt.f(_("Plasma burns deal an extra {damage:.1f} of damage over {duration} seconds on the target{corrosion}, while reducing the targets turn, speed, and thrust by 25% [Paralyzing Plasma], and lowering fire rate by 20% [Crippling Plasma]."),{damage=dmg, duration=dur, corrosion=cor})
      elseif map.paralyzing then -- Learned at 2
         return fmt.f(_("Plasma burns deal an extra {damage:.1f} of damage over {duration} seconds on the target{corrosion}, while reducing the targets turn, speed, and thrust by 25% [Paralyzing Plasma]."),{damage=dmg, duration=dur, corrosion=cor})
      end
   end

   return fmt.f(_("Plasma burns deal an extra {damage:.1f} of damage over {duration} seconds on the target{corrosion}."),{damage=dmg, duration=dur, corrosion=cor})
end

function onimpact( p, target )
   local ts = target:stats()
   local dmg = damage * (1 - math.min( 1, math.max( 0, ts.absorb - penetration ) ))

   -- Modify by damage
   if p:exists() then
      local mod
      if isturret then
         mod = p:shipstat("tur_damage",true)
      else
         mod = p:shipstat("fwd_damage",true)
      end
      dmg = dmg * mod
   end

   if mem.corrosion_ii then
      target:effectAdd( effect_name, 10, dmg )
      target:effectAdd( "Paralyzing Plasma", 10 )
      target:effectAdd( "Crippling Plasma", 10 )
   elseif mem.corrosion_i then
      target:effectAdd( effect_name, 7.5, dmg )
      if mem.paralyzing then
         target:effectAdd( "Paralyzing Plasma", 7.5 )
      end
      if mem.crippling then
         target:effectAdd( "Crippling Plasma", 7.5 )
      end
   else
      target:effectAdd( effect_name, 5, dmg )
   end
end
