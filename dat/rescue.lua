--[[

   Rescue script for stranded pilots.

   The basic structure is as follows:

      1) If the player lacks any core outfits, the ship's defaults are added.
      2) If that's insufficient, all non-core outfits are removed.
      3) Finally, if nothing else works, the current cores are removed and
      replaced with the stock ones.

      There is no failure mode beyond this, as a ship that isn't spaceworthy
      in its default state already generates warnings on load.

--]]

lang = naev.lang()
if lang == "es" then
   -- Break everything. Why do we do this?
else -- default english
   msg_title = "Stranded"

   msg_missing = [[It appears that your ship is missing %d core outfit%s. Would you like to add the following missing outfit%s?

      %s]]

   msg_success = [[After adding the missing outfit%s, your ship is now spaceworthy, though it may have somewhat lower performance than usual. You should get to a planet with a proper shipyard and outfitter.]]

   -- Used when a missing outfit isn't the entry point to this script.
   msg_nomissing = [[It appears that your ship isn't spaceworthy. This is likely due to the combination of non-core outfits your ship has. Would you like to remove them and put them in storage?]]

   msg_failure = [[Unfortunately, your ship still isn't spaceworthy, likely due to the combination of non-core outfits your ship has. Would you like to remove them and put them in storage?]]

   msg_success2 = [[After removing all non-core outfits, your ship is now spaceworthy, though defenseless. You should get to a planet with a proper shipyard and outfitter as soon as possible.]]

   msg_failure2 = [[Well, that didn't work. The only remaining option is to strip your current core outfits and replace them with the ship's defaults. Do you want to try that?]]

   msg_success3 = [[There we go. Your ship should now be able to take off. Keep in mind that it's now unarmed and likely quite slow for its size. You should get to an outfitter as soon as possible.]]

   msg_failure3 = [[Well... this isn't good. Your ship has been restored to its default configuration, yet still isn't spaceworthy.

   Please report this to the developers along with a copy of your save file.]]

   msg_refuse = [[Very well, but it's unlikely you'll be able to take off.

   If you can't find a way to make your ship spaceworthy, you can always attempt to take off again to trigger this dialogue, or try loading your backup save.]]
end

function rescue()
   slots   = player.pilot():ship():getSlots()
   outfits = player.pilot():outfits()

   -- Find the number of required cores, and their default outfits.
   required  = {}
   nrequired = 0
   for k,v in pairs(slots) do
      if v.property then
         required[ v.property ] = v.default
         nrequired = nrequired + 1
      end
   end

   -- Find equipped cores.
   equipped  = {}
   nequipped = 0
   for k,v in pairs(outfits) do
      _, _, prop = v:slot()
      if prop then
         equipped[ prop ] = v:name()
         nequipped = nequipped + 1
      end
   end

   missing = {}
   for k,v in pairs(required) do
      if not equipped[k] then
         table.insert(missing, v)
      end
   end

   if #missing > 0 then
      suffix = "s"
      if #missing == 1 then
         suffix = ""
      end
         
      if not tk.yesno(msg_title, string.format(msg_missing, #missing,
            suffix, suffix, table.concat(missing, "\n   "))) then
         tk.msg(msg_title, msg_refuse)
         return
      end

      -- Fill empty core slots with defaults.
      for k,v in pairs(missing) do
         player.pilot():addOutfit( v )
      end

      if player.pilot():spaceworthy() then
         tk.msg(msg_title, string.format(msg_success, suffix))
         return
      elseif not tk.yesno(msg_title, msg_failure) then
         tk.msg(msg_title, msg_refuse)
         return
      end
   elseif not tk.yesno(msg_title, msg_nomissing) then
      tk.msg(msg_title, msg_refuse)
      return
   end

   -- Remove all non-core outfits from the ship and add them to the inventory.
   for k,v in pairs(outfits) do
      _, _, prop = v:slot()
      if not prop then
         name = v:name()
         player.addOutfit(name)
         player.pilot():rmOutfit(name)
      end
   end

   if player.pilot():spaceworthy() then
      tk.msg(msg_title, msg_success2)
      return
   elseif not tk.yesno(msg_title, msg_failure2) then
      tk.msg(msg_title, msg_refuse)
      return
   end

   -- Remove all non-default cores from the ship and add them to the inventory.
   for k,v in pairs(equipped) do
      -- Store and remove old
      player.addOutfit(v)
      player.pilot():rmOutfit(v)

      -- Add new
      player.pilot():addOutfit( required[k] )
      print( required[k] )
   end

   if player.pilot():spaceworthy() then
      tk.msg(msg_title, msg_success3)
   else
      tk.msg(msg_title, msg_failure3)
   end
end
