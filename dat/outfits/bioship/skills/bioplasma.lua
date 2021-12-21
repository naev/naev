local damage
function onload( o )
   local s = o:specificstats()
   damage = s.damage
end

local mapping = {
   ["Corrosion I"] = "corrosion_i",
   ["Paralyzing Plasma"] = "paralyzing",
   ["Crippling Plasma"] = "crippling",
   ["Corrosion II"] = "corrosion_ii",
}
function init( p, _po )
   for k,o in ipairs(p:outfits("intrinsic")) do
      local m = mapping[ o:nameRaw() ]
      if m then
         mem[m] = true
      end
   end
end

function onhit( _p, _po, target )
   if mem.corrosion_ii then
      target:effectAdd( "Plasma Burn I", 10, damage )
      target:effectAdd( "Paralyzing Plasma", 10 )
      target:effectAdd( "Crippling Plasma", 10 )
   elseif mem.corrosion_i then
      target:effectAdd( "Plasma Burn I", 7.5, damage )
      if mem.paralyzing then
         target:effectAdd( "Paralyzing Plasma", 7.5 )
      end
      if mem.crippling then
         target:effectAdd( "Crippling Plasma", 7.5 )
      end
   else
      target:effectAdd( "Plasma Burn I", 5, damage )
   end
end
