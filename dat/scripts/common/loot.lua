local lib = {}

local tier1_nonweapon, tier1_weapon

function lib.tier1_nonweapon ()
   if not tier1_nonweapon then
      tier1_nonweapon = {
         outfit.get("Emergency Stasis Inducer"),
         outfit.get("Combat Hologram Projector"),
         outfit.get("Berserk Chip"),
         outfit.get("Hellburner"),
         outfit.get("Nanofiber Structural Enhancement"),
         outfit.get("Biometal Armour"),
      }
   end
   return tier1_nonweapon[ rnd.rnd(1,#tier1_nonweapon) ]
end

function lib.tier1_weapon ()
   if not tier1_weapon then
      tier1_weapon = {
         outfit.get("Flak Gun"),
         outfit.get("Neutralizer"),
         outfit.get("Reaver Cannon"),
         outfit.get("Pincushion Battery"),
         outfit.get("Plasma Eruptor"),
      }
   end
   return tier1_weapon[ rnd.rnd(1,#tier1_nonweapon) ]
end

function lib.tier1()
   if rnd.rnd() < 0.5 then
      return lib.tier1_nonweapon()
   end
   return lib.tier1_weapon()
end

return lib
