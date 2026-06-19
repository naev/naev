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
      ship.get("Goddard"),
   },
   empire = {
      ship.get("Empire Shark"),
      ship.get("Empire Lancelot"),
      ship.get("Empire Admonisher"),
      ship.get("Empire Pacifier"),
      ship.get("Empire Hawking"),
      ship.get("Empire Peacemaker"),
   },
   flf = {
      ship.get("Tristan"),
      ship.get("Vendetta"),
      ship.get("Bedivere"),
      ship.get("Pacifier"),
      ship.get("Clydesdale"), -- Specially added here
   },
   dvaered = {
      ship.get("Dvaered Vendetta"),
      ship.get("Dvaered Ancestor"),
      ship.get("Dvaered Phalanx"),
      ship.get("Dvaered Vigilance"),
      ship.get("Dvaered Retribution"),
      ship.get("Dvaered Goddard"),
   },
   sirius = {
      ship.get("Sirius Fidelity"),
      ship.get("Sirius Preacher"),
      ship.get("Sirius Shaman"),
      ship.get("Sirius Dogma"),
      ship.get("Sirius Divinity"),
   },
   zalek = {
      ship.get("Za'lek Light Drone"),
      ship.get("Za'lek Heavy Drone"),
      ship.get("Za'lek Bomber Drone"),
      ship.get("Za'lek Sting"),
      ship.get("Za'lek Demon"),
      ship.get("Za'lek Diablo"),
      ship.get("Za'lek Mephisto"),
   },
   soromid = {
      ship.get("Soromid Brigand"),
      ship.get("Soromid Reaver"),
      ship.get("Soromid Marauder"),
      ship.get("Soromid Odium"),
      ship.get("Soromid Nyx"),
      ship.get("Soromid Ira"),
      ship.get("Soromid Arx"),
   },
   proteron = {
      ship.get("Proteron Euler"),
      ship.get("Proteron Dalton"),
      ship.get("Proteron Hippocrates"),
      ship.get("Proteron Gauss"),
      ship.get("Proteron Pythagoras"),
      ship.get("Proteron Archimedes"),
      ship.get("Proteron Watson"),
   },
   collective = {
      ship.get("Drone"),
      ship.get("Heavy Drone"),
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
