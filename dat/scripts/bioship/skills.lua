local mt = require "merge_tables"

local skills = {
   set = {},
   ship = {},
}

-- The Bite skill tree
-- For everyone
-- Needs bite slot
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
      slot = "thebite",
      requires = { "bite2" },
      desc = _("The ship will lunge at the target enemy and take a huge bite out of it. +200% thrust, +50% absorb for 3 seconds or until target ship is bitten. Damage is based on ship's mass and half of armour damage done will be restored."),
   },
   ["bite4"] = {
      name = _("Blood Lust"),
      tier = 4,
      requires = {"bite3"},
      desc = _("more range"),
   },
   ["bite5"] = {
      name = _("Strong Jaws"),
      tier = 5,
      requires = {"bite4"},
      desc = _("more damage"),
   },
}

-- Movement skill tree
-- Only for up to destroyers
-- Needs adrenal gland slot
skills.set.move = {
   ["move1"] = {
      name = _("Adrenal Gland I"),
      tier = 1,
      slot = "adrenal",
      desc = _("weak afterburner"),
   },
   ["move2"] = {
      name = _("Adrenal Gland II"),
      tier = 2,
      requires = { "move1" },
      replaces = "move1",
      slot = "adrenal",
      desc = _("improved afterburner"),
   },
   ["move3"] = {
      name = _("Wanderer"),
      tier = 3,
      requires = { "move2" },
      desc = _("thrust bonus"),
   },
   ["move4"] = {
      name = _("Adrenal Gland III"),
      tier = 4,
      requires = { "move3" },
      replaces = "move2",
      slot = "adrenal",
      desc = _("+ time slowdown and mass limit")
   },
}

-- Stealth skill tree
-- For up to destroyer
-- Uses intrinsics only
skills.set.stealth = {
   ["stealth1"] = {
      name = _("Compound Eyes"),
      tier = 1,
      desc = _("detection bonus"),
   },
   ["stealth2"] = {
      name =_("Hunter's Instinct"),
      tier = 2,
      requires = { "stealth1" },
      desc = _("stealh move bonus"),
   },
   ["stealth3"] = {
      name =_("Ambush Hunter"),
      tier = 3,
      requires = { "stealth2" },
      desc = _("temporary damage bonus after stealth"),
   },
   ["stealth4"] = {
      name =_("Silence"),
      tier = 4,
      requires = { "stealth3" },
      desc = _("increased stealth"),
   },
}

-- Health tree
-- For everyone!
-- Uses intrinsics only
skills.set.health = {
   ["health1"] = {
      name = _("Bulky Abdomen"),
      tier = 1,
      desc = _("armour bonus"),
   },
   ["health2"] = {
      --name = _("Regeneration I"),
      name = _("Regen I"),
      tier = 2,
      requires = { "health1" },
      desc = _("shield regen"),
   },
   ["health3"] = {
      name = _("Hard Shell"),
      tier = 3,
      requires = { "health2" },
      desc = _("armour absorption"),
   },
   ["health4"] = {
      --name = _("Regeneration II"),
      name = _("Regen II"),
      tier = 4,
      requires = { "health3" },
      desc = _("more shield regen"),
   },
   ["health5"] = {
      name = _("Reflective Shell"),
      tier = 5,
      requires = { "health4" },
      desc = _("damage reflection?"),
   },
}

-- Attack tree
-- For everyone!
-- Uses feral rage slot (only for ships that get tier 5)
skills.set.attack = {
   ["attack1"] = {
      name = _("Feral Rage I"),
      tier = 1,
      desc = _("Temporary damage bonus on armour damage"),
   },
   ["attack2"] = {
      name = _("Adrenaline Hormones"),
      tier = 2,
      requires = { "attack1" },
      desc = _("fixed fire rate bonus"),
   },
   ["attack3"] = {
      name = _("Feral Rage II"),
      tier = 3,
      requires = { "attack2" },
      --replaces = "attack1",
      desc = _("movement bonus to feral rage"),
   },
   ["attack4"] = {
      name = _("Antenna Sensitivity"),
      tier = 4,
      requires = { "attack3" },
      desc = _("tracking bonus"),
   },
   ["attack5"] = {
      name = _("Feral Rage III"),
      tier = 5,
      requires = { "attack4" },
      --replaces = "attack3",
      desc = _("feral rage becomes a triggerable skill too"),
   },
}

-- Plasma tree
-- For everyone!
-- Requires plasma slot for tier 5
skills.set.plasma = {
   ["plasma1"] = {
      name = _("Corrosion I"),
      tier = 1,
      desc = _("increases plasma burn effect (more damage over longer time)"),
   },
   ["plasma2"] = {
      name = _("Paralyzing Plasma"),
      tier = 2,
      requires = { "plasma1" },
      desc = _("plasma burns slow down enemies"),
   },
   ["plasma3"] = {
      name = _("Crippling Plasma"),
      tier = 3,
      requires = { "plasma2" },
      desc = _("plasma burns lower enemy fire rate"),
   },
   ["plasma4"] = {
      name = _("Corrosion II"),
      tier = 4,
      requires = { "plasma3" },
      desc = _("further increases plasma burn effect"),
   },
   ["plasma5"] = {
      name = _("Plasma Burst"),
      tier = 5,
      requires = { "plasma4" },
      desc = _("creates an explosion of plasma affecting all ships around the pilot"),
   },
}

-- Misc tree
-- For everyone!
-- All intrinsics
skills.set.misc = {
   ["misc1"] = {
      name = _("Cargo Sacs"),
      tier = 1,
      desc = _("cargo bonus"),
   },
   ["misc2"] = {
      name = _("Fuel Bladder"),
      tier = 2,
      requires = { "misc1" },
      desc = _("fuel bonus"),
   },
   ["misc3"] = {
      name = _("Adaptive Jump"),
      tier = 3,
      requires = { "misc2" },
      desc = _("jump delay bonus"),
   },
   ["misc4"] = {
      name = _("Enhanced Smell"),
      tier = 4,
      requires = { "misc3" },
      desc = _("looting bonus"),
   },
   ["misc5"] = {
      name = _("TODO"),
      tier = 5,
      requires = { "misc4" },
      desc = _("TODO"),
   },
}

-- Brigand Intrinsics
skills.ship["Soromid Brigand"] = {
   {
      name = _("Innate"),
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
   {
      name = _("Shell Growth I"),
      stage = 1,
      outfit = "Ultralight Shell Stage X",
      slot = "shell",
   },
   {
      name = _("Gene Drive Growth I"),
      stage = 2,
      outfit = "Ultralight Fast Gene Drive Stage X",
      slot = "genedrive",
   },
   {
      name = _("Weapon Organ Growth I"),
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
   {
      name = _("Brain Growth I"),
      stage = 4,
      outfit = "Ultralight Brain Stage X",
      slot = "brain",
   },
   {
      name = _("Weapon Organ Growth II"),
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
