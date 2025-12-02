local lib = {}

local tier1_nonweapon_list, tier1_weapon_list

local function get_not_owned( list )
   local rewards = {}
   for k, r in ipairs(list) do
      if player.outfitNum(r) <= 0 then
         table.insert( rewards, r )
      end
   end
   if #rewards > 0 then
      return rewards
   else
      return list
   end
end

local function tier1_nonweapon ()
   if not tier1_nonweapon_list then
      tier1_nonweapon_list = {
         outfit.get("Emergency Stasis Inducer"),
         outfit.get("Combat Hologram Projector"),
         --outfit.get("Berserk Chip"),
         outfit.get("Hellburner"),
         outfit.get("Nanofiber Structural Enhancement"),
         outfit.get("Biometal Armour"),
      }
   end
   return tier1_nonweapon_list
end
function lib.tier1_nonweapon ()
   local r = get_not_owned( tier1_nonweapon() )
   return r[ rnd.rnd(1,#r) ]
end

local function tier1_weapon ()
   if not tier1_weapon_list then
      tier1_weapon_list = {
         outfit.get("Flak Gun"),
         outfit.get("Neutralizer"),
         outfit.get("Reaver Cannon"),
         outfit.get("Pincushion Battery"),
         outfit.get("Plasma Eruptor"),
      }
   end
   return tier1_weapon_list
end
function lib.tier1_weapon ()
   local r = get_not_owned( tier1_weapon() )
   return r[ rnd.rnd(1,#r) ]
end

function lib.tier1()
   local r = get_not_owned( tmergei( tier1_nonweapon(), tier1_weapon()  ) )
   return r[ rnd.rnd(1,#r) ]
end

return lib
