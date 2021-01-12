--[[

   Rescue script for stranded pilots.

   The basic structure is as follows:

      If the player lacks any core outfits, either:

         a) The ship's defaults are added to the empty slots
         b) All cores are replaced with the ship's defaults

      If that fails, then the player is presented with various options:

         a) Remove all non-cores
         b) Remove weapons, utility, or structure outfits separately
         b) Replace cores with defaults

      There is no failure mode beyond this, as a ship that isn't spaceworthy
      in its default state already generates warnings on load.

   This script has two known flaws at the moment. Players aren't considered
   stranded if the outfitter sells outfits that can fill all missing slots,
   or replace existing cores. This is done without regard to affordability.

   The second is that some conditions (e.g. free cargo space) are independent
   of outfits, and because we don't explicitly check them, fall-through to the
   final "this is a bug, please report" happens more than it ought to.

--]]

required = {} -- Slots that must be filled in order to take off.
equipped = {} -- Outfits equipped in required slots.
missing  = {} -- Array of empty required slots and their default outfits.

nrequired = 0
nequipped = 0

-- Array of tasks for filling in missing slots in the following format:
--    { { desc, callback }, ... }
mtasks = {
   { _("Add missing default cores"), function() fillMissing( missing ) end },
   { _("Replace all cores with defaults"), function() equipDefaults( required ) end },
   { _("Cancel"), function() tk.msg( msg_title, msg_refuse ) end }
   -- TODO: Possibly add a "Select from inventory" option
}

tasks = {
   { _("Replace all cores with defaults"), function() equipDefaults( required ) end },
   { _("Remove all non-core outfits"), function() removeNonCores() end },
   { _("Remove weapons"), function() removeNonCores( "Weapon" ) end },
   { _("Remove utility outfits"), function() removeNonCores( "Utility" ) end },
   { _("Remove structure outfits"), function() removeNonCores( "Structure" ) end },
   { _("Cancel"), function() tk.msg( msg_title, msg_refuse ) end }
}

msg_title = _("Stranded")

msg_prompt = _([[The following actions are available to make your ship spaceworthy:]])

msg_failure = _([[Unfortunately, your ship still isn't spaceworthy. However, there are still other options for getting your ship airborne.]])

msg_failure_final = _([[Well... this isn't good. Your ship has been restored to its default configuration, yet still isn't spaceworthy.

Please report this to the developers along with a copy of your save file.]])

msg_success_generic = _([[Your ship is now spaceworthy, though you should get to an outfitter as soon as possible.]])

msg_refuse = _([[Very well, but it's unlikely you'll be able to take off.

If you can't find a way to make your ship spaceworthy, you can always attempt to take off again to trigger this dialogue, or try loading your backup save.]])


function rescue()
   -- Do nothing if already spaceworthy.
   if player.pilot():spaceworthy() then
      return
   end

   buildTables()

   -- Bail out if the player has a reasonable chance of fixing their ship.
   if not check_stranded( missing ) then
      return
   end

   -- Add missing core outfits.
   if #mtasks > 0 and #missing > 0 then
      local defaults, weapons, utility, structure = assessOutfits()

      -- Build list of suitable tasks.
      local opts = {}
      for k,v in ipairs(mtasks) do
         table.insert(opts, v)
      end

      if #missing == nrequired or defaults then
         table.remove( opts, 2 )
      end

      local strings = {}
      for k,v in ipairs(opts) do
         strings[k] = v[1]
      end


      local msg_missing = gettext.ngettext(
         [[Your ship is missing %d core outfit. How do you want to resolve this?]],
         [[Your ship is missing %d core outfits. How do you want to resolve this?]], #missing)
      local ind, str = tk.choice( msg_title,
            string.format(msg_missing, #missing), table.unpack(strings) )

      opts[ind][2]() -- Run callback.
      if str == _("Cancel") then
         return
      end

      -- If ship is now spaceworthy, bail out.
      if player.pilot():spaceworthy() then
         local msg_success = gettext.ngettext(
            [[After adding the missing outfit, your ship is now spaceworthy, though it may have somewhat lower performance than usual. You should get to a planet with a proper shipyard and outfitter.]],
            [[After adding the missing outfits, your ship is now spaceworthy, though it may have somewhat lower performance than usual. You should get to a planet with a proper shipyard and outfitter.]], #missing)
         tk.msg(msg_title, msg_success)
         return
      end

      tk.msg( msg_title, msg_failure )
      buildTables() -- Rebuild tables, as we've added outfits.
   end

   -- Remove non-cores, replace cores with defaults, etc.
   while true do
      local defaults, weapons, utility, structure = assessOutfits()

      -- Build list of suitable tasks.
      local opts = {}
      if not defaults then table.insert( opts, tasks[1] ) end

      if weapons or utility or structure then
         table.insert( opts, tasks[2] )
      end

      if weapons then table.insert( opts, tasks[3] ) end
      if utility then table.insert( opts, tasks[4] ) end
      if structure then table.insert( opts, tasks[5] ) end

      table.insert( opts, tasks[6] ) -- "Cancel"

      local strings = {}
      for k,v in ipairs(opts) do
         strings[k] = v[1]
      end

      -- Only "Cancel" remains, nothing to do.
      if #strings < 2 then
         tk.msg(msg_title, msg_failure_final)
         return
      end

      local ind, str = tk.choice( msg_title, msg_prompt, table.unpack(strings) )

      opts[ind][2]() -- Run callback.
      if str == _("Cancel") then
         return
      end

      if player.pilot():spaceworthy() then
         tk.msg( msg_title, msg_success_generic )
         return
      end

      buildTables() -- Rebuild tables, as we've added outfits.
   end
end


--[[

   Determines whether the player is stranded.

   In the simplest case, a player is stranded if their ship isn't spaceworthy
   and the planet has no outfitter or shipyard.

   If the player isn't missing any core outfits, and has non-core outfits
   equipped, we let them resolve the issue manually.

   The last simple case is: If the planet has a shipyard that sells ships the
   player can afford, the player is not considered stranded.

   From there, it gets decidedly more complex. An outfit pool is created,
   consisting of the player's inventory, plus:

      a) If the planet has a shipyard, the player's other ships' outfits
      b) If the planet has an outfitter, the outfitter's stock

   At this point, the player is only stranded under two circumstances:

      1) They're missing core outfits and nothing in the pool fits.
      2) The ship is bare aside from core outfits, yet isn't spaceworthy
            and no other suitable core outfits are available.

--]]
function check_stranded( missing )
   local pp = player.pilot()
   local services = planet.cur():services()

   -- Player has no ability to fix whatever's wrong, definitely stuck.
   if not services["shipyard"] and not services["outfits"] then
      return true
   end

   if services["shipyard"] then
      -- Get value of player's ship.
      local _, playervalue = pp:ship():price()
      for k,v in ipairs( pp:outfits() ) do
         playervalue = playervalue + v:price()
      end

      -- If the player can afford another ship, they're not stranded.
      for k,v in ipairs( planet.cur():shipsSold() ) do
         if v:price() <= (player.credits() + playervalue) then
            return false
         end
      end
   end

   -- When all cores are equipped along with non-cores, it's nearly impossible
   -- to determine where the problem lies, so let the player sort it out.
   if #missing == 0 then
      for k,v in ipairs( pp:outfits() ) do
         _, _, prop = v:slot()
         if not prop then
            return false
         end
      end
   end

   local inv = player.outfits()

   -- Add the player's other ships' outfits.
   if services["shipyard"] then
      for _, s in ipairs( player.ships() ) do
         if s ~= pp:name() then
            local outfits = player.shipOutfits(s)
            for _, o in ipairs(outfits) do
               table.insert(inv, o)
            end
         end
      end
   end

   -- Add outfits sold by the outfitter.
   if services["outfits"] then
      for k,v in ipairs( planet.cur():outfitsSold() ) do
         table.insert(inv, v)
      end
   end

   -- Generate a deduplicated list.
   table.sort( inv, function(a, b) return a:nameRaw() < b:nameRaw() end )

   local last  = nil
   local found = 0
   for _,o in ipairs(inv) do
      if #missing <= 0 then
         break
      end

      -- Iterate to find outfits matching the size and property of the empty
      -- core outfit slots.
      if last and o ~= last then
         _, osize, oprop = o:slot()
         for k,r in ipairs(missing) do
            if osize == r.size and oprop == r.property then
               table.remove(missing, k)
               found = found + 1
               break
            end
         end
      end
      last = o
   end

   -- Nothing to fill the slots could be found.
   if #missing > 0 or found == 0 then
      return true
   end

   return false
end


-- Builds the tables of required, equipped, and missing outfits.
function buildTables()
   local slots   = player.pilot():ship():getSlots()
   local outfits = player.pilot():outfits()

   -- Clear tables.
   required = {}
   equipped = {}
   missing  = {}

   nrequired = 0
   nequipped = 0

   -- Find the number of required cores, and their default outfits.
   for k,v in pairs(slots) do
      if v.property then
         required[ v.property ] = v
         nrequired = nrequired + 1
      end
   end

   -- Find equipped cores.
   for k,v in pairs(outfits) do
      _, _, prop = v:slot()
      if prop then
         equipped[ prop ] = v
         nequipped = nequipped + 1
      end
   end

   -- Build the table of missing cores.
   for k,v in pairs(required) do
      if not equipped[k] then
         table.insert(missing, v)
      end
   end
end


-- Builds a table of outfits suitable for putting into a slot of a given
-- size and property.
function buildOutfitTable( size, property, outfits )
   local out = {}

   for k,v in ipairs( outfits ) do
      local _, osize, oprop = v:slot()
      if size == osize and property == oprop then
         table.insert(out, v:nameRaw())
      end
   end

   return out
end


-- Removes all non-core outfits from the ship and adds them to the inventory.
function removeNonCores( slottype )
   local pp = player.pilot() -- Convenience.

   for k,v in pairs( pp:outfits() ) do
      local slot, _, prop = v:slot()
      if not prop and (not slottype or slot == slottype) then
         -- Store and remove old
         player.addOutfit(v:nameRaw())
         pp:rmOutfit(v:nameRaw())
      end
   end
end


-- Replace all cores with the ship's defaults.
function equipDefaults( defaults )
   local pp = player.pilot() -- Convenience.

   for k,v in ipairs( pp:outfits() ) do
      local _, _, prop = v:slot()

      -- Remove if required but not default.
      if prop and v ~= defaults[prop].outfit then
         -- Store and remove old
         player.addOutfit(v:nameRaw())
         pp:rmOutfit(v:nameRaw())

         -- Add new
         pp:addOutfit( defaults[k].outfit:nameRaw() )

         -- Remove the outfit from the to-add list.
         defaults[k] = nil
      end
   end

   -- Add remaining outfits.
   fillMissing( defaults )
end


function fillMissing( missing )
   -- Fill empty core slots with defaults.
   for k,v in pairs(missing) do
      player.pilot():addOutfit( v.outfit:nameRaw() )
   end
end


function assessOutfits()
   local defaults  = true  -- Equipped cores are the same as defaults.
   local weapons   = false -- Ship has weapons.
   local utility   = false -- Ship has utility outfits.
   local structure = false -- Ship has structure outfits.

   for k,v in pairs(equipped) do
      local _, _, prop = v:slot()
      if required[prop] and v ~= required[prop].outfit then
         defaults = false
         break
      end
   end

   for _,o in ipairs( player.pilot():outfits() ) do
      local s, _, prop = o:slot()
      if not prop and s == "Weapon" then
         weapons = true
      elseif not prop and s == "Utility" then
         utility = true
      elseif not prop and s == "Structure" then
         structure = true
      end
   end

   return defaults, weapons, utility, structure
end
