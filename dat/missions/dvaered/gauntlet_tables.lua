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
      { layout="pincer"; "Shark", "Hyena" },
      { "Lancelot", "Hyena" },
      { layout="circle"; "Ancestor", "Hyena", "Hyena" },
      { "Admonisher" }, -- 10
      { layout="circle"; "Shark", "Hyena", "Hyena" },
      { layout="pincer"; "Shark", "Shark" },
      { "Shark", "Lancelot" },
      { layout="pincer"; "Lancelot", "Lancelot" },
      { layout="pincer"; "Phalanx", "Ancestor", "Ancestor" }, -- 15
      { layout="pincer"; "Vendetta", "Lancelot" },
      { layout="pincer"; "Vendetta", "Vendetta" },
      { layout="circle"; "Lancelot", "Shark", "Shark" },
      { layout="circle"; "Hyena", "Hyena", "Hyena", "Hyena", "Hyena" },
      { "Pacifier" }, --20
   },
   medium = {
      { "Phalanx" }, -- 1
      { layout="circle"; "Phalanx", "Hyena", "Hyena" },
      { "Admonisher" },
      { "Vigilance" },
      { "Pacifier" }, -- 5
      { layout="pincer"; "Admonisher", "Admonisher" },
      { layout="circle"; "Phalanx", "Ancestor", "Ancestor", "Hyena", "Hyena" },
      { layout="circle"; "Pacifier", "Lancelot", "Lancelot" },
      { layout="circle"; "Lancelot", "Shark", "Shark", "Shark", "Shark", "Shark" },
      { "Kestrel" } -- 10
   },
   heavy = {
      { "Kestrel" }, -- 1
      { "Hawking" },
      { "Kestrel", "Lancelot", "Lancelot" },
      { layout="pincer"; "Pacifier", "Pacifier", "Admonisher", "Admonisher" },
      { "Goddard" }, -- 5
      { layout="pincer"; "Hawking", "Admonisher", "Admonisher" },
      { layout="circle"; "Pacifier", "Admonisher", "Admonisher", "Ancestor", "Ancestor", "Ancestor", "Lancelot", "Lancelot" },
      { "Kestrel", "Kestrel" },
      { "Hawking", "Pacifier", "Pacifier" },
      { "Dvaered Goddard", "Dvaered Vigilance", "Dvaered Vigilance" },
   }
}

