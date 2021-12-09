local mt = require "merge_tables"

local skills = {
   set = {},
   ship = {},
}

-- The Bite skill tree
skills.set.bite = {
   ["bite1"] = {
      --name = _("Cannibalism I"),
      name = _("Cannibal I"),
      tier = 1,
      desc = _("The ship is able to cannibalize boarded vessels to restore armour. For every 2 points of armour cannibalized, the ship will gain a single point of armour."),
   },
   ["bite2"] = {
      --name = _("Cannibalism II"),
      name = _("Cannibal II"),
      tier = 2,
      requires = { "bite1" },
      desc = _("Cannibalizing boarded ships will now restore 2 points of armour per 3 points of armour cannibalized, and will also similarly restore energy."),
   },
   ["bite3"] = {
      name = _("The Bite"),
      tier = 3,
      requires = { "bite2" },
      desc = _("The ship will lunge at the target enemy and take a huge bite out of it. +200% thrust, +50% absorb for 3 seconds or until target ship is bitten. Damage is based on ship's mass and half of armour damage done will be restored."),
   },
   ["bite4"] = {
      name = _("Blood Lust"),
      tier = 4,
      requires = {"bite3"},
   },
   ["bite5"] = {
      name = _("Strong Jaws"),
      tier = 5,
      requires = {"bite4"},
   },
}

-- Movement skill tree
skills.set.move = {
   ["move1"] = {
      name = _("Adrenal Gland I"),
      tier = 1,
   },
   ["move2"] = {
      name = _("Adrenal Gland II"),
      tier = 2,
      requires = { "move1" },
   },
   ["move3"] = {
      name = _("Wanderer"),
      tier = 3,
      requires = { "move2" },
   },
   ["move4"] = {
      name = _("Adrenal Gland III"),
      tier = 4,
      requires = { "move3" },
   },
}

-- Stealth skill tree
skills.set.stealth = {
   ["stealth1"] = {
      name = _("Compound Eyes"),
      tier = 1,
   },
   ["stealth2"] = {
      name =_("Hunter's Instinct"),
      tier = 2,
      requires = { "stealth1" },
   },
   ["stealth3"] = {
      name =_("Ambush Hunter"),
      tier = 3,
      requires = { "stealth2" },
   },
   ["stealth4"] = {
      name =_("Silence"),
      tier = 4,
      requires = { "stealth3" },
   },
}

skills.set.health = {
   ["health1"] = {
      name = _("Bulky Abdomen"),
      tier = 1,
   },
   ["health2"] = {
      --name = _("Regeneration I"),
      name = _("Regen I"),
      tier = 2,
      requires = { "health1" },
   },
   ["health3"] = {
      name = _("Hard Shell"),
      tier = 3,
      requires = { "health2" },
   },
   ["health4"] = {
      --name = _("Regeneration II"),
      name = _("Regen II"),
      tier = 4,
      requires = { "health3" }
   },
   ["health5"] = {
      name = _("Reflective Shell"),
      tier = 5,
      requires = { "health4" }
   },
}

skills.set.attack = {
   ["attack1"] = {
      name = _("Feral Rage I"),
      tier = 1,
   },
   ["attack2"] = {
      name = _("Adrenaline Hormones"),
      tier = 2,
      requires = { "attack1" },
   },
   ["attack3"] = {
      name = _("Feral Rage II"),
      tier = 3,
      requires = { "attack2" },
   },
   ["attack4"] = {
      name = _("Antenna Sensitivity"),
      tier = 4,
      requires = { "attack3" },
   },
   ["attack5"] = {
      name = _("Feral Rage III"),
      tier = 5,
      requires = { "attack4" },
   },
}

skills.set.misc = {
   ["misc1"] = {
      name = _("Cargo Sacs"),
      tier = 1,
   },
   ["misc2"] = {
      name = _("Fuel Bladder"),
      tier = 2,
      requires = { "misc1" },
   },
   ["misc3"] = {
      name = _("Adaptive Jump"),
      tier = 3,
      requires = { "misc2" },
   },
   ["misc4"] = {
      name = _("Enhanced Smell"),
      tier = 4,
      requires = { "misc3" },
   },
   ["misc5"] = {
      name = _("TODO"),
      tier = 5,
      requires = { "misc4" },
   },
}

skills.ship["Soromid Brigand"] = {
   ["core"] = {
      stage = 0,
      outfit = {
         "Ultralight Fast Gene Drive Stage 1",
         "Ultralight Shell Stage 1",
         "Ultralight Brain Stage 1",
         "BioPlasma Stinger Stage 1",
         "BioPlasma Stinger Stage 1",
      },
      slot = {
         "genedrive",
         "shell",
         "brain",
         "rightweap",
         "leftweap",
      },
   },
   ["hull2"] = {
      stage = 1,
      outfit = "Ultralight Shell Stage X",
      slot = "shell",
   },
   ["engine2"] = {
      stage = 2,
      outfit = "Ultralight Fast Gene Drive Stage X",
      slot = "genedrive",
   },
   ["weapons2"] = {
      stage = 3,
      outfit = {
         "BioPlasma Stinger Stage 2",
         "BioPlasma Stinger Stage 2",
      },
      slot = {
         "rightweap",
         "leftweap",
      },
   },
   ["systems2"] = {
      stage = 4,
      outfit = "Ultralight Brain Stage X",
      slot = "brain",
   },
   ["weapons3"] = {
      stage = 5,
      outfit = {
         "BioPlasma Stinger Stage X",
         "BioPlasma Stinger Stage X",
      },
      slot = {
         "rightweap",
         "leftweap",
      },
   },
}

function skills.get( sets )
   local s = {}
   for k,v in ipairs(sets) do
      mt.merge_tables_recursive( s, skills.set[v] )
   end
   return s
end

return skills
