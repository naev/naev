--[[
-- @brief Wrapper for pilot.add() that can operate on tables of ships.
--
-- @usage pilots = addShips( 1, "Hyena", "Pirate" ) -- Creates a facsimile of a Pirate Hyena.
-- @usage pilots = addShips( 1, "Hyena", "Pirate", nil, nil, "pirate_norun" ) -- Ditto, but use the "norun" AI variant.
-- @usage pilots = addShips( 2, { "Rhino", "Koala" }, "Trader" ) -- Creates four Trader ships.
--
--    @luaparam count Number of times to repeat the pattern.
--    @luaparam ship Ship to add.
--    @luaparam faction Faction to give the pilot.
--    @luaparam location Location to jump in from, take off from, or appear at.
--    @luaparam pilotname Name to give the pilot.
--    @luaparam ai AI to give the pilot.
--    @luareturn Table of created pilots.
-- @luafunc addShips
--]]
function addShips( count, ship, faction, location, pilotname, ai )
   local pilotnames = {}
   local ais = {}
   local locations = {}
   local factions = {}
   local out = {}

   if type(ship) ~= "table" and type(ship) ~= "string" then
      print(_("addShips: Error, ship list is not a ship or table of ships!"))
      return
   elseif type(ship) == "string" then -- Put lone ship into table.
      ship = { ship }
   end
   pilotnames= _buildDupeTable( pilotname, #ship )
   ais       = _buildDupeTable( ai, #ship )
   locations = _buildDupeTable( location, #ship )
   factions  = _buildDupeTable( faction, #ship )
   if factions[1] == nil then
      print(_("addShips: Error, raw ships must have factions!"))
      return
   end

   if count == nil then
      count = 1
   end
   for i=1,count do -- Repeat the pattern as necessary.
      for k,v in ipairs(ship) do
         out[k+(i-1)*#ship] = pilot.add( ship[k], factions[k], locations[k], pilotnames[k], ais[k] )
      end
   end
   if #out > 1 then
      _randomizePositions( out )
   end
   return out
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


-- Randomize the locations of ships in the same manner than pilot.addFleet() does.
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
