local mt = require "merge_tables"
local fmt = require "format"

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
      shipvar = "cannibal",
      desc = _("The ship is able to cannibalize boarded vessels to restore armour. For every 2 points of armour cannibalized, the ship will gain a single point of armour."),
   },
   ["bite2"] = {
      --name = _("Cannibalism II"),
      name = _("Cannibal II"),
      tier = 2,
      requires = { "bite1" },
      shipvar = "cannibal2",
      desc = _("Cannibalizing boarded ships will now restore 2 points of armour per 3 points of armour cannibalized, and boarding will cause your ship to perform a full cooldown cycle."),
   },
   ["bite3"] = {
      name = _("The Bite"),
      tier = 3,
      outfit = "The Bite",
      slot = "thebite",
      requires = { "bite2" },
      desc = function( p )
         local dmg = 4*math.pow(p:mass(), 0.7)
         return fmt.f(_("The ship will lunge at the target enemy and take a huge bite out of it. +800% thrust and +50% absorb for 3 seconds or until target ship is bitten. This ship will do {dmg:.0f} damage with its current mass. Has a 15 second cooldown period."),{dmg=dmg})
      end,
   },
   ["bite4"] = {
      name = _("Blood Lust"),
      tier = 4,
      requires = {"bite3"},
      desc = _("more range"),
      -- TODO
   },
   ["bite5"] = {
      name = _("Strong Jaws"),
      tier = 5,
      requires = {"bite4"},
      desc = _("more damage"),
      -- TODO
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
      -- TODO
   },
   ["move2"] = {
      name = _("Adrenal Gland II"),
      tier = 2,
      requires = { "move1" },
      replaces = "move1",
      slot = "adrenal",
      desc = _("improved afterburner"),
      -- TODO
   },
   ["move3"] = {
      name = _("Wanderer"),
      tier = 3,
      requires = { "move2" },
      desc = _("Gives a 50% thrust bonus."),
      outfit = "Wanderer",
   },
   ["move4"] = {
      name = _("Adrenal Gland III"),
      tier = 4,
      requires = { "move3" },
      replaces = "move2",
      slot = "adrenal",
      desc = _("+ time slowdown and mass limit")
      -- TODO
   },
}

-- Stealth skill tree
-- For up to destroyer
-- Uses intrinsics only
skills.set.stealth = {
   ["stealth1"] = {
      name = _("Compound Eyes"),
      tier = 1,
      desc = _("Gives a +15% detection bonus."),
      outfit = "Compound Eyes",
   },
   ["stealth2"] = {
      name =_("Hunter's Instinct"),
      tier = 2,
      requires = { "stealth1" },
      desc = _("Gives a +40% speed bonus when stealthed."),
      --outfit = "Hunter's Instinct",
      -- TODO
   },
   ["stealth3"] = {
      name =_("Ambush Hunter"),
      tier = 3,
      requires = { "stealth2" },
      desc = _("temporary damage bonus after stealth"),
      -- TODO
   },
   ["stealth4"] = {
      name =_("Silence"),
      tier = 4,
      requires = { "stealth3" },
      desc = _("increased stealth"),
      outfit = "Silence",
   },
}

-- Health tree
-- For everyone!
-- Uses intrinsics only
skills.set.health = {
   ["health1"] = {
      name = _("Bulky Abdomen"),
      tier = 1,
      desc = _("Provides an additional 50 armour and 20% armour bonus."),
      outfit = "Bulky Abdomen",
   },
   ["health2"] = {
      --name = _("Regeneration I"),
      name = _("Regen I"),
      tier = 2,
      requires = { "health1" },
      desc = _("Gives 5 armour regeneration."),
      outfit = "Regeneration I",
   },
   ["health3"] = {
      name = _("Hard Shell"),
      tier = 3,
      requires = { "health2" },
      desc = _("Gives a 10% absorption bonus."),
      outfit = "Hard Shell",
   },
   ["health4"] = {
      --name = _("Regeneration II"),
      name = _("Regen II"),
      tier = 4,
      requires = { "health3" },
      desc = _("Gives 5 additional armour regeneration."),
      outfit = "Regeneration II",
   },
   ["health5"] = {
      name = _("Reflective Shell"),
      tier = 5,
      requires = { "health4" },
      desc = _("damage reflection?"),
      -- TODO
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
      -- TODO
   },
   ["attack2"] = {
      name = _("Adrenaline Hormones"),
      tier = 2,
      requires = { "attack1" },
      desc = _("Gives a 8% bonus to weapon fire rate."),
      outfit = "Adrenaline Hormones",
   },
   ["attack3"] = {
      name = _("Feral Rage II"),
      tier = 3,
      requires = { "attack2" },
      --replaces = "attack1",
      desc = _("movement bonus to feral rage"),
      -- TODO
   },
   ["attack4"] = {
      name = _("Antenna Sensitivity"),
      tier = 4,
      requires = { "attack3" },
      desc = _("tracking bonus"),
      -- TODO
   },
   ["attack5"] = {
      name = _("Feral Rage III"),
      tier = 5,
      requires = { "attack4" },
      --replaces = "attack3",
      desc = _("feral rage becomes a triggerable skill too"),
      -- TODO
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
      -- TODO
   },
   ["plasma2"] = {
      name = _("Paralyzing Plasma"),
      tier = 2,
      requires = { "plasma1" },
      desc = _("plasma burns slow down enemies"),
      -- TODO
   },
   ["plasma3"] = {
      name = _("Crippling Plasma"),
      tier = 3,
      requires = { "plasma2" },
      desc = _("plasma burns lower enemy fire rate"),
      -- TODO
   },
   ["plasma4"] = {
      name = _("Corrosion II"),
      tier = 4,
      requires = { "plasma3" },
      desc = _("further increases plasma burn effect"),
      -- TODO
   },
   ["plasma5"] = {
      name = _("Plasma Burst"),
      tier = 5,
      requires = { "plasma4" },
      desc = _("creates an explosion of plasma affecting all ships around the pilot"),
      -- TODO
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

-- Brigand intrinsics
skills.ship["Soromid Brigand"] = {
   {
      name = _("Innate"),
      stage = 0,
      outfit = {
         "Perleve Cerebrum I",
         "Perlevis Cortex I",
         "Perlevis Gene Drive I",
         "Pincer Organ I",
         "Pincer Organ I",
      },
      slot = {
         "brain",
         "shell",
         "genedrive",
         "rightweap",
         "leftweap",
      },
   },
   {
      name = _("Cortex Growth I"),
      stage = 1,
      outfit = "Perlevis Cortex II",
      slot = "shell",
   },
   {
      name = _("Gene Drive Growth I"),
      stage = 2,
      outfit = "Perlevis Gene Drive II",
      slot = "genedrive",
   },
   {
      name = _("Weapon Organ Growth I"),
      stage = 3,
      outfit = {
         "Pincer Organ II",
         "Pincer Organ II",
      },
      slot = {
         "rightweap",
         "leftweap",
      },
   },
   {
      name = _("Cerebrum Growth I"),
      stage = 4,
      outfit = "Perleve Cerebrum II",
      slot = "brain",
   },
}

-- Arx Intrinsics
skills.ship["Soromid Arx"] = {
   {
      name = _("Innate"),
      outfit = {
         "Immanis Gene Drive I",
         "Immanis Cortex I",
         "Immane Cerebrum I",
         "Pincer Organ I",
         "Pincer Organ I",
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
      name = _("Cortex Growth I"),
      outfit = "Immanis Cortex II",
      slot = "shell",
   },
   {
      name = _("Gene Drive Growth I"),
      outfit = "Immanis Gene Drive II",
      slot = "genedrive",
   },
   {
      name = _("Weapon Organ Growth I"),
      outfit = {
         "Pincer Organ II",
         "Pincer Organ II",
      },
      slot = {
         "rightweap",
         "leftweap",
      },
   },
   {
      name = _("Cerebrum Growth I"),
      outfit = "Immane Cerebrum II",
      slot = "brain",
   },
   {
      name = _("Cortex Growth II"),
      outfit = "Immanis Cortex III",
      slot = "shell",
   },
   {
      name = _("Weapon Organ Growth II"),
      outfit = {
         "Pincer Organ II",
         "Pincer Organ II",
      },
      slot = {
         "rightweap",
         "leftweap",
      },
   },
   {
      name = _("Gene Drive Growth II"),
      outfit = "Immanis Gene Drive III",
      slot = "genedrive",
   },
   {
      name = _("Cerebrum Growth II"),
      outfit = "Immane Cerebrum III",
      slot = "brain",
   },
   {
      name = _("Weapon Organ Growth III"),
      outfit = {
         "Pincer Organ II",
         "Pincer Organ II",
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
