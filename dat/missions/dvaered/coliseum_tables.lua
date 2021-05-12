wave_score_table = {
   -- Fighters
   Hyena = 100,
   Shark = 200,
   Lancelot = 300,
   Vendetta = 400,
   -- Bombers
   Ancestor = 250,
   -- Corvettes
   Phalanx = 500,
   Admonisher = 600,
   -- Destroyers
   Vigilance = 800,
   Pacifier = 1000,
   ["Dvaered Vigilance"] = 1200,
   -- Cruisers
   Kestrel = 2000,
   Hawking = 3000,
   Goddard = 4000,
   ["Dvaered Goddard"] = 5000,
   -- Carriers
}
wave_round_enemies = {
   light = {
      { "Hyena" }, -- 1
      { "Ancestor" },
      { "Shark" },
      { "Lancelot" },
      { "Vendetta" }, -- 5
      { "Ancestor", "Hyena" },
      { "Shark", "Hyena" },
      { "Lancelot", "Hyena" },
      { "Ancestor", "Hyena", "Hyena" },
      { "Admonisher" }, -- 10
      { "Shark", "Hyena", "Hyena" },
      { "Shark", "Shark" },
      { "Shark", "Lancelot" },
      { "Lancelot", "Lancelot" },
      { "Phalanx", "Ancestor", "Ancestor" }, -- 15
      { "Vendetta", "Lancelot" },
      { "Vendetta", "Vendetta" },
      { "Lancelot", "Shark", "Shark" },
      { "Hyena", "Hyena", "Hyena", "Hyena", "Hyena" },
      { "Pacifier" }, --20
   },
   medium = {
      { "Phalanx" }, -- 1
      { "Phalanx", "Hyena", "Hyena" },
      { "Admonisher" },
      { "Vigilance" },
      { "Pacifier" }, -- 5
      { "Admonisher", "Admonisher" },
      { "Phalanx", "Ancestor", "Ancestor", "Hyena", "Hyena" },
      { "Pacifier", "Lancelot", "Lancelot" },
      { "Lancelot", "Shark", "Shark", "Shark", "Shark", "Shark" },
      { "Kestrel" } -- 10
   },
   heavy = {
      { "Kestrel" }, -- 1
      { "Hawking" },
      { "Kestrel", "Lancelot", "Lancelot" },
      { "Pacifier", "Pacifier", "Admonisher", "Admonisher" },
      { "Goddard" }, -- 5
      { "Hawking", "Admonisher", "Admonisher" },
      { "Pacifier", "Admonisher", "Admonisher", "Ancestor", "Ancestor", "Ancestor", "Lancelot", "Lancelot" },
      { "Kestrel", "Kestrel" },
      { "Hawking", "Pacifier", "Pacifier" },
      { "Dvaered Goddard", "Dvaered Vigilance", "Dvaered Vigilance" },
   }
}

