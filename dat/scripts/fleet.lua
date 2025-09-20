--[[--
Functions for adding fleets of pilots.

@module fleet
--]]
local fleet = {}


local function _buildDupeTable( input, count )
   local tmp = {}
   if type(input) == "table" then
      if #input ~= count then
         warn(_("Tables are different lengths."))
      end
      return input
   else
      for i=1,count do
         tmp[i] = input
      end
      return tmp
   end
end

local function postprocess( pilots )
   if #pilots<=0 then
      return pilots
   end
   local leader = pilots[1]
   for k,p in ipairs(pilots) do
      if k~=1 then
         p:setPos( p:pos() + vec2.newP( rnd.rnd()*75 + 75, rnd.angle() ) )
         p:setLeader( leader )
      end
   end
   return pilots
end

--[[--
Wrapper for pilot.add() that can operate on tables of ships.

The first pilot is set to be the fleet leader.

   @usage pilots = fleet.add( 1, "Pirate Hyena", "Pirate" ) -- Creates a single Pirate Hyena.
   @usage pilots = fleet.add( 1, "Pirate Hyena", "Pirate", nil, nil, {ai="pirate_norun"} ) -- Ditto, but use the "norun" AI variant.
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

   -- Put lone ship into table
   if type(ship) ~= "table" then
      ship = { ship }
   end
   local pilotnames= _buildDupeTable( pilotname, #ship )
   local locations = _buildDupeTable( location,  #ship )
   local factions  = _buildDupeTable( faction,   #ship )
   local out       = {}
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
   return postprocess(out)
end

--[[--
Simplified version of fleet.add where they all share faction, locations, and parameters.

   @tparam {Ship} ships Table of ships (or ship names) to spawn.
   @tparam Faction fct Faction to give the pilots.
   @tparam Vec2|Jump|System|Spob location Location to spawn the pilot. Pilots will jump in from jumps or systems, while they will take off from spobs.
   @tparam table parameters Additional parameters to pass to `pilot.add`.
   @treturn {Pilot} Table containing the pilots spawned.
--]]
function fleet.spawn( ships, fct, location, parameters )
   fct = faction.get(fct)
   local out = {}
   for k,v in ipairs(ships) do
      local p = pilot.add( ships[k], fct, location, parameters )
      table.insert( out, p )
   end
   return postprocess(out)
end

return fleet
