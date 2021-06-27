require "jumpdist"


-- Probability of cargo by class.
equip_classCargo = {
   ["Yacht"] = .7,
   ["Luxury Yacht"] = .6,
   ["Scout"] = .05,
   ["Courier"] = .8,
   ["Freighter"] = .8,
   ["Armoured Transport"] = .8,
   ["Fighter"] = .1,
   ["Bomber"] = .1,
   ["Corvette"] = .15,
   ["Destroyer"] = .2,
   ["Cruiser"] = .2,
   ["Carrier"] = .3,
   ["Drone"] = .05,
   ["Heavy Drone"] = .05,
}

-- Table of available core systems by class.
equip_classOutfits_coreSystems = {
   ["Yacht"] = {
      "Unicorp PT-16 Core System", "Unicorp PT-68 Core System",
   },
   ["Luxury Yacht"] = {
      "Unicorp PT-16 Core System", "Milspec Orion 2301 Core System",
   },
   ["Scout"] = {
      "Unicorp PT-16 Core System", "Milspec Orion 2301 Core System",
   },
   ["Courier"] = {
      "Unicorp PT-68 Core System",
   },
   ["Freighter"] = {
      "Unicorp PT-310 Core System",
   },
   ["Armoured Transport"] = {
      "Unicorp PT-310 Core System", "Milspec Orion 5501 Core System",
   },
   ["Fighter"] = {
      "Unicorp PT-68 Core System", "Milspec Orion 3701 Core System",
      "Milspec Thalos 3602 Core System",
   },
   ["Bomber"] = {
      "Unicorp PT-68 Core System", "Milspec Orion 3701 Core System",
      "Milspec Thalos 3602 Core System",
   },
   ["Corvette"] = {
      "Unicorp PT-200 Core System", "Milspec Orion 4801 Core System",
      "Milspec Thalos 4702 Core System",
   },
   ["Destroyer"] = {
      "Unicorp PT-310 Core System", "Milspec Orion 5501 Core System",
      "Milspec Thalos 5402 Core System",
   },
   ["Cruiser"] = {
      "Milspec Orion 8601 Core System", "Milspec Orion 9901 Core System",
      "Unicorp PT-2200 Core System",
   },
   ["Carrier"] = {
      "Milspec Thalos 8502 Core System", "Milspec Thalos 9802 Core System",
   },
   ["Drone"] = {
      "Milspec Orion 2301 Core System",
   },
   ["Heavy Drone"] = {
      "Milspec Orion 3701 Core System",
   },
}


-- Table of available engines by class.
equip_classOutfits_engines = {
   ["Yacht"] = {
      "Nexus Dart 150 Engine", "Unicorp Hawk 350 Engine",
      "Tricon Zephyr II Engine",
   },
   ["Luxury Yacht"] = {
      "Nexus Dart 150 Engine", "Tricon Zephyr Engine",
   },
   ["Scout"] = {
      "Nexus Dart 150 Engine", "Tricon Zephyr Engine",
   },
   ["Courier"] = {
      "Unicorp Hawk 350 Engine", "Tricon Zephyr II Engine",
      "Melendez Ox XL Engine",
   },
   ["Freighter"] = {
      "Unicorp Falcon 1300 Engine", "Melendez Buffalo XL Engine",
   },
   ["Armoured Transport"] = {
      "Unicorp Falcon 1300 Engine", "Melendez Buffalo XL Engine",
   },
   ["Fighter"] = {
      "Unicorp Hawk 350 Engine", "Tricon Zephyr II Engine",
   },
   ["Bomber"] = {
      "Unicorp Hawk 350 Engine", "Tricon Zephyr II Engine",
      "Melendez Ox XL Engine",
   },
   ["Corvette"] = {
      "Nexus Arrow 700 Engine", "Tricon Cyclone Engine",
   },
   ["Destroyer"] = {
      "Unicorp Falcon 1300 Engine", "Tricon Cyclone II Engine",
   },
   ["Cruiser"] = {
      "Unicorp Eagle 7000 Engine", "Tricon Typhoon II Engine",
   },
   ["Carrier"] = {
      "Unicorp Eagle 7000 Engine", "Tricon Typhoon II Engine",
      "Melendez Mammoth XL Engine",
   },
   ["Drone"] = {
      "Nexus Dart 150 Engine",
   },
   ["Heavy Drone"] = {
      "Unicorp Hawk 350 Engine",
   },
}


-- Table of available hulls by class.
equip_classOutfits_hulls = {
   ["Yacht"] = {
      "Unicorp D-2 Light Plating", "Unicorp D-4 Light Plating",
      "S&K Small Cargo Hull",
   },
   ["Luxury Yacht"] = {
      "Unicorp D-2 Light Plating", "Nexus Light Stealth Plating",
   },
   ["Scout"] = {
      "Unicorp D-2 Light Plating", "Nexus Light Stealth Plating",
   },
   ["Courier"] = {
      "Unicorp D-4 Light Plating", "S&K Small Cargo Hull",
   },
   ["Freighter"] = {
      "Unicorp D-24 Medium Plating", "S&K Medium Cargo Hull",
   },
   ["Armoured Transport"] = {
      "Unicorp D-24 Medium Plating", "S&K Medium Cargo Hull",
   },
   ["Fighter"] = {
      "Unicorp D-4 Light Plating", "Nexus Light Stealth Plating",
      "S&K Light Combat Plating"
   },
   ["Bomber"] = {
      "Unicorp D-4 Light Plating", "Nexus Light Stealth Plating",
      "S&K Light Combat Plating"
   },
   ["Corvette"] = {
      "Unicorp D-12 Medium Plating", "Nexus Medium Stealth Plating",
      "S&K Medium Combat Plating"
   },
   ["Destroyer"] = {
      "Unicorp D-24 Medium Plating", "Nexus Medium Stealth Plating",
      "S&K Medium-Heavy Combat Plating"
   },
   ["Cruiser"] = {
      "Unicorp D-68 Heavy Plating", "S&K Superheavy Combat Plating"
   },
   ["Carrier"] = {
      "Unicorp D-68 Heavy Plating", "S&K Superheavy Combat Plating"
   },
   ["Drone"] = {
      "Nexus Light Stealth Plating"
   },
   ["Heavy Drone"] = {
      "Nexus Light Stealth Plating", "S&K Light Combat Plating"
   },
}


-- Tables of available weapons by class.
-- See equip_set function for more info.
equip_classOutfits_weapons = {
   ["Yacht"] = {
      {
         "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
         "Laser Turret MK1", "Razor Turret MK1", "Turreted Gauss Gun",
         "Plasma Turret MK1", "Particle Beam",
      },
   },
   ["Luxury Yacht"] = {
      {
         "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
      },
   },
   ["Scout"] = {
      {
         "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
         "TeraCom Mace Launcher", "TeraCom Banshee Launcher",
      },
   },
   ["Courier"] = {
      {
         "Laser Turret MK1", "Razor Turret MK1", "Turreted Gauss Gun",
         "Plasma Turret MK1", "Particle Beam",
      },
   },
   ["Freighter"] = {
      {
         num = 1;
         "Enygma Systems Turreted Fury Launcher",
         "Enygma Systems Turreted Headhunter Launcher",
      },
      {
         "Laser Turret MK2", "Razor Turret MK2", "Turreted Vulcan Gun",
         "Plasma Turret MK2", "Orion Beam",
      },
   },
   ["Armoured Transport"] = {
      {
         "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
      },
      {
         num = 1;
         "Enygma Systems Turreted Fury Launcher",
         "Enygma Systems Turreted Headhunter Launcher",
      },
      {
         "Laser Turret MK2", "Razor Turret MK2", "Turreted Vulcan Gun",
         "Plasma Turret MK2", "Orion Beam", "EMP Grenade Launcher",
         "Enygma Systems Turreted Fury Launcher",
         "Enygma Systems Turreted Headhunter Launcher",
      },
   },
   ["Fighter"] = {
      {
         "Unicorp Headhunter Launcher", "Unicorp Fury Launcher",
         "Unicorp Medusa Launcher",
      },
      {
         "Laser Cannon MK2", "Razor MK2", "Vulcan Gun", "Plasma Blaster MK2",
         "Orion Lance", "Ion Cannon",
      },
      {
         "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
         "Unicorp Mace Launcher", "Unicorp Banshee Launcher",
      },
   },
   ["Bomber"] = {
      {
         varied = true;
         "TeraCom Fury Launcher", "TeraCom Medusa Launcher",
         "Unicorp Headhunter Launcher",
      },
      {
         "Laser Cannon MK2", "Razor MK2", "Vulcan Gun", "Plasma Blaster MK2",
         "Ion Cannon",
      },
   },
   ["Corvette"] = {
      {
         varied = true;
         "Unicorp Fury Launcher", "Unicorp Headhunter Launcher",
         "Unicorp Medusa Launcher", "Unicorp Vengeance Launcher",
         "Enygma Systems Spearhead Launcher", "Unicorp Caesar IV Launcher",
         "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
         "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
         "TeraCom Imperator Launcher",
      },
      {
         probability = {
            ["Ripper Cannon"] = 8, ["Slicer"] = 8, ["Shredder"] = 8,
            ["Plasma Cannon"] = 8,
         };
         "Ripper Cannon", "Slicer", "Shredder", "Plasma Cannon",
         "Laser Cannon MK2", "Razor MK2", "Vulcan Gun", "Plasma Blaster MK2",
      },
   },
   ["Destroyer"] = {
      {
         "Railgun", "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
      },
      {
         num = 1;
         "Enygma Systems Turreted Fury Launcher",
         "Enygma Systems Turreted Headhunter Launcher",
      },
      {
         num = 1;
         "Heavy Ripper Cannon", "Mass Driver", "Plasma Cluster Cannon",
         "Grave Lance", "Heavy Ion Cannon", "Laser Turret MK2",
         "Razor Turret MK2", "Turreted Vulcan Gun", "Plasma Turret MK2",
         "Orion Beam", "Enygma Systems Turreted Fury Launcher",
         "Enygma Systems Turreted Headhunter Launcher",
      },
      {
         "Heavy Ripper Cannon", "Mass Driver", "Plasma Cluster Cannon",
         "Grave Lance", "Heavy Ion Cannon", "Laser Turret MK2",
         "Razor Turret MK2", "Turreted Vulcan Gun", "Plasma Turret MK2",
         "Orion Beam",
      },
   },
   ["Cruiser"] = {
      {
         num = 1;
         "Enygma Systems Turreted Fury Launcher",
         "Enygma Systems Turreted Headhunter Launcher",
      },
      {
         "Heavy Ripper Turret", "Railgun Turret", "Ragnarok Beam",
      },
      {
         "Railgun", "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
      },
   },
   ["Carrier"] = {
      {
         varied = true;
         "Lancelot Fighter Bay",
         "Hyena Fighter Dock",
      },
      {
         "Heavy Laser Turret", "Railgun Turret", "Ragnarok Beam",
      },
      {
         "Heavy Laser Turret", "Grave Beam",
      },
   },
   ["Drone"] = {
      {
         "Neutron Disruptor"
      },
   },
   ["Heavy Drone"] = {
      {
         "Shatterer Launcher"
      },
      {
         "Heavy Neutron Disruptor"
      },
   }
}


-- Tables of available utilities by class.
-- See equip_set function for more info.
equip_classOutfits_utilities = {
   ["Yacht"] = {
      {
         varied = true;
         "Unicorp Scrambler", "Jump Scanner",
         "Unicorp Light Afterburner", "Sensor Array",
      },
   },
   ["Luxury Yacht"] = {
      {
         varied = true;
         "Unicorp Scrambler", "Jump Scanner",
         "Unicorp Light Afterburner", "Sensor Array",
      },
   },
   ["Scout"] = {
      {
         varied = true;
         "Unicorp Scrambler", "Jump Scanner",
         "Unicorp Light Afterburner",
         "Unicorp Jammer", "Sensor Array",
      },
   },
   ["Courier"] = {
      {
         varied = true;
         "Unicorp Scrambler", "Jump Scanner",
         "Unicorp Light Afterburner",
      },
   },
   ["Freighter"] = {
      {
         varied = true;
         "Droid Repair Crew", "Targeting Array",
         "Milspec Jammer", "Emergency Shield Booster", "Milspec Scrambler",
      },
   },
   ["Armoured Transport"] = {
      {
         varied = true;
         "Targeting Array", "Milspec Scrambler", "Agility Combat AI",
         "Milspec Jammer", "Emergency Shield Booster", "Droid Repair Crew",
      },
   },
   ["Fighter"] = {
      {
         varied = true;
         "Unicorp Scrambler", "Unicorp Light Afterburner",
         "Weapons Ionizer", "Emergency Shield Booster",
         "Sensor Array", "Hellburner",
      },
   },
   ["Bomber"] = {
      {
         varied = true;
         "Unicorp Scrambler", "Unicorp Light Afterburner",
         "Milspec Scrambler", "Milspec Jammer", "Weapons Ionizer",
         "Emergency Shield Booster", "Sensor Array",
         "Hellburner",
      },
   },
   ["Corvette"] = {
      {
         varied = true;
         "Milspec Scrambler", "Milspec Jammer", "Weapons Ionizer",
         "Hellburner", "Agility Combat AI",
         "Emergency Shield Booster", "Sensor Array",
         "Unicorp Medium Afterburner", "Droid Repair Crew",
      },
   },
   ["Destroyer"] = {
      {
         varied = true;
         "Targeting Array", "Weapons Ionizer",
         "Hellburner", "Emergency Shield Booster",
         "Sensor Array", "Droid Repair Crew",
         "Agility Combat AI",
      },
   },
   ["Cruiser"] = {
      {
         varied = true;
         "Targeting Array", "Weapons Ionizer",
         "Sensor Array", "Droid Repair Crew",
         "Agility Combat AI",
      },
   },
   ["Carrier"] = {
      {
         varied = true;
         "Targeting Array", "Sensor Array", "Droid Repair Crew",
         "Agility Combat AI",
      },
   },
   ["Drone"] = {
      {
      },
   },
   ["Heavy Drone"] = {
      {
         num = 1;
         "Unicorp Scrambler"
      },
   }
}

-- Tables of available structurals by class.
-- See equip_set function for more info.
equip_classOutfits_structurals = {
   ["Yacht"] = {
      {
         varied = true;
         "Cargo Pod", "Fuel Pod", "Battery", "Shield Capacitor",
         "Improved Stabilizer", "Engine Reroute",
      },
   },
   ["Luxury Yacht"] = {
      {
         varied = true;
         "Improved Stabilizer", "Engine Reroute",
      },
   },
   ["Scout"] = {
      {
         varied = true, probability = {
            ["Fuel Pod"] = 4, ["Improved Stabilizer"] = 2
         };
         "Fuel Pod", "Improved Stabilizer", "Shield Capacitor",
      },
   },
   ["Courier"] = {
      {
         varied = true, probability = {
            ["Cargo Pod"] = 4,
         };
         "Cargo Pod", "Fuel Pod", "Improved Stabilizer",
      },
   },
   ["Freighter"] = {
      {
         varied = true, probability = {
            ["Medium Cargo Pod"] = 6,
         };
         "Medium Cargo Pod", "Medium Fuel Pod",
      },
   },
   ["Armoured Transport"] = {
      {
         varied = true, probability = {
            ["Cargo Pod"] = 15, ["Medium Fuel Pod"] = 3,
         };
         "Cargo Pod", "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
         "Plasteel Plating",
      },
   },
   ["Fighter"] = {
      {
         varied = true, probability = {
            ["Improved Stabilizer"] = 4, ["Engine Reroute"] = 4,
         };
         "Fuel Pod", "Improved Stabilizer", "Engine Reroute",
         "Battery", "Shield Capacitor", "Reactor Class I",
      },
   },
   ["Bomber"] = {
      {
         varied = true;
         "Fuel Pod", "Improved Stabilizer", "Engine Reroute",
         "Shield Capacitor", "Reactor Class I",
      },
   },
   ["Corvette"] = {
      {
         varied = true;
         "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
         "Plasteel Plating", "Reactor Class II",
      },
   },
   ["Destroyer"] = {
      {
         varied = true;
         "Large Fuel Pod", "Battery III", "Shield Capacitor IV",
         "Shield Capacitor III", "Nanobond Plating", "Reactor Class III",
      },
      {
         varied = true;
         "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
         "Plasteel Plating", "Reactor Class II",
      },
   },
   ["Cruiser"] = {
      {
         varied = true, probability = {
            ["Nanobond Plating"] = 3, ["Shield Capacitor IV"] = 2,
         };
         "Large Fuel Pod", "Biometal Armour", "Nanobond Plating",
         "Battery III", "Shield Capacitor III", "Shield Capacitor IV",
         "Reactor Class III",
      },
   },
   ["Carrier"] = {
      {
         varied = true, probability = {
            ["Nanobond Plating"] = 6, ["Shield Capacitor IV"] = 4,
            ["Large Fuel Pod"] = 3, ["Biometal Armour"] = 2
         };
         "Large Fuel Pod", "Biometal Armour", "Nanobond Plating",
         "Battery III", "Shield Capacitor III", "Shield Capacitor IV",
      },
   },
   ["Drone"] = {
      {
      },
   },
   ["Heavy Drone"] = {
      {
         num = 1;
         "Battery"
      },
      {
      },
   }
}


-- Table of available core systems by base type.
equip_typeOutfits_coreSystems = {
   ["Hyena"] = {
      "Unicorp PT-16 Core System", "Milspec Orion 2301 Core System",
   },
   ["Shark"] = {
      "Unicorp PT-16 Core System", "Milspec Orion 2301 Core System",
   },
   ["Fidelity"] = {
      "Unicorp PT-16 Core System", "Milspec Orion 2301 Core System",
   },
   ["Derivative"] = {
      "Unicorp PT-16 Core System", "Milspec Orion 2301 Core System",
   },
   ["Vendetta"] = {
      "Unicorp PT-68 Core System", "Milspec Orion 3701 Core System",
      "Milspec Thalos 3602 Core System",
   },
   ["Kestrel"] = {
      "Unicorp PT-500 Core System", "Milspec Orion 8601 Core System",
   },
   ["Hawking"] = {
      "Unicorp PT-500 Core System", "Milspec Orion 8601 Core System",
   },
   ["Brigand"] = {
      probability = {
         ["Ultralight Brain Stage X"] = 2
      };
      "Ultralight Brain Stage 1", "Ultralight Brain Stage 2",
      "Ultralight Brain Stage X",
   },
   ["Reaver"] = {
      probability = {
         ["Light Brain Stage X"] = 3
      };
      "Light Brain Stage 1", "Light Brain Stage 2",
      "Light Brain Stage 3", "Light Brain Stage X",
   },
   ["Marauder"] = {
      probability = {
         ["Light Brain Stage X"] = 3
      };
      "Light Brain Stage 1", "Light Brain Stage 2",
      "Light Brain Stage 3", "Light Brain Stage X",
   },
   ["Odium"] = {
      probability = {
         ["Medium Brain Stage X"] = 4
      };
      "Medium Brain Stage 1", "Medium Brain Stage 2",
      "Medium Brain Stage 3", "Medium Brain Stage 4",
      "Medium Brain Stage X",
   },
   ["Nyx"] = {
      probability = {
         ["Medium-Heavy Brain Stage X"] = 5
      };
      "Medium-Heavy Brain Stage 1",
      "Medium-Heavy Brain Stage 2",
      "Medium-Heavy Brain Stage 3",
      "Medium-Heavy Brain Stage 4",
      "Medium-Heavy Brain Stage 5",
      "Medium-Heavy Brain Stage X",
   },
   ["Ira"] = {
      probability = {
         ["Heavy Brain Stage X"] = 6
      };
      "Heavy Brain Stage 1",
      "Heavy Brain Stage 2",
      "Heavy Brain Stage 3",
      "Heavy Brain Stage 4",
      "Heavy Brain Stage 5",
      "Heavy Brain Stage 6",
      "Heavy Brain Stage X",
   },
   ["Arx"] = {
      probability = {
         ["Superheavy Brain Stage X"] = 7
      };
      "Superheavy Brain Stage 1",
      "Superheavy Brain Stage 2",
      "Superheavy Brain Stage 3",
      "Superheavy Brain Stage 4",
      "Superheavy Brain Stage 5",
      "Superheavy Brain Stage 6",
      "Superheavy Brain Stage 7",
      "Superheavy Brain Stage X",
   },
   ["Vox"] = {
      probability = {
         ["Superheavy Brain Stage X"] = 7
      };
      "Superheavy Brain Stage 1",
      "Superheavy Brain Stage 2",
      "Superheavy Brain Stage 3",
      "Superheavy Brain Stage 4",
      "Superheavy Brain Stage 5",
      "Superheavy Brain Stage 6",
      "Superheavy Brain Stage 7",
      "Superheavy Brain Stage X",
   },
}


-- Table of available engines by base type.
equip_typeOutfits_engines = {
   ["Hyena"] = {
      "Nexus Dart 150 Engine", "Tricon Zephyr Engine",
   },
   ["Shark"] = {
      "Nexus Dart 150 Engine", "Tricon Zephyr Engine",
   },
   ["Fidelity"] = {
      "Nexus Dart 150 Engine", "Tricon Zephyr Engine",
   },
   ["Derivative"] = {
      "Nexus Dart 150 Engine", "Tricon Zephyr Engine",
   },
   ["Vendetta"] = {
      "Unicorp Hawk 350 Engine", "Melendez Ox XL Engine",
      "Tricon Zephyr II Engine",
   },
   ["Kestrel"] = {
      "Nexus Bolt 4500 Engine", "Krain Remige Engine",
      "Tricon Typhoon Engine",
   },
   ["Hawking"] = {
      "Nexus Bolt 4500 Engine", "Tricon Typhoon Engine",
   },
   ["Brigand"] = {
      probability = {
         ["Ultralight Fast Gene Drive Stage X"] = 2
      };
      "Ultralight Fast Gene Drive Stage 1", "Ultralight Fast Gene Drive Stage 2",
      "Ultralight Fast Gene Drive Stage X",
   },
   ["Reaver"] = {
      probability = {
         ["Light Fast Gene Drive Stage X"] = 3
      };
      "Light Fast Gene Drive Stage 1", "Light Fast Gene Drive Stage 2",
      "Light Fast Gene Drive Stage 3", "Light Fast Gene Drive Stage X",
   },
   ["Marauder"] = {
      probability = {
         ["Light Fast Gene Drive Stage X"] = 3
      };
      "Light Fast Gene Drive Stage 1", "Light Fast Gene Drive Stage 2",
      "Light Fast Gene Drive Stage 3", "Light Fast Gene Drive Stage X",
   },
   ["Odium"] = {
      probability = {
         ["Medium Fast Gene Drive Stage X"] = 4
      };
      "Medium Fast Gene Drive Stage 1", "Medium Fast Gene Drive Stage 2",
      "Medium Fast Gene Drive Stage 3", "Medium Fast Gene Drive Stage 4",
      "Medium Fast Gene Drive Stage X",
   },
   ["Nyx"] = {
      probability = {
         ["Medium-Heavy Fast Gene Drive Stage X"] = 5
      };
      "Medium-Heavy Fast Gene Drive Stage 1",
      "Medium-Heavy Fast Gene Drive Stage 2",
      "Medium-Heavy Fast Gene Drive Stage 3",
      "Medium-Heavy Fast Gene Drive Stage 4",
      "Medium-Heavy Fast Gene Drive Stage 5",
      "Medium-Heavy Fast Gene Drive Stage X",
   },
   ["Ira"] = {
      probability = {
         ["Heavy Fast Gene Drive Stage X"] = 6
      };
      "Heavy Fast Gene Drive Stage 1",
      "Heavy Fast Gene Drive Stage 2",
      "Heavy Fast Gene Drive Stage 3",
      "Heavy Fast Gene Drive Stage 4",
      "Heavy Fast Gene Drive Stage 5",
      "Heavy Fast Gene Drive Stage 6",
      "Heavy Fast Gene Drive Stage X",
   },
   ["Arx"] = {
      probability = {
         ["Superheavy Strong Gene Drive Stage X"] = 7
      };
      "Superheavy Strong Gene Drive Stage 1",
      "Superheavy Strong Gene Drive Stage 2",
      "Superheavy Strong Gene Drive Stage 3",
      "Superheavy Strong Gene Drive Stage 4",
      "Superheavy Strong Gene Drive Stage 5",
      "Superheavy Strong Gene Drive Stage 6",
      "Superheavy Strong Gene Drive Stage 7",
      "Superheavy Strong Gene Drive Stage X",
   },
   ["Vox"] = {
      probability = {
         ["Superheavy Strong Gene Drive Stage X"] = 7
      };
      "Superheavy Strong Gene Drive Stage 1",
      "Superheavy Strong Gene Drive Stage 2",
      "Superheavy Strong Gene Drive Stage 3",
      "Superheavy Strong Gene Drive Stage 4",
      "Superheavy Strong Gene Drive Stage 5",
      "Superheavy Strong Gene Drive Stage 6",
      "Superheavy Strong Gene Drive Stage 7",
      "Superheavy Strong Gene Drive Stage X",
   },
}


-- Table of available hulls by base type.
equip_typeOutfits_hulls = {
   ["Hyena"] = {
      "Unicorp D-2 Light Plating", "Nexus Light Stealth Plating",
   },
   ["Shark"] = {
      "Unicorp D-2 Light Plating", "Nexus Light Stealth Plating",
      "S&K Ultralight Combat Plating",
   },
   ["Fidelity"] = {
      "Unicorp D-2 Light Plating", "Nexus Light Stealth Plating",
      "S&K Ultralight Combat Plating",
   },
   ["Derivative"] = {
      "Unicorp D-2 Light Plating", "Nexus Light Stealth Plating",
      "S&K Ultralight Combat Plating",
   },
   ["Kestrel"] = {
      "Unicorp D-48 Heavy Plating", "Unicorp D-68 Heavy Plating",
   },
   ["Hawking"] = {
      "Unicorp D-48 Heavy Plating", "Unicorp D-68 Heavy Plating",
   },
   ["Brigand"] = {
      probability = {
         ["Ultralight Shell Stage X"] = 2,
      };
      "Ultralight Shell Stage 1", "Ultralight Shell Stage 2",
      "Ultralight Shell Stage X",
   },
   ["Reaver"] = {
      probability = {
         ["Light Shell Stage X"] = 3,
      };
      "Light Shell Stage 1", "Light Shell Stage 2",
      "Light Shell Stage 3", "Light Shell Stage X",
   },
   ["Marauder"] = {
      probability = {
         ["Light Shell Stage X"] = 3,
      };
      "Light Shell Stage 1", "Light Shell Stage 2",
      "Light Shell Stage 3", "Light Shell Stage X",
   },
   ["Odium"] = {
      probability = {
         ["Medium Shell Stage X"] = 4,
      };
      "Medium Shell Stage 1", "Medium Shell Stage 2",
      "Medium Shell Stage 3", "Medium Shell Stage 4",
      "Medium Shell Stage X",
   },
   ["Nyx"] = {
      probability = {
         ["Medium-Heavy Shell Stage X"] = 5,
      };
      "Medium-Heavy Shell Stage 1",
      "Medium-Heavy Shell Stage 2",
      "Medium-Heavy Shell Stage 3",
      "Medium-Heavy Shell Stage 4",
      "Medium-Heavy Shell Stage 5",
      "Medium-Heavy Shell Stage X",
   },
   ["Ira"] = {
      probability = {
         ["Heavy Shell Stage X"] = 6,
      };
      "Heavy Shell Stage 1",
      "Heavy Shell Stage 2",
      "Heavy Shell Stage 3",
      "Heavy Shell Stage 4",
      "Heavy Shell Stage 5",
      "Heavy Shell Stage 6",
      "Heavy Shell Stage X",
   },
   ["Arx"] = {
      probability = {
         ["Superheavy Shell Stage X"] = 7,
      };
      "Superheavy Shell Stage 1",
      "Superheavy Shell Stage 2",
      "Superheavy Shell Stage 3",
      "Superheavy Shell Stage 4",
      "Superheavy Shell Stage 5",
      "Superheavy Shell Stage 6",
      "Superheavy Shell Stage 7",
      "Superheavy Shell Stage X",
   },
   ["Vox"] = {
      probability = {
         ["Superheavy Shell Stage X"] = 7,
      };
      "Superheavy Shell Stage 1",
      "Superheavy Shell Stage 2",
      "Superheavy Shell Stage 3",
      "Superheavy Shell Stage 4",
      "Superheavy Shell Stage 5",
      "Superheavy Shell Stage 6",
      "Superheavy Shell Stage 7",
      "Superheavy Shell Stage X",
   },
}


-- Tables of available weapons by base type.
-- See equip_set function for more info.
equip_typeOutfits_weapons = {
   ["Hyena"] = {
      {
         num = 1;
         "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
      },
      {
         "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
         "Ion Cannon",
      },
   },
   ["Shark"] = {
      {
         num = 1;
         "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
         "Unicorp Mace Launcher", "TeraCom Mace Launcher",
      },
      {
         "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
         "Ion Cannon",
      },
   },
   ["Fidelity"] = {
      {
         num = 1;
         "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
         "Unicorp Mace Launcher", "TeraCom Mace Launcher",
      },
      {
         "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
         "Ion Cannon",
      },
   },
   ["Derivative"] = {
      {
         num = 1;
         "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
         "Unicorp Mace Launcher", "TeraCom Mace Launcher",
      },
      {
         "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
         "Ion Cannon",
      },
   },
   ["Vendetta"] = {
      {
         num = 2;
         "Laser Cannon MK2", "Razor MK2", "Vulcan Gun", "Plasma Blaster MK2",
         "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
         "Unicorp Mace Launcher", "TeraCom Mace Launcher", "Ion Cannon",
      },
      {
         num = 2;
         "Laser Cannon MK2", "Razor MK2", "Vulcan Gun", "Plasma Blaster MK2",
         "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
         "Unicorp Mace Launcher", "TeraCom Mace Launcher", "Ion Cannon",
      },
      {
         "Laser Cannon MK2", "Razor MK2", "Vulcan Gun", "Plasma Blaster MK2",
         "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
         "Unicorp Mace Launcher", "TeraCom Mace Launcher", "Ion Cannon",
      },
   },
   ["Vigilance"] = {
      {
         num = 2;
         "Railgun",
      },
      {
         num = 1;
         "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
      },
      {
         varied = true;
         "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
         "Heavy Ripper Cannon", "Mass Driver", "Plasma Cluster Cannon",
      },
   },
   ["Kestrel"] = {
      {
         "Railgun", "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
      },
      {
         "Enygma Systems Turreted Fury Launcher",
         "Enygma Systems Turreted Headhunter Launcher",
         "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
      },
   },
   ["Hawking"] = {
      {
         num = 1;
         "Enygma Systems Turreted Fury Launcher",
         "Enygma Systems Turreted Headhunter Launcher",
      },
      {
         "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
      },
   },
   ["Goddard"] = {
      {
         num = 1;
         "Enygma Systems Turreted Fury Launcher",
         "Enygma Systems Turreted Headhunter Launcher",
      },
      {
         "Railgun", "Heavy Laser Turret", "Railgun Turret",
      },
      {
         "TeraCom Mace Launcher",
      },
   },
   ["Brigand"] = {
      {
         "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
         "Unicorp Mace Launcher", "TeraCom Mace Launcher",
      },
      {
         varied = true, probability = {
            ["BioPlasma Stinger Stage X"] = 3,
         };
         "BioPlasma Stinger Stage 1", "BioPlasma Stinger Stage 2",
         "BioPlasma Stinger Stage 3", "BioPlasma Stinger Stage X",
      },
   },
   ["Reaver"] = {
      {
         "Unicorp Headhunter Launcher", "Unicorp Fury Launcher",
         "Unicorp Medusa Launcher",
      },
      {
         varied = true, probability = {
            ["BioPlasma Stinger Stage X"] = 4,
         };
         "BioPlasma Claw Stage 1", "BioPlasma Claw Stage 2",
         "BioPlasma Claw Stage 3", "BioPlasma Claw Stage 4",
         "BioPlasma Claw Stage X",
      },
      {
         varied = true, probability = {
            ["BioPlasma Stinger Stage X"] = 3,
         };
         "BioPlasma Stinger Stage 1", "BioPlasma Stinger Stage 2",
         "BioPlasma Stinger Stage 3", "BioPlasma Stinger Stage X",
      },
   },
   ["Marauder"] = {
      {
         varied = true;
         "TeraCom Fury Launcher", "TeraCom Medusa Launcher",
         "Unicorp Headhunter Launcher", "Unicorp Fury Launcher",
         "Unicorp Medusa Launcher",
      },
      {
         varied = true, probability = {
            ["BioPlasma Claw Stage X"] = 4,
         };
         "BioPlasma Claw Stage 1", "BioPlasma Claw Stage 2",
         "BioPlasma Claw Stage 3", "BioPlasma Claw Stage 4",
         "BioPlasma Claw Stage X",
      },
      {
         varied = true, probability = {
            ["BioPlasma Stinger Stage X"] = 3,
         };
         "BioPlasma Stinger Stage 1", "BioPlasma Stinger Stage 2",
         "BioPlasma Stinger Stage 3", "BioPlasma Stinger Stage X",
      },
   },
   ["Odium"] = {
      {
         varied = true;
         "Unicorp Fury Launcher", "Unicorp Headhunter Launcher",
         "Unicorp Medusa Launcher", "Unicorp Vengeance Launcher",
         "Enygma Systems Spearhead Launcher", "Unicorp Caesar IV Launcher",
         "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
         "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
         "TeraCom Imperator Launcher",
      },
      {
         varied = true, probability = {
            ["BioPlasma Fang Stage X"] = 5,
         };
         "BioPlasma Fang Stage 1", "BioPlasma Fang Stage 2",
         "BioPlasma Fang Stage 3", "BioPlasma Fang Stage 4",
         "BioPlasma Fang Stage 5", "BioPlasma Fang Stage X",
      },
      {
         varied = true, probability = {
            ["BioPlasma Claw Stage X"] = 4,
         };
         "BioPlasma Claw Stage 1", "BioPlasma Claw Stage 2",
         "BioPlasma Claw Stage 3", "BioPlasma Claw Stage 4",
         "BioPlasma Claw Stage X",
      },
   },
   ["Nyx"] = {
      {
         varied = true, probability = {
            ["BioPlasma Talon Stage X"] = 6,
         };
         "BioPlasma Talon Stage 1", "BioPlasma Talon Stage 2",
         "BioPlasma Talon Stage 3", "BioPlasma Talon Stage 4",
         "BioPlasma Talon Stage 5", "BioPlasma Talon Stage 6",
         "BioPlasma Talon Stage X",
      },
      {
         num = 1;
         "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
      },
      {
         "Plasma Cluster Cannon",
      },
      {
         varied = true, probability = {
            ["BioPlasma Claw Stage X"] = 4,
         };
         "BioPlasma Claw Stage 1", "BioPlasma Claw Stage 2",
         "BioPlasma Claw Stage 3", "BioPlasma Claw Stage 4",
         "BioPlasma Claw Stage X",
      },
   },
   ["Ira"] = {
      {
         num = 1;
         "TeraCom Fury Launcher", "Enygma Systems Turreted Fury Launcher",
         "TeraCom Headhunter Launcher",
         "Enygma Systems Turreted Headhunter Launcher",
      },
      {
         varied = true, probability = {
            ["BioPlasma Talon Stage X"] = 6,
         };
         "BioPlasma Talon Stage 1", "BioPlasma Talon Stage 2",
         "BioPlasma Talon Stage 3", "BioPlasma Talon Stage 4",
         "BioPlasma Talon Stage 5", "BioPlasma Talon Stage 6",
         "BioPlasma Talon Stage X",
      },
   },
   ["Arx"] = {
      {
         varied = true;
         "Soromid Brigand Fighter Bay",
      },
      {
         varied = true, probability = {
            ["BioPlasma Tentacle Stage X"] = 7,
         };
         "BioPlasma Tentacle Stage 1", "BioPlasma Tentacle Stage 2",
         "BioPlasma Tentacle Stage 3", "BioPlasma Tentacle Stage 4",
         "BioPlasma Tentacle Stage 5", "BioPlasma Tentacle Stage 6",
         "BioPlasma Tentacle Stage 7", "BioPlasma Tentacle Stage X",
      },
      {
         "Heavy Laser Turret", "Grave Beam",
      },
   },
   ["Vox"] = {
      {
         num = 1;
         "Enygma Systems Turreted Fury Launcher",
         "Enygma Systems Turreted Headhunter Launcher",
      },
      {
         varied = true, probability = {
            ["BioPlasma Tentacle Stage X"] = 7,
         };
         "BioPlasma Tentacle Stage 1", "BioPlasma Tentacle Stage 2",
         "BioPlasma Tentacle Stage 3", "BioPlasma Tentacle Stage 4",
         "BioPlasma Tentacle Stage 5", "BioPlasma Tentacle Stage 6",
         "BioPlasma Tentacle Stage 7", "BioPlasma Tentacle Stage X",
      },
   },
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
         "Cargo Pod", "Fuel Pod",
      },
   },
}


-- Table of available core systems by ship.
equip_shipOutfits_coreSystems = {
   ["Empire Lancelot"] = { "Milspec Orion 3701 Core System" },
   ["Sirius Fidelity"] = { "Milspec Orion 2301 Core System" },
   ["Za'lek Scout Drone"] = { "Milspec Orion 2301 Core System" },
   ["Za'lek Heavy Drone"] = {  "Milspec Orion 3701 Core System" },
   ["Za'lek Bomber Drone"] = { "Milspec Orion 3701 Core System" },
}


-- Table of available engines by ship.
equip_shipOutfits_engines = {
   ["Empire Lancelot"] = { "Tricon Zephyr II Engine" },
   ["Sirius Fidelity"] = { "Tricon Zephyr Engine" },
   ["Za'lek Scout Drone"] = { "Tricon Zephyr Engine" },
   ["Za'lek Heavy Drone"] = { "Tricon Zephyr II Engine" },
   ["Za'lek Bomber Drone"] = { "Tricon Zephyr II Engine" },
}


-- Table of available hulls by ship.
equip_shipOutfits_hulls = {
   ["Empire Lancelot"] = {
      "Nexus Light Stealth Plating", "S&K Light Combat Plating",
   },
   ["Sirius Fidelity"] = {
      "Nexus Light Stealth Plating", "S&K Ultralight Combat Plating",
   },
   ["Za'lek Scout Drone"] = { "Nexus Light Stealth Plating" },
   ["Za'lek Heavy Drone"] = { "S&K Light Combat Plating" },
   ["Za'lek Bomber Drone"] = { "Nexus Light Stealth Plating" },
}


-- Tables of available weapons by ship.
-- See equip_set function for more info.
equip_shipOutfits_weapons = {
   ["Empire Lancelot"] = {
      {
         num = 1;
         "Unicorp Fury Launcher", "Unicorp Headhunter Launcher",
         "TeraCom Fury Launcher",
      },
      {
         "Laser Cannon MK2", "Plasma Blaster MK2",
      },
   },
   ["Sirius Fidelity"] = {
      {
         num = 1;
         "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
      },
      {
         "Razor MK1", "Ion Cannon",
      },
   },
   ["Za'lek Scout Drone"] = {
      {
         "Particle Lance",
      },
   },
   ["Za'lek Light Drone"] = {
      {
         "Particle Lance",
      },
   },
   ["Za'lek Heavy Drone"] = {
      {
         num = 2;
         "Orion Lance"
      },
      {
         num = 1;
         "Electron Burst Cannon"
      }
   },
   ["Za'lek Bomber Drone"] = {
      {
         num = 2;
         "Electron Burst Cannon"
      },
      {
         "Particle Lance"
      },
   },
}


-- Tables of available utilities by ship.
-- See equip_set function for more info.
equip_shipOutfits_utilities = {}

-- Tables of available structurals by ship.
-- See equip_set function for more info.
equip_shipOutfits_structurals = {}

equip_nocores = {
   ["Za'lek Scout Drone"] = true,
   ["Za'lek Light Drone"] = true,
   ["Za'lek Heavy Drone"] = true,
   ["Za'lek Bomber Drone"] = true,
}


--[[
-- @brief Wrapper for pilot.addOutfit that prints a warning if no outfits added.
--]]
function equip_warn( p, outfit, q, bypass )
   q = q or 1
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
            chance = probability[ choice ] or 0
            -- Starting at 2 because the first one is already in the table.
            for j=2,chance do
               choices[ #choices + 1 ] = choice
            end
         end
      end

      c = rnd.rnd( 1, #choices )
      i = 1
      while #choices > 0 and (num == nil or i <= num) do
         i = i + 1

         -- If varied we switch up the outfit next iteration
         if varied then c = rnd.rnd( 1, #choices ) end

         equipped = p:addOutfit( choices[c] )
         -- Failed to equip
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

   if not equip_nocores[shipname] then
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
         equip_warn( p, "Unicorp PT-16 Core System" )
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
         equip_warn( p, "Unicorp Hawk 350 Engine" )
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

   -- Fill ammo
   p:fillAmmo()

   -- Add cargo
   local pb = equip_classCargo[class]
   if pb == nil then
      warn( string.format(
            "Class %s not handled by equip_classCargo in equip script",
            class ) )
      return
   end

   if rnd.rnd() < pb then
      local avail_cargo = commodity.getStandard()

      if #avail_cargo > 0 then
         for i=1,rnd.rnd(1,3) do
            local ncargo = rnd.rnd( 0, math.floor(p:cargoFree()*pb) )
            p:cargoAdd( avail_cargo[ rnd.rnd( 1, #avail_cargo ) ]:nameRaw(), ncargo )
         end
      end
   end
end
