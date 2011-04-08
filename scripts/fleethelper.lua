function _buildDupeTable( input, count )
   local tmp = {}
   if type(input) == "table" then
      if #input ~= count then
         print("Warning: Tables are different lengths.")
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
      print("_mergeTables: Error, this function only accepts tables.")
   end

   for k,v in ipairs(new) do
      table.insert(old, v )
   end
   return old
end


-- Randomize the locations of ships in the same manner than pilot.add() does.
function _randomizePositions( ship, override )
   if type(ship) ~= "table" and type(ship) ~= "userdata" then
      print("_randomizePositions: Error, ship list is not a pilot or table of pilots!")
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


-- Wrapper for pilot.add() that can operate on tables of fleets.
function addShips( ship, ai, location, count )
   local ais = {}
   local locations = {}
   local out = {}

   if type(ship) ~= "table" and type(ship) ~= "string" then
      print("addShips: Error, ship list is not a fleet or table of fleets!")
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
   if count > 1 then
      _randomizePositions( out )
   end
   return out
end


-- Wrapper for pilot.addRaw() that can operate on tables of ships.
function addRawShips( ship, ai, location, faction, count )
   local ais = {}
   local locations = {}
   local factions = {}
   local out = {}

   if type(ship) ~= "table" and type(ship) ~= "string" then
      print("addRawShips: Error, ship list is not a ship or table of ships!")
      return
   elseif type(ship) == "string" then -- Put lone ship into table.
      ship = { ship }
   end
   ais       = _buildDupeTable( ai, #ship )
   locations = _buildDupeTable( location, #ship )
   factions  = _buildDupeTable( faction, #ship )
   if factions[1] == nil then
      print("addRawShips: Error, raw ships must have factions!")
      return
   end

   if count == nil then
      count = 1
   end
   for i=1,count do -- Repeat the pattern as necessary.
      for k,v in ipairs(ship) do
         out[k+(i-1)*#ship] = pilot.addRaw( ship[k], ais[k], locations[k], factions[k] )[1]
      end
   end
   if count > 1 then
      _randomizePositions( out )
   end
   return out
end


-- Renames ships with full regex support.
function renameShips( ship, match, replace, limit )
   if type(ship) ~= "table" and type(ship) ~= "userdata" then
      print("renameShips: Error, ship list is not a pilot or table of pilots!")
      return
   elseif type(ship) == "userdata" then -- Put lone pilot into table.
      ship = { ship }
   end

   if match == nil or replace == nil then
      print("renameShips: Error, need a pattern to match and one to replace!")
      return
   end

   for k,v in ipairs(ship) do
      v:rename(string.gsub( tostring(v:name()), match, replace, limit ))
   end
end
