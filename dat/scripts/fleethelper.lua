--[[
-- @brief Wrapper for pilot.add() that can operate on tables of fleets.
--
-- @usage pilots = addShips( "Pirate Hyena", "pirate", nil, 2 ) -- Creates two Pirate Hyenas with pirate AIs.
-- @usage pilots = addShips( { "Trader Rhino", "Trader Koala" }, nil, nil, 2 ) -- Creates a convoy of four trader ships with default AIs.
--
--    @luaparam ship Fleet to add.
--    @luaparam ai AI to override the default with.
--    @luaparam location Location to jump in from, take off from, or appear at.
--    @luaparam count Number of times to repeat the pattern.
--    @luareturn Table of created pilots.
-- @luafunc addShips( fleet, ai, location, count )
--]]
function addShips( ship, ai, location, count )
   local ais = {}
   local locations = {}
   local out = {}

   if type(ship) ~= "table" and type(ship) ~= "string" then
      print(_("addShips: Error, ship list is not a fleet or table of fleets!"))
      return
   elseif type(ship) == "string" then -- Put lone fleet into table.
      ship = { ship }
   end
   ais       = _buildDupeTable( ai, #ship )
   locations = _buildDupeTable( location, #ship )

   if count == nil then
      count = 1
   end
   for i=1,count do -- Repeat the pattern as necessary.
      for k,v in ipairs(ship) do
         out = _mergeTables( out, pilot.add( ship[k], ais[k], locations[k] ) )
      end
   end
   if #out > 1 then
      _randomizePositions( out )
   end
   return out
end


--[[
-- @brief Wrapper for pilot.addRaw() that can operate on tables of ships.
--
-- @usage pilots = addRawShips( "Hyena", "pirate", nil, "Pirate" ) -- Creates a facsimile of a Pirate Hyena.
-- @usage pilots = addRawShips( { "Rhino", "Koala" }, nil, nil, "Trader", 2 ) -- Creates four Trader ships.
--
--    @luaparam ship Ship to add.
--    @luaparam ai AI to give the pilot.
--    @luaparam location Location to jump in from, take off from, or appear at.
--    @luaparam faction Faction to give the pilot.
--    @luaparam count Number of times to repeat the pattern.
--    @luareturn Table of created pilots.
-- @luafunc addRawShips( ship, ai, location, faction, count )
--]]
function addRawShips( ship, ai, location, faction, count )
   local ais = {}
   local locations = {}
   local factions = {}
   local out = {}

   if type(ship) ~= "table" and type(ship) ~= "string" then
      print(_("addRawShips: Error, ship list is not a ship or table of ships!"))
      return
   elseif type(ship) == "string" then -- Put lone ship into table.
      ship = { ship }
   end
   ais       = _buildDupeTable( ai, #ship )
   locations = _buildDupeTable( location, #ship )
   factions  = _buildDupeTable( faction, #ship )
   if factions[1] == nil then
      print(_("addRawShips: Error, raw ships must have factions!"))
      return
   end

   if count == nil then
      count = 1
   end
   for i=1,count do -- Repeat the pattern as necessary.
      for k,v in ipairs(ship) do
         out[k+(i-1)*#ship] = pilot.addRaw( ship[k], ais[k], locations[k], factions[k] )
      end
   end
   if #out > 1 then
      _randomizePositions( out )
   end
   return out
end


--[[
-- @brief Renames ships (or tables of ships) with full regex support.
--
-- @usage renameShips( pilots, "Trader", "" ) -- Removes "Trader" prefix, if present.
--
--    @luaparam ship Ship(s) to modify.
--    @luaparam match Pattern to match.
--    @luaparam replace Pattern to replace matches with.
--    @luaparam limit Maximum number of times to replace.
-- @luafunc renameShips( ship, match, replace, limit )
--]]
function renameShips( ship, match, replace, limit )
   if type(ship) ~= "table" and type(ship) ~= "userdata" then
      print(_("renameShips: Error, ship list is not a pilot or table of pilots!"))
      return
   elseif type(ship) == "userdata" then -- Put lone pilot into table.
      ship = { ship }
   end

   if match == nil or replace == nil then
      print(_("renameShips: Error, need a pattern to match and one to replace!"))
      return
   end

   for k,v in ipairs(ship) do
      v:rename(string.gsub( tostring(v:name()), match, replace, limit ))
   end
end

function _buildDupeTable( input, count )
   local tmp = {}
   if type(input) == "table" then
      if #input ~= count then
         print(_("Warning: Tables are different lengths."))
      end
      return input
   else
      for i=1,count do
         tmp[i] = input
      end
      return tmp
   end
end


function _mergeTables( old, new )
   if type(old) ~= "table" or type(new) ~= "table" then
      print(_("_mergeTables: Error, this function only accepts tables."))
   end

   for k,v in ipairs(new) do
      table.insert(old, v )
   end
   return old
end


-- Randomize the locations of ships in the same manner than pilot.add() does.
function _randomizePositions( ship, override )
   if type(ship) ~= "table" and type(ship) ~= "userdata" then
      print(_("_randomizePositions: Error, ship list is not a pilot or table of pilots!"))
      return
   elseif type(ship) == "userdata" then -- Put lone pilot into table.
      ship = { ship }
   end

   local x = 0
   local y = 0
   for k,v in ipairs(ship) do
      if k ~= 1 and not override then
         if vec2.dist( ship[1]:pos(), v:pos() ) == 0 then
            x = x + rnd.rnd(75,150) * (rnd.rnd(0,1) - 0.5) * 2
            y = y + rnd.rnd(75,150) * (rnd.rnd(0,1) - 0.5) * 2
            v:setPos( v:pos() + vec2.new( x, y ) )
         end
      end
   end
end
