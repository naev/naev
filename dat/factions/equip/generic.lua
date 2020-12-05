require "jumpdist.lua"


-- Table of available core systems by class.
equip_classOutfits_coreSystems = {
   ["Yacht"] = {
      "Unicorp PT-100 Core System"
   },
   ["Luxury Yacht"] = {
      "Unicorp PT-100 Core System"
   },
   ["Scout"] = {
      "Unicorp PT-100 Core System", "Milspec Aegis 2201 Core System"
   },
   ["Courier"] = {
      "Unicorp PT-200 Core System", "Milspec Aegis 3601 Core System"
   },
   ["Freighter"] = {
      "Unicorp PT-600 Core System", "Milspec Aegis 5401 Core System"
   },
   ["Armoured Transport"] = {
      "Milspec Aegis 5401 Core System", "Milspec Orion 5501 Core System"
   },
   ["Fighter"] = {
      "Unicorp PT-200 Core System", "Milspec Orion 3701 Core System"
   },
   ["Bomber"] = {
      "Unicorp PT-200 Core System", "Milspec Orion 3701 Core System"
   },
   ["Corvette"] = {
      "Unicorp PT-500 Core System", "Milspec Orion 4801 Core System"
   },
   ["Destroyer"] = {
      "Unicorp PT-600 Core System", "Milspec Orion 5501 Core System"
   },
   ["Cruiser"] = {
      "Unicorp PT-1000 Core System", "Milspec Orion 9901 Core System"
   },
   ["Carrier"] = {
      "Milspec Orion 9901 Core System"
   },
   ["Drone"] = {
      "Milspec Orion 2301 Core System"
   },
   ["Heavy Drone"] = {
      "Milspec Orion 3701 Core System"
   }
}


-- Table of available engines by class.
equip_classOutfits_engines = {
   ["Yacht"] = {
      "Unicorp Hawk 150 Engine", "Nexus Dart 150 Engine"
   },
   ["Luxury Yacht"] = {
      "Unicorp Hawk 150 Engine", "Nexus Dart 150 Engine"
   },
   ["Scout"] = {
      "Unicorp Hawk 150 Engine", "Nexus Dart 150 Engine",
      "Tricon Zephyr Engine"
   },
   ["Courier"] = {
      "Unicorp Hawk 300 Engine", "Nexus Dart 300 Engine",
      "Tricon Zephyr II Engine", "Melendez Ox XL Engine"
   },
   ["Freighter"] = {
      "Unicorp Falcon 1200 Engine", "Melendez Buffalo XL Engine"
   },
   ["Armoured Transport"] = {
      "Melendez Buffalo XL Engine"
   },
   ["Fighter"] = {
      "Unicorp Hawk 300 Engine", "Nexus Dart 300 Engine",
      "Tricon Zephyr II Engine"
   },
   ["Bomber"] = {
      "Unicorp Hawk 300 Engine", "Nexus Dart 300 Engine",
      "Tricon Zephyr II Engine"
   },
   ["Corvette"] = {
      "Unicorp Falcon 700 Engine", "Nexus Arrow 700 Engine",
      "Tricon Cyclone Engine"
   },
   ["Destroyer"] = {
      "Unicorp Falcon 1200 Engine", "Nexus Arrow 1200 Engine",
      "Tricon Cyclone II Engine"
   },
   ["Cruiser"] = {
      "Unicorp Eagle 6500 Engine", "Nexus Bolt 6500 Engine",
      "Tricon Typhoon II Engine"
   },
   ["Carrier"] = {
      "Nexus Bolt 6500 Engine", "Tricon Typhoon II Engine",
      "Melendez Mammoth XL Engine"
   },
   ["Drone"] = {
      "Tricon Zephyr Engine"
   },
   ["Heavy Drone"] = {
      "Tricon Zephyr II Engine"
   }
}


-- Table of available hulls by class.
equip_classOutfits_hulls = {
   ["Yacht"] = {
      "Unicorp D-2 Light Plating", "Unicorp B-2 Light Plating"
   },
   ["Luxury Yacht"] = {
      "Unicorp D-2 Light Plating", "Unicorp B-2 Light Plating"
   },
   ["Scout"] = {
      "Unicorp D-2 Light Plating", "Unicorp B-2 Light Plating",
      "S&K Ultralight Stealth Plating"
   },
   ["Courier"] = {
      "Unicorp D-4 Light Plating", "S&K Small Cargo Hull"
   },
   ["Freighter"] = {
      "Unicorp D-24 Medium Plating", "S&K Medium Cargo Hull"
   },
   ["Armoured Transport"] = {
      "S&K Medium Cargo Hull"
   },
   ["Fighter"] = {
      "Unicorp D-4 Light Plating", "Unicorp B-4 Light Plating",
      "S&K Light Stealth Plating", "S&K Light Combat Plating"
   },
   ["Bomber"] = {
      "Unicorp D-4 Light Plating", "Unicorp B-4 Light Plating",
      "S&K Light Stealth Plating", "S&K Light Combat Plating"
   },
   ["Corvette"] = {
      "Unicorp D-12 Medium Plating", "Unicorp B-12 Medium Plating",
      "S&K Medium Stealth Plating", "S&K Medium Combat Plating"
   },
   ["Destroyer"] = {
      "Unicorp D-24 Medium Plating", "Unicorp B-24 Medium Plating",
      "S&K Medium-Heavy Stealth Plating", "S&K Medium-Heavy Combat Plating"
   },
   ["Cruiser"] = {
      "Unicorp D-72 Heavy Plating", "Unicorp B-72 Heavy Plating",
      "S&K Superheavy Combat Plating"
   },
   ["Carrier"] = {
      "Unicorp B-72 Heavy Plating", "S&K Superheavy Combat Plating"
   },
   ["Drone"] = {
      "S&K Ultralight Stealth Plating"
   },
   ["Heavy Drone"] = {
      "S&K Light Stealth Plating"
   }
}


-- Tables of available weapons by class.
-- See equip_set function for more info.
equip_classOutfits_weapons = {
   ["Yacht"] = {
      {
         "Laser Cannon MK1", "Laser Cannon MK2", "Razor MK1", "Razor MK2",
         "Laser PD MK1", "Turreted Gauss Gun"
      }
   },
   ["Luxury Yacht"] = {
      {
         "Laser Cannon MK1", "Laser Cannon MK2", "Razor MK1", "Razor MK2"
      }
   },
   ["Scout"] = {
      {
         "Laser PD MK1", "Laser PD MK2", "Razor Turret MK1",
         "Turreted Gauss Gun"
      }
   },
   ["Courier"] = {
      {
         "Laser PD MK1", "Laser PD MK2", "Razor Turret MK1",
         "Turreted Gauss Gun"
      }
   },
   ["Freighter"] = {
      {
         num = 1;
         "Laser Turret MK2", "Enygma Systems Turreted Fury Launcher"
      },
      {
         "Laser Turret MK1", "Laser Turret MK2", "EMP Grenade Launcher",
         "Pulse Beam"
      }
   },
   ["Armoured Transport"] = {
      {
         num = 2, varied = true;
         "Pulse Beam", "Enygma Systems Turreted Fury Launcher",
         "Heavy Laser", "Heavy Ripper Turret"
      },
      {
         "Laser Turret MK2", "Laser Turret MK3"
      }
   },
   ["Fighter"] = {
      {
         num = 1;
         "Mass Driver MK1", "Ion Cannon", "Unicorp Mace Launcher",
         "Unicorp Banshee Launcher", "Orion Lance", "Shattershield Lance",
         "Unicorp Headhunter Launcher", "Unicorp Fury Launcher",
         "Unicorp Medusa Launcher", "Ripper Cannon"
      },
      {
         "Plasma Blaster MK1", "Plasma Blaster MK2", "Gauss Gun",
         "Vulcan Gun", "Ripper Cannon"
      }
   },
   ["Bomber"] = {
      {
         num = 3, varied = true;
         "TeraCom Fury Launcher", "TeraCom Medusa Launcher",
         "Unicorp Headhunter Launcher", "Unicorp Mace Launcher",
         "Unicorp Banshee Launcher"
      },
      {
         "Gauss Gun", "Vulcan Gun", "Laser Cannon MK2", "Plasma Blaster MK2"
      }
   },
   ["Corvette"] = {
      {
         num = 1;
         "TeraCom Fury Launcher", "Unicorp Headhunter Launcher",
         "TeraCom Medusa Launcher"
      },
      {
         num = 2;
         "Mass Driver MK1", "Mass Driver MK2", "Heavy Ion Cannon",
         "Laser Turret MK1", "Plasma Turret MK2", "Razor Turret MK2"
      },
      {
         "Ripper Cannon", "Plasma Blaster MK2", "Laser Cannon MK2",
         "Vulcan Gun", "Ion Cannon"
      }
   },
   ["Destroyer"] = {
      {
         num = 2;
         "Railgun", "Heavy Ripper Turret", "Heavy Laser", "Orion Beam",
         "Grave Beam", "Laser Turret MK3", "Razor Turret MK2"
      },
      {
         num = 1;
         "Enygma Systems Turreted Fury Launcher", "Unicorp Caesar IV Launcher",
         "Unicorp Headhunter Launcher", "TeraCom Medusa Launcher"
      },
      {
         "Laser Turret MK2", "Laser Turret MK3", "Turreted Vulcan Gun"
      }
   },
   ["Cruiser"] = {
      {
         num = 2;
         "Turbolaser", "Ragnarok Beam", "Grave Beam", "Railgun Turret"
      },
      {
         "Heavy Laser", "Heavy Ripper Turret", "Railgun Turret"
      },
      {
         "Laser Turret MK3", "Turreted Vulcan Gun"
      }
   },
   ["Carrier"] = {
      {
         num = 2;
         "Turbolaser", "Ragnarok Beam"
      },
      {
--         num = 2;
         "Heavy Laser", "Grave Beam", "Railgun Turret"
      },
--      {
--         "Lancelot Fighter Bay"
--      },
      {
         "Laser Turret MK3", "Turreted Vulcan Gun"
      }
   },
   ["Drone"] = {
      {
         "Neutron Disruptor"
      }
   },
   ["Heavy Drone"] = {
      {
         "Heavy Neutron Disruptor"
      },
      {
         "Electron Burst Cannon"
      }
   }
}


-- Tables of available utilities by class.
-- See equip_set function for more info.
equip_classOutfits_utilities = {
   ["Yacht"] = {
      {
         varied = true;
         "Reactor Class I", "Unicorp Scrambler", "Jump Scanner",
         "Generic Afterburner"
      }
   },
   ["Luxury Yacht"] = {
      {
         varied = true;
         "Reactor Class I", "Unicorp Scrambler", "Small Shield Booster"
      }
   },
   ["Scout"] = {
      {
         varied = true;
         "Unicorp Scrambler", "Small Shield Booster", "Jump Scanner",
         "Generic Afterburner", "Emergency Shield Booster"
      }
   },
   ["Courier"] = {
      {
         varied = true;
         "Reactor Class I", "Small Shield Booster", "Unicorp Scrambler",
         "Hellburner", "Emergency Shield Booster"
      }
   },
   ["Freighter"] = {
      {
         varied = true;
         "Reactor Class II", "Medium Shield Booster", "Milspec Scrambler",
         "Droid Repair Crew", "Boarding Androids MK1"
      }
   },
   ["Armoured Transport"] = {
      {
         varied = true;
         "Reactor Class II", "Medium Shield Booster", "Milspec Scrambler",
         "Droid Repair Crew", "Boarding Androids MK1"
      }
   },
   ["Fighter"] = {
      {
         varied = true;
         "Reactor Class I", "Unicorp Scrambler", "Emergency Shield Booster",
         "Reverse Thrusters"
      }
   },
   ["Bomber"] = {
      {
         varied = true;
         "Reactor Class I", "Milspec Scrambler", "Small Shield Booster",
         "Emergency Shield Booster", "Hellburner", "Reverse Thrusters"
      }
   },
   ["Corvette"] = {
      {
         varied = true;
         "Reactor Class II", "Medium Shield Booster", "Milspec Scrambler",
         "Droid Repair Crew", "Boarding Androids MK1", "Hellburner"
      }
   },
   ["Destroyer"] = {
      {
         varied = true;
         "Reactor Class II", "Medium Shield Booster", "Droid Repair Crew",
         "Boarding Androids MK1"
      }
   },
   ["Cruiser"] = {
      {
         varied = true;
         "Reactor Class III", "Large Shield Booster", "Droid Repair Crew",
         "Boarding Androids MK2"
      }
   },
   ["Carrier"] = {
      {
         varied = true;
         "Reactor Class III", "Large Shield Booster", "Droid Repair Crew",
         "Boarding Androids MK2"
      }
   },
   ["Drone"] = {
      {
         "Reactor Class I"
      }
   },
   ["Heavy Drone"] = {
      {
         num = 1;
         "Unicorp Scrambler"
      },
      {
         "Reactor Class I"
      }
   }
}

-- Tables of available structurals by class.
-- See equip_set function for more info.
equip_classOutfits_structurals = {
   ["Yacht"] = {
      {
         varied = true;
         "Cargo Pod", "Solar Panel", "Fuel Pod", "Battery", "Shield Capacitor",
         "Improved Stabilizer", "Engine Reroute", "Steering Thrusters"
      }
   },
   ["Luxury Yacht"] = {
      {
         varied = true;
         "Shield Capacitor", "Engine Reroute", "Steering Thrusters"
      }
   },
   ["Scout"] = {
      {
         varied = true, probability = {
            ["Fuel Pod"] = 4, ["Improved Stabilizer"] = 2
         };
         "Fuel Pod", "Improved Stabilizer", "Shield Capacitor"
      }
   },
   ["Courier"] = {
      {
         varied = true, probability = {
            ["Improved Stabilizer"] = 10, ["Cargo Pod"] = 4
         };
         "Cargo Pod", "Fuel Pod", "Improved Stabilizer",
         "Improved Refrigeration Cycle"
      }
   },
   ["Freighter"] = {
      {
         varied = true, probability = {
            ["Cargo Pod"] = 6
         };
         "Cargo Pod", "Medium Fuel Pod"
      }
   },
   ["Armoured Transport"] = {
      {
         varied = true, probability = {
            ["Cargo Pod"] = 15, ["Medium Fuel Pod"] = 3
         };
         "Cargo Pod", "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
         "Improved Power Regulator"
      }
   },
   ["Fighter"] = {
      {
         varied = true, probability = {
            ["Steering Thrusters"] = 8, ["Engine Reroute"] = 4,
            ["Forward Shock Absorbers"] = 2, ["Power Regulation Override"] = 2
         };
         "Solar Panel", "Fuel Pod", "Steering Thrusters", "Engine Reroute",
         "Battery", "Shield Capacitor", "Power Regulation Override",
         "Forward Shock Absorbers"
      }
   },
   ["Bomber"] = {
      {
         varied = true;
         "Fuel Pod", "Steering Thrusters", "Engine Reroute", "Shield Capacitor"
      }
   },
   ["Corvette"] = {
      {
         varied = true;
         "Solar Panel", "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
         "Forward Shock Absorbers"
      }
   },
   ["Destroyer"] = {
      {
         varied = true;
         "Plasteel Plating", "Medium Fuel Pod", "Battery II",
         "Shield Capacitor II", "Improved Power Regulator",
         "Targeting Array"
      }
   },
   ["Cruiser"] = {
      {
         varied = true, probability = {
            ["Nanobond Plating"] = 3, ["Shield Capacitor IV"] = 2
         };
         "Biometal Armour", "Nanobond Plating", "Large Fuel Pod",
         "Battery III", "Shield Capacitor III", "Shield Capacitor IV"
      }
   },
   ["Carrier"] = {
      {
         varied = true, probability = {
            ["Nanobond Plating"] = 6, ["Shield Capacitor IV"] = 4,
            ["Large Fuel Pod"] = 3, ["Biometal Armour"] = 2
         };
         "Biometal Armour", "Nanobond Plating", "Large Fuel Pod",
         "Battery III", "Shield Capacitor III", "Shield Capacitor IV"
      }
   },
   ["Drone"] = {
      {
         "Steering Thrusters"
      }
   },
   ["Heavy Drone"] = {
      {
         num = 1;
         "Steering Thrusters"
      },
      {
         "Solar Panel"
      }
   }
}


-- Table of available core systems by base type.
equip_typeOutfits_coreSystems = {
   ["Hyena"] = {
      "Unicorp PT-100 Core System", "Milspec Orion 2301 Core System"
   },
   ["Shark"] = {
      "Unicorp PT-100 Core System", "Milspec Orion 2301 Core System"
   },
   ["Brigand"] = {
      "Unicorp PT-100 Core System", "Milspec Orion 2301 Core System"
   },
   ["Fidelity"] = {
      "Unicorp PT-100 Core System", "Milspec Orion 2301 Core System"
   },
   ["Perspicacity"] = {
      "Unicorp PT-100 Core System", "Milspec Orion 2301 Core System"
   },
   ["Derivative"] = {
      "Unicorp PT-100 Core System", "Milspec Orion 2301 Core System"
   },
   ["Kestrel"] = {
      "Unicorp PT-900 Core System", "Milspec Orion 8601 Core System"
   },
   ["Brigand"] = {
      probability = {
         ["Ultralight Bioship Brain Stage X"] = 2
      };
      "Ultralight Bioship Brain Stage 1", "Ultralight Bioship Brain Stage 2",
      "Ultralight Bioship Brain Stage X"
   },
   ["Reaver"] = {
      probability = {
         ["Light Bioship Brain Stage X"] = 3
      };
      "Light Bioship Brain Stage 1", "Light Bioship Brain Stage 2",
      "Light Bioship Brain Stage 3", "Light Bioship Brain Stage X"
   },
   ["Marauder"] = {
      probability = {
         ["Light Bioship Brain Stage X"] = 3
      };
      "Light Bioship Brain Stage 1", "Light Bioship Brain Stage 2",
      "Light Bioship Brain Stage 3", "Light Bioship Brain Stage X"
   },
   ["Odium"] = {
      probability = {
         ["Medium Bioship Brain Stage X"] = 4
      };
      "Medium Bioship Brain Stage 1", "Medium Bioship Brain Stage 2",
      "Medium Bioship Brain Stage 3", "Medium Bioship Brain Stage 4",
      "Medium Bioship Brain Stage X"
   },
   ["Nyx"] = {
      probability = {
         ["Medium-Heavy Bioship Brain Stage X"] = 5
      };
      "Medium-Heavy Bioship Brain Stage 1",
      "Medium-Heavy Bioship Brain Stage 2",
      "Medium-Heavy Bioship Brain Stage 3",
      "Medium-Heavy Bioship Brain Stage 4",
      "Medium-Heavy Bioship Brain Stage 5",
      "Medium-Heavy Bioship Brain Stage X"
   },
   ["Ira"] = {
      probability = {
         ["Superheavy Bioship Brain Stage X"] = 7
      };
      "Superheavy Bioship Brain Stage 1",
      "Superheavy Bioship Brain Stage 2",
      "Superheavy Bioship Brain Stage 3",
      "Superheavy Bioship Brain Stage 4",
      "Superheavy Bioship Brain Stage 5",
      "Superheavy Bioship Brain Stage 6",
      "Superheavy Bioship Brain Stage 7",
      "Superheavy Bioship Brain Stage X"
   },
   ["Arx"] = {
      probability = {
         ["Superheavy Bioship Brain Stage X"] = 7
      };
      "Superheavy Bioship Brain Stage 1",
      "Superheavy Bioship Brain Stage 2",
      "Superheavy Bioship Brain Stage 3",
      "Superheavy Bioship Brain Stage 4",
      "Superheavy Bioship Brain Stage 5",
      "Superheavy Bioship Brain Stage 6",
      "Superheavy Bioship Brain Stage 7",
      "Superheavy Bioship Brain Stage X"
   },
   ["Vox"] = {
      probability = {
         ["Superheavy Bioship Brain Stage X"] = 7
      };
      "Superheavy Bioship Brain Stage 1",
      "Superheavy Bioship Brain Stage 2",
      "Superheavy Bioship Brain Stage 3",
      "Superheavy Bioship Brain Stage 4",
      "Superheavy Bioship Brain Stage 5",
      "Superheavy Bioship Brain Stage 6",
      "Superheavy Bioship Brain Stage 7",
      "Superheavy Bioship Brain Stage X"
   },
}


-- Table of available engines by base type.
equip_typeOutfits_engines = {
   ["Hyena"] = {
      "Unicorp Hawk 150 Engine", "Nexus Dart 150 Engine",
      "Tricon Zephyr Engine"
   },
   ["Shark"] = {
      "Unicorp Hawk 150 Engine", "Nexus Dart 150 Engine",
      "Tricon Zephyr Engine"
   },
   ["Brigand"] = {
      "Unicorp Hawk 150 Engine", "Nexus Dart 150 Engine",
      "Tricon Zephyr Engine"
   },
   ["Fidelity"] = {
      "Unicorp Hawk 150 Engine", "Nexus Dart 150 Engine",
      "Tricon Zephyr Engine"
   },
   ["Perspicacity"] = {
      "Unicorp Hawk 150 Engine", "Nexus Dart 150 Engine",
      "Tricon Zephyr Engine"
   },
   ["Derivative"] = {
      "Unicorp Hawk 150 Engine", "Nexus Dart 150 Engine",
      "Tricon Zephyr Engine"
   },
   ["Kestrel"] = {
      "Unicorp Eagle 4500 Engine", "Nexus Bolt 4500 Engine",
      "Krain Remige Engine", "Tricon Typhoon Engine"
   },
   ["Brigand"] = {
      probability = {
         ["Ultralight Bioship Fast Fin Stage X"] = 2
      };
      "Ultralight Bioship Fast Fin Stage 1", "Ultralight Bioship Fast Fin Stage 2",
      "Ultralight Bioship Fast Fin Stage X"
   },
   ["Reaver"] = {
      probability = {
         ["Light Bioship Fast Fin Stage X"] = 3
      };
      "Light Bioship Fast Fin Stage 1", "Light Bioship Fast Fin Stage 2",
      "Light Bioship Fast Fin Stage 3", "Light Bioship Fast Fin Stage X"
   },
   ["Marauder"] = {
      probability = {
         ["Light Bioship Fast Fin Stage X"] = 3
      };
      "Light Bioship Fast Fin Stage 1", "Light Bioship Fast Fin Stage 2",
      "Light Bioship Fast Fin Stage 3", "Light Bioship Fast Fin Stage X"
   },
   ["Odium"] = {
      probability = {
         ["Medium Bioship Fast Fin Stage X"] = 4
      };
      "Medium Bioship Fast Fin Stage 1", "Medium Bioship Fast Fin Stage 2",
      "Medium Bioship Fast Fin Stage 3", "Medium Bioship Fast Fin Stage 4",
      "Medium Bioship Fast Fin Stage X"
   },
   ["Nyx"] = {
      probability = {
         ["Medium-Heavy Bioship Fast Fin Stage X"] = 5
      };
      "Medium-Heavy Bioship Fast Fin Stage 1",
      "Medium-Heavy Bioship Fast Fin Stage 2",
      "Medium-Heavy Bioship Fast Fin Stage 3",
      "Medium-Heavy Bioship Fast Fin Stage 4",
      "Medium-Heavy Bioship Fast Fin Stage 5",
      "Medium-Heavy Bioship Fast Fin Stage X"
   },
   ["Ira"] = {
      probability = {
         ["Superheavy Bioship Fast Fin Stage X"] = 7
      };
      "Superheavy Bioship Fast Fin Stage 1",
      "Superheavy Bioship Fast Fin Stage 2",
      "Superheavy Bioship Fast Fin Stage 3",
      "Superheavy Bioship Fast Fin Stage 4",
      "Superheavy Bioship Fast Fin Stage 5",
      "Superheavy Bioship Fast Fin Stage 6",
      "Superheavy Bioship Fast Fin Stage 7",
      "Superheavy Bioship Fast Fin Stage X"
   },
   ["Arx"] = {
      probability = {
         ["Superheavy Bioship Strong Fin Stage X"] = 7
      };
      "Superheavy Bioship Strong Fin Stage 1",
      "Superheavy Bioship Strong Fin Stage 2",
      "Superheavy Bioship Strong Fin Stage 3",
      "Superheavy Bioship Strong Fin Stage 4",
      "Superheavy Bioship Strong Fin Stage 5",
      "Superheavy Bioship Strong Fin Stage 6",
      "Superheavy Bioship Strong Fin Stage 7",
      "Superheavy Bioship Strong Fin Stage X"
   },
   ["Vox"] = {
      probability = {
         ["Superheavy Bioship Strong Fin Stage X"] = 7
      };
      "Superheavy Bioship Strong Fin Stage 1",
      "Superheavy Bioship Strong Fin Stage 2",
      "Superheavy Bioship Strong Fin Stage 3",
      "Superheavy Bioship Strong Fin Stage 4",
      "Superheavy Bioship Strong Fin Stage 5",
      "Superheavy Bioship Strong Fin Stage 6",
      "Superheavy Bioship Strong Fin Stage 7",
      "Superheavy Bioship Strong Fin Stage X"
   },
}


-- Table of available hulls by base type.
equip_typeOutfits_hulls = {
   ["Hyena"] = {
      "Unicorp D-2 Light Plating", "Unicorp B-2 Light Plating",
      "S&K Ultralight Stealth Plating"
   },
   ["Shark"] = {
      "Unicorp D-2 Light Plating", "Unicorp B-2 Light Plating",
      "S&K Ultralight Stealth Plating", "S&K Ultralight Combat Plating"
   },
   ["Brigand"] = {
      "Unicorp D-2 Light Plating", "Unicorp B-2 Light Plating",
      "S&K Ultralight Stealth Plating", "S&K Ultralight Combat Plating"
   },
   ["Fidelity"] = {
      "Unicorp D-2 Light Plating", "Unicorp B-2 Light Plating",
      "S&K Ultralight Stealth Plating", "S&K Ultralight Combat Plating"
   },
   ["Perspicacity"] = {
      "Unicorp D-2 Light Plating", "Unicorp B-2 Light Plating",
      "S&K Ultralight Stealth Plating", "S&K Ultralight Combat Plating"
   },
   ["Derivative"] = {
      "Unicorp D-2 Light Plating", "Unicorp B-2 Light Plating",
      "S&K Ultralight Stealth Plating", "S&K Ultralight Combat Plating"
   },
   ["Kestrel"] = {
      "Unicorp D-48 Heavy Plating", "Unicorp B-48 Heavy Plating",
      "S&K Heavy Combat Plating"
   },
   ["Brigand"] = {
      probability = {
         ["Ultralight Bioship Shell Stage X"] = 2
      };
      "Ultralight Bioship Shell Stage 1", "Ultralight Bioship Shell Stage 2",
      "Ultralight Bioship Shell Stage X"
   },
   ["Reaver"] = {
      probability = {
         ["Light Bioship Shell Stage X"] = 3
      };
      "Light Bioship Shell Stage 1", "Light Bioship Shell Stage 2",
      "Light Bioship Shell Stage 3", "Light Bioship Shell Stage X"
   },
   ["Marauder"] = {
      probability = {
         ["Light Bioship Shell Stage X"] = 3
      };
      "Light Bioship Shell Stage 1", "Light Bioship Shell Stage 2",
      "Light Bioship Shell Stage 3", "Light Bioship Shell Stage X"
   },
   ["Odium"] = {
      probability = {
         ["Medium Bioship Shell Stage X"] = 4
      };
      "Medium Bioship Shell Stage 1", "Medium Bioship Shell Stage 2",
      "Medium Bioship Shell Stage 3", "Medium Bioship Shell Stage 4",
      "Medium Bioship Shell Stage X"
   },
   ["Nyx"] = {
      probability = {
         ["Medium-Heavy Bioship Shell Stage X"] = 5
      };
      "Medium-Heavy Bioship Shell Stage 1",
      "Medium-Heavy Bioship Shell Stage 2",
      "Medium-Heavy Bioship Shell Stage 3",
      "Medium-Heavy Bioship Shell Stage 4",
      "Medium-Heavy Bioship Shell Stage 5",
      "Medium-Heavy Bioship Shell Stage X"
   },
   ["Ira"] = {
      probability = {
         ["Superheavy Bioship Shell Stage X"] = 7
      };
      "Superheavy Bioship Shell Stage 1",
      "Superheavy Bioship Shell Stage 2",
      "Superheavy Bioship Shell Stage 3",
      "Superheavy Bioship Shell Stage 4",
      "Superheavy Bioship Shell Stage 5",
      "Superheavy Bioship Shell Stage 6",
      "Superheavy Bioship Shell Stage 7",
      "Superheavy Bioship Shell Stage X"
   },
   ["Arx"] = {
      probability = {
         ["Superheavy Bioship Shell Stage X"] = 7
      };
      "Superheavy Bioship Shell Stage 1",
      "Superheavy Bioship Shell Stage 2",
      "Superheavy Bioship Shell Stage 3",
      "Superheavy Bioship Shell Stage 4",
      "Superheavy Bioship Shell Stage 5",
      "Superheavy Bioship Shell Stage 6",
      "Superheavy Bioship Shell Stage 7",
      "Superheavy Bioship Shell Stage X"
   },
   ["Vox"] = {
      probability = {
         ["Superheavy Bioship Shell Stage X"] = 7
      };
      "Superheavy Bioship Shell Stage 1",
      "Superheavy Bioship Shell Stage 2",
      "Superheavy Bioship Shell Stage 3",
      "Superheavy Bioship Shell Stage 4",
      "Superheavy Bioship Shell Stage 5",
      "Superheavy Bioship Shell Stage 6",
      "Superheavy Bioship Shell Stage 7",
      "Superheavy Bioship Shell Stage X"
   },
}


-- Tables of available weapons by base type.
-- See equip_set function for more info.
equip_typeOutfits_weapons = {
   ["Vigilance"] = {
      {
         num = 2;
         "Railgun"
      },
      {
         varied = true;
         "Turreted Vulcan Gun", "Mass Driver MK2", "Unicorp Mace Launcher",
         "Heavy Ion Cannon"
      }
   },
   ["Goddard"] = {
      {
         varied = true;
         "Railgun Turret", "Railgun", "Repeating Railgun", "Mass Driver MK3",
         "Mass Driver MK2", "Heavy Laser", "Grave Beam"
      }
   }
}


-- Tables of available utilities by base type.
-- See equip_set function for more info.
equip_typeOutfits_utilities = {}

-- Tables of available structurals by base type.
-- See equip_set function for more info.
equip_typeOutfits_structurals = {
   ["Koala"] = {
      {
         varied = true, probability = {
            ["Cargo Pod"] = 9, ["Fuel Pod"] = 2
         };
         "Cargo Pod", "Fuel Pod", "Improved Refrigeration Cycle"
      }
   }
}


-- Table of available core systems by ship.
equip_shipOutfits_coreSystems = {
   ["Za'lek Scout Drone"] = { "Milspec Aegis 2201 Core System" },
   ["Za'lek Light Drone"] = { "Milspec Orion 2301 Core System" },
   ["Za'lek Heavy Drone"] = {  "Milspec Orion 3701 Core System" },
   ["Za'lek Bomber Drone"] = { "Milspec Hermes 3602 Core System" }
}


-- Table of available engines by ship.
equip_shipOutfits_engines = {
   ["Za'lek Scout Drone"] = { "Tricon Zephyr Engine" },
   ["Za'lek Light Drone"] = { "Tricon Zephyr Engine" },
   ["Za'lek Heavy Drone"] = { "Tricon Zephyr II Engine" },
   ["Za'lek Bomber Drone"] = { "Tricon Zephyr II Engine" }
}


-- Table of available hulls by ship.
equip_shipOutfits_hulls = {
   ["Za'lek Scout Drone"] = { "S&K Ultralight Stealth Plating" },
   ["Za'lek Light Drone"] = { "S&K Ultralight Combat Plating" },
   ["Za'lek Heavy Drone"] = { "S&K Light Combat Plating" },
   ["Za'lek Bomber Drone"] = { "S&K Light Combat Plating" }
}


-- Tables of available weapons by ship.
-- See equip_set function for more info.
equip_shipOutfits_weapons = {
   ["Empire Lancelot"] = {
      {
         num = 1;
         "TeraCom Fury Launcher", "Unicorp Headhunter Launcher",
         "Unicorp Vengeance Launcher", "Enygma Systems Spearhead Launcher",
         "Heavy Ripper Cannon"
      },
      {
         "Ripper Cannon"
      }
   },
   ["Sirius Fidelity"] = {
      {
         num = 2;
         "Razor MK2", "Razor MK3", "Ion Cannon"
      },
      {
         "Razor MK2", "Razor MK3", "Ion Cannon"
      }
   },
   ["Za'lek Scout Drone"] = {
      {
         "Particle Lance"
      }
   },
   ["Za'lek Light Drone"] = {
      {
         "Particle Lance", "Orion Lance"
      }
   },
   ["Za'lek Heavy Drone"] = {
      {
         "Grave Lance"
      },
      {
         num = 1;
         "Orion Lance"
      },
      {
         "Electron Burst Cannon"
      }
   },
   ["Za'lek Bomber Drone"] = {
      {
         varied = true;
         "Unicorp Fury Launcher", "TeraCom Fury Launcher",
         "Unicorp Headhunter Launcher", "Unicorp Vengeance Launcher"
      },
      {
         "Electron Burst Cannon"
      },
      {
         "Particle Lance"
      }
   }
}


-- Tables of available utilities by ship.
-- See equip_set function for more info.
equip_shipOutfits_utilities = {}

-- Tables of available structurals by ship.
-- See equip_set function for more info.
equip_shipOutfits_structurals = {}


--[[
-- @brief Wrapper for pilot.addOutfit that prints a warning if no outfits added.
--]]
function equip_warn( p, outfit, q, bypass )
   if q == nil then q = 1 end
   if bypass == nil then bypass = false end
   local r = pilot.addOutfit( p, outfit, q, bypass )
   if r <= 0 then
      warn( string.format( _("Could not equip %s on pilot %s!"), outfit, p:name() ) )
   end
   return r
end


--[[
-- @brief Choose an outfit from a table of outfits.
--
--    @param p Pilot to equip to.
--    @param set table laying out the set of outfits to equip (see below).
--
-- ``set`` is split up into sub-tables that are iterated through. These
-- tables include a "num" field which indicates how many of the chosen outfit
-- to equip before moving on to the next set; if nil, the chosen outfit will be
-- equipped as many times as possible. For example, if you list 3 tables with
-- "num" set to 2, 1, and nil respectively, two of an outfit from the first
-- table will be equipped, followed by one of an outfit from the second table,
-- and then finally all remaining slots will be filled with an outfit from the
-- third table.
--
-- If, rather than equipping multiples of the same outfit you would like to
-- select a random outfit `num` times, you can do so by setting "varied" to
-- true.
--
-- "probability" can be set to a table specifying the relative chance of each
-- outfit (keyed by name) to the other outfits. If unspecified, each outfit
-- will have a relative chance of 1. So for example, if the outfits are "Foo",
-- "Bar", and "Baz", with no "probability" table, each outfit will have a 1/3
-- chance of being selected; however, with this "probability" table:
--
--    probability = { ["Foo"] = 6, ["Bar"] = 2 }
--
-- This will lead to "Foo" having a 6/9 (2/3) chance, "Bar" will have a 2/9
-- chance, and "Baz" will have a 1/9 chance
--
-- Note that there should only be one type of outfit (weapons, utilities, or
-- structurals) in ``set``; including multiple types will prevent proper
-- detection of how many are needed.
--]]
function equip_set( p, set )
   if set == nil then return end

   local num, varied, probability
   local choices, chance, c, i, equipped

   for k, v in ipairs( set ) do
      num = v.num
      varied = v.varied
      probability = v.probability

      choices = {}
      for i, choice in ipairs( v ) do
         choices[ #choices + 1 ] = choice

         -- Add entries based on "probability".
         if probability ~= nil then
            chance = probability[ choice ]
            if chance ~= nil then
               -- Starting at 2 because the first one is already in the table.
               for j=2,chance do
                  choices[ #choices + 1 ] = choice
               end
            end
         end
      end

      c = rnd.rnd( 1, #choices )
      i = 1
      while #choices > 0 and (num == nil or i <= num) do
         i = i + 1
         if varied then c = rnd.rnd( 1, #choices ) end

         equipped = p:addOutfit( choices[c] )
         if equipped <= 0 then
            if varied or num == nil then
               table.remove( choices, c )
               c = rnd.rnd( 1, #choices )
            else
               break
            end
         end
      end
   end
end


--[[
-- @brief Does generic pilot equipping
--
--    @param p Pilot to equip
--]]
function equip_generic( p )
   -- Start with an empty ship
   p:rmOutfit( "all" )
   p:rmOutfit( "cores" )

   local shipname = p:ship():nameRaw()
   local basetype = p:ship():baseType()
   local class = p:ship():class()
   local success
   local o

   -- Core systems
   success = false
   o = equip_shipOutfits_coreSystems[shipname]
   if o ~= nil then
      success = equip_warn( p, o[rnd.rnd(1, #o)] )
   end
   o = equip_typeOutfits_coreSystems[basetype]
   if not success and o ~= nil then
      success = equip_warn( p, o[rnd.rnd(1, #o)] )
   end
   o = equip_classOutfits_coreSystems[class]
   if not success and o ~= nil then
      success = equip_warn( p, o[rnd.rnd(1, #o)] )
   end
   if not success then
      equip_warn( p, "Unicorp PT-100 Core System" )
   end

   -- Engines
   success = false
   o = equip_shipOutfits_engines[shipname]
   if o ~= nil then
      success = equip_warn( p, o[rnd.rnd(1, #o)] )
   end
   o = equip_typeOutfits_engines[basetype]
   if not success and o ~= nil then
      success = equip_warn( p, o[rnd.rnd(1, #o)] )
   end
   o = equip_classOutfits_engines[class]
   if not success and o ~= nil then
      success = equip_warn( p, o[rnd.rnd(1, #o)] )
   end
   if not success then
      equip_warn( p, "Unicorp Hawk 150 Engine" )
   end

   -- Hulls
   success = false
   o = equip_shipOutfits_hulls[shipname]
   if o ~= nil then
      success = equip_warn( p, o[rnd.rnd(1, #o)] )
   end
   o = equip_typeOutfits_hulls[basetype]
   if not success and o ~= nil then
      success = equip_warn( p, o[rnd.rnd(1, #o)] )
   end
   o = equip_classOutfits_hulls[class]
   if not success and o ~= nil then
      success = equip_warn( p, o[rnd.rnd(1, #o)] )
   end
   if not success then
      equip_warn( p, "Unicorp D-2 Light Plating" )
   end

   -- Weapons
   equip_set( p, equip_shipOutfits_weapons[shipname] )
   equip_set( p, equip_typeOutfits_weapons[basetype] )
   equip_set( p, equip_classOutfits_weapons[class] )

   -- Utilities
   equip_set( p, equip_shipOutfits_utilities[shipname] )
   equip_set( p, equip_typeOutfits_utilities[basetype] )
   equip_set( p, equip_classOutfits_utilities[class] )

   -- Structurals
   equip_set( p, equip_shipOutfits_structurals[shipname] )
   equip_set( p, equip_typeOutfits_structurals[basetype] )
   equip_set( p, equip_classOutfits_structurals[class] )

   -- Add cargo
   local avail_cargo = {}
   local systems = getsysatdistance( nil, 0, 4 )
   for i, sys in ipairs( systems ) do
      for j, pl in ipairs( sys:planets() ) do
         for k, com in ipairs( pl:commoditiesSold() ) do
            avail_cargo[ #avail_cargo + 1 ] = com
         end
      end
   end

   if #avail_cargo > 0 then
      for i=1,rnd.rnd(1,3) do
         local ncargo = rnd.rnd( 0, p:cargoFree() )
         p:cargoAdd( avail_cargo[ rnd.rnd( 1, #avail_cargo ) ]:nameRaw(), ncargo )
      end
   end
end
