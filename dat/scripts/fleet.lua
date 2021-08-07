--[[--
Functions for adding fleets of pilots.

@module fleet
--]]
local fleet = {}


local function _buildDupeTable( input, count )
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


-- Randomly stagger the locations of ships so they don't all spawn on top of each other.
local function _randomizePositions( ship )
   for k,v in ipairs(ship) do
      if k~=1 then
         v:setPos( v:pos() + vec2.newP( rnd.rnd()*75 + 75, rnd.rnd() * 360 ) )
      end
   end
end


--[[--
Wrapper for pilot.add() that can operate on tables of ships.

   @usage pilots = fleet.add( 1, "Hyena", "Pirate" ) -- Creates a facsimile of a Pirate Hyena.
   @usage pilots = fleet.add( 1, "Hyena", "Pirate", nil, nil, {ai="pirate_norun"} ) -- Ditto, but use the "norun" AI variant.
   @usage pilots = fleet.add( 2, { "Rhino", "Koala" }, "Trader" ) -- Creates four Trader ships.

      @param[opt=1] count Number of times to repeat the pattern.
      @param ship Ship(s) to add.
      @param faction Faction(s) to give the pilot.
      @param location Location(s) to jump in from, take off from, or appear at.
      @param pilotname Name(s) to give each pilot.
      @param parameters Common table of extra parameters to pass pilot.add(), e.g. {ai="escort"}.
      @return Table of created pilots.

      <em>TODO</em>: With a little work we can support a table of parameters tables, but no one even wants that. (Yet?)
--]]
function fleet.add( count, ship, faction, location, pilotname, parameters )
   count = count or 1

   local pilotnames  = {}
   local locations   = {}
   local factions    = {}
   local out         = {}

   -- Put lone ship into table
   if type(ship) ~= "table" then
      ship = { ship }
   end
   pilotnames= _buildDupeTable( pilotname, #ship )
   locations = _buildDupeTable( location,  #ship )
   factions  = _buildDupeTable( faction,   #ship )
   if factions[1] == nil then
      print(_("fleet.add: Error, raw ships must have factions!"))
      return
   end

   for i=1,count do -- Repeat the pattern as necessary.
      for k,v in ipairs(ship) do
         local p = pilot.add( ship[k], factions[k], locations[k], pilotnames[k], parameters )
         table.insert( out, p )
      end
   end
   if #out > 1 then
      _randomizePositions( out )
   end
   return out
end


return fleet
