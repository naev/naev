require "ships.lua.zalek"

local fmt = require "format"

local OKTYPES = {
   ["Beam Weapon"] = true,
   ["Fighter Bay"] = true,
}
local BONUSES = {
   weapon_firerate = 200,
   weapon_damage   = -60,
   weapon_energy   = -70,
}

function descextra( _p )
   return fmt.f("#o".._("When only beam and fighter bay weapons are equipped, ship gains +{firerate}% fire rate, {damage}% damage, and {energy}% energy usage for weapons.").."#0", {
      firerate = BONUSES.weapon_firerate,
      damage   = BONUSES.weapon_damage,
      energy   = BONUSES.weapon_energy,
   })
end

function init( p )
   if not p then return end

   local ok = true
   for k,o in ipairs(p:outfitsList("weapon")) do
      local t = o:typeBroad()
      if not OKTYPES[t] then
         ok = false
         break
      end
   end

   -- Add bonuses if ship has only beam / fighter bays
   p:shippropReset()
   if ok then
      p:shippropSet( BONUSES )
   end
end
