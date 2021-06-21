wave_score_table = {
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
wave_round_enemies = {
   skirmisher = {
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
   warrior = {
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
   warlord = {
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

