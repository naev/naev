local bhelp = {}

bhelp.ships = {
   pirate = {
      ship.get("Pirate Hyena"),
      ship.get("Pirate Shark"),
      ship.get("Pirate Vendetta"),
      ship.get("Pirate Ancestor"),
      ship.get("Pirate Admonisher"),
      --ship.get("Pirate Revenant"),
      ship.get("Pirate Phalanx"),
      ship.get("Pirate Starbridge"),
      ship.get("Pirate Rhino"),
      ship.get("Pirate Kestrel"),
      --ship.get("Pirate Zebra"),
      --ship.get("Dealbreaker"),
   },
   mercenary = {
      ship.get("Hyena"),
      ship.get("Shark"),
      ship.get("Lancelot"),
      ship.get("Ancestor"),
      ship.get("Admonisher"),
      ship.get("Bedivere"),
      ship.get("Goddard"),
      ship.get("Kestrel"),
      ship.get("Phalanx"),
      ship.get("Pacifier"),
      ship.get("Starbridge"),
      ship.get("Tristan"),
      ship.get("Vendetta"),
      ship.get("Vigilance"),
   },
   empire = {
      ship.get("Empire Shark"),
      ship.get("Empire Lancelot"),
      ship.get("Empire Admonisher"),
      ship.get("Empire Pacifier"),
      ship.get("Empire Hawking"),
   },
}

function bhelp.choose_ships_from_points_and_capship( capship, shiplist, points )
   local maybeship
   if capship then
      local cappoints = capship:points()
      maybeship = {}
      for k,v in ipairs(shiplist) do
         local p = v:points()
         if p < cappoints then
            table.insert( maybeship, v )
         end
      end
   else
      maybeship = shiplist
   end
   table.sort( maybeship, function( a, b ) return a:points() > b:points() end )
   local smallest = maybeship[ #maybeship ]:points()

   local ships = {}
   while points >= smallest do
      local candidates = rnd.permutation( maybeship )
      local s
      local id = 1
      repeat
         s = candidates[id]
         if not s then return ships end
         id = id+1
      until s:points() < points
      table.insert( ships, s )
      points = points - s:points()
   end

   return ships
end

function bhelp.cond_bounty_points( points )
   return function ()
      return (var.peek( "astra_vigilis_points" ) or 0) > points
   end
end

return bhelp
