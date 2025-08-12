local equipopt = require 'equipopt'

local tables = {}

tables.wave_score = {
   -- Fighters
   Hyena = 1000,
   Shark = 2000,
   Lancelot = 3000,
   Vendetta = 4000,
   -- Bombers
   Ancestor = 2500,
   -- Corvettes
   Phalanx = 6000,
   Admonisher = 8000,
   -- Destroyers
   Vigilance = 12000,
   Pacifier = 14000,
   ["Dvaered Vigilance"] = 14000,
   -- Cruisers
   Kestrel = 20000,
   Hawking = 30000,
   Goddard = 40000,
   ["Dvaered Goddard"] = 45000,
   -- Carriers
}
tables.wave_round_enemies = {
   independent = {
      skirmisher = {
         { "Hyena" }, -- 1
         { "Shark" },
         { "Ancestor" },
         { "Vendetta" },
         { "Lancelot" }, -- 5
         { "Shark", "Hyena" },
         { layout="pincer"; "Ancestor", "Hyena" },
         { "Lancelot", "Shark" },
         { layout="circle"; "Ancestor", "Shark" },
         { "Admonisher" }, -- 10
         { layout="circle"; "Shark", "Hyena", "Hyena" },
         { layout="pincer"; "Vendetta", "Shark", "Hyena" },
         { "Ancestor", "Lancelot", "Hyena" },
         { layout="pincer"; "Lancelot", "Lancelot", "Shark" },
         { layout="pincer"; "Phalanx", "Ancestor", "Ancestor" }, -- 15
         { layout="pincer"; "Vendetta", "Lancelot" },
         { layout="pincer"; "Vendetta", "Vendetta", "Vendetta" },
         { layout="circle"; "Lancelot", "Lancelot", "Lancelot" },
         { layout="circle"; "Hyena", "Hyena", "Hyena", "Hyena", "Hyena" },
         { "Pacifier" }, --20
      },
      warrior = {
         { "Phalanx" }, -- 1
         { layout="circle"; "Phalanx", "Shark", "Shark" },
         { "Admonisher" },
         { "Vigilance", "Ancestor" },
         { "Pacifier" }, -- 5
         { layout="pincer"; "Admonisher", "Admonisher" },
         { layout="circle"; "Phalanx", "Ancestor", "Ancestor", "Hyena", "Hyena" },
         { layout="circle"; "Pacifier", "Lancelot", "Lancelot" },
         { layout="circle"; "Vigilance", "Shark", "Shark", "Shark", "Shark", "Shark" },
         { "Kestrel" } -- 10
      },
      warlord = {
         --[[
         equip = function( p )
            equipopt.generic( p, nil, "elite" )
         end;
         --]]
         { "Kestrel" }, -- 1
         { "Hawking" },
         { "Kestrel", "Lancelot", "Lancelot" },
         { layout="pincer"; "Pacifier", "Pacifier", "Admonisher", "Admonisher" },
         { "Goddard" }, -- 5
         { layout="pincer"; "Hawking", "Admonisher", "Admonisher" },
         { layout="circle"; "Pacifier", "Admonisher", "Admonisher", "Ancestor", "Ancestor", "Ancestor", "Lancelot", "Lancelot" },
         { "Kestrel", "Kestrel" },
         { "Hawking", "Pacifier", "Pacifier", "Ancestor", "Ancestor" },
         { func = function( shipname, fct, pos, id )
            local name
            if id==1 then -- Indicates boss or main enemy (note that in the case of double enemies there may be another)
               name = _("Lady Sainte-Beuverie")
            end
            -- This is the same as the main script, except we equip them as Dvaered
            local p = pilot.add( shipname, fct, pos, name, {ai="baddie_norun", naked=true} )
            equipopt.dvaered( p )
            return p
         end, "Dvaered Goddard", "Dvaered Vigilance", "Dvaered Vigilance" },
      },
   }
}

return tables
