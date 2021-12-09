local mt = require "merge_tables"

local skills = {
   set = {},
}

-- The Bite skill tree
skills.set.bite = {
   ["bite1"] = {
      name = _("Cannibalism I"),
      tier = 1,
      desc = _("The ship is able to cannibalize boarded vessels to restore armour. For every 2 points of armour cannibalized, the ship will gain a single point of armour."),
   },
   ["bite2"] = {
      name = _("Cannibalism II"),
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
      name = _("Regeneration I"),
      tier = 2,
      requires = { "health1" },
   },
   ["health3"] = {
      name = _("Hard Shell"),
      tier = 3,
      requires = { "health2" },
   },
   ["health4"] = {
      name = _("Regeneration II"),
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

skills.misc = {
}

function skills.get( sets )
   local s = {}
   for k,v in ipairs(sets) do
      mt.merge_tables_recursive( s, skills.set[v] )
   end
   return s
end

return skills

--[=[
local skills = {
   -- Core Gene Drives
   ["engines1"] = {
      name = _("Gene Drive I"),
      tier = 0,
      outfit = "Ultralight Fast Gene Drive Stage 1",
      slot = "genedrive",
   },
   ["engines2"] = {
      name = _("Gene Drive II"),
      tier = 2,
      requires = { "engines1" },
      outfit = "Ultralight Fast Gene Drive Stage 2",
      slot = "genedrive",
   },
   ["engines3"] = {
      name = _("Gene Drive III"),
      tier = 4,
      requires = { "engines2" },
      outfit = "Ultralight Fast Gene Drive Stage X",
      slot = "genedrive",
   },
   ["engines4"] = {
      name = _("Gene Drive IV"),
      tier = 6,
      requires = { "engines3" },
      outfit = "Ultralight Fast Gene Drive Stage X",
      slot = "genedrive",
   },
   -- Core Brains
   ["systems1"] = {
      name = _("Brain Stage I"),
      tier = 0,
      outfit = "Ultralight Brain Stage 1",
      slot = "brain",
   },
   ["systems2"] = {
      name = _("Brain Stage II"),
      tier = 2,
      requires = { "systems1" },
      outfit = "Ultralight Brain Stage 2",
      slot = "brain",
   },
   ["systems3"] = {
      name = _("Brain Stage III"),
      tier = 4,
      requires = { "systems2" },
      outfit = "Ultralight Brain Stage X",
      slot = "brain",
   },
   ["systems4"] = {
      name = _("Brain Stage IV"),
      tier = 6,
      requires = { "systems3" },
      outfit = "Ultralight Brain Stage X",
      slot = "brain",
   },
   -- Core Shells
   ["hull1"] = {
      name = _("Shell Stage I"),
      tier = 0,
      outfit = "Ultralight Shell Stage 1",
      slot = "shell",
   },
   ["hull2"] = {
      name = _("Shell Stage II"),
      tier = 2,
      requires = { "hull1" },
      outfit = "Ultralight Shell Stage 2",
      slot = "shell",
   },
   ["hull3"] = {
      name = _("Shell Stage III"),
      tier = 4,
      requires = { "hull2" },
      outfit = "Ultralight Shell Stage X",
      slot = "shell",
   },
   ["hull4"] = {
      name = _("Shell Stage IV"),
      tier = 6,
      requires = { "hull3" },
      outfit = "Ultralight Shell Stage X",
      slot = "shell",
   },
   -- Right Weapon
   ["weap2a1"] = {
      name = _("Right Stinger I"),
      tier = 1,
      --conflicts = { "weap2b1" },
      slot = "rightweap",
      outfit = "BioPlasma Stinger Stage 1",
   },
   ["weap2a2"] = {
      name = _("Right Stinger I"),
      tier = 3,
      requires = { "weap2a1" },
      slot = "rightweap",
      outfit = "BioPlasma Stinger Stage X",
   },
   --[[
   ["weap2b1"] = {
      name = _("Right Claw I"),
      tier = 1,
      slot = "rightweap",
      outfit = "BioPlasma Claw Stage 1",
   },
   ["weap2b2"] = {
      name = _("Right Claw II"),
      tier = 3,
      requires = { "weap2b1" },
      slot = "rightweap",
      outfit = "BioPlasma Claw Stage X",
   },
   --]]
   -- Left Weapon
   ["weap1a1"] = {
      name = _("Left Stinger I"),
      tier = 1,
      --conflicts = { "weap1b1" },
      slot = "leftweap",
      outfit = "BioPlasma Stinger Stage 1",
   },
   ["weap1a2"] = {
      name = _("Left Stinger I"),
      tier = 3,
      requires = { "weap1a1" },
      slot = "leftweap",
      outfit = "BioPlasma Stinger Stage X",
   },
   --[[
   ["weap1b1"] = {
      name = _("Left Claw I"),
      tier = 1,
      slot = "leftweap",
      outfit = "BioPlasma Claw Stage 1",
   },
   ["weap1b2"] = {
      name = _("Left Claw II"),
      tier = 3,
      requires = { "weap1b1" },
      slot = "leftweap",
      outfit = "BioPlasma Claw Stage X",
   },
   --]]
   -- Movement Line
   -- Health Line
   -- Attack Line
}
--]=]
