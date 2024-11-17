local fmt = require "format"
local luatk = require "luatk"

local skills = {
   set = {},
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
      icon = "food-chain.webp",
   },
   ["bite2"] = {
      name = _("The Bite"),
      tier = 2,
      outfit = "The Bite",
      slot = "the_bite",
      requires = { "bite1" },
      desc = function( p )
         local dmg = 10*math.sqrt(p:mass())
         return fmt.f(_("The ship will lunge at the target enemy and take a huge bite out of it. +800% accel and +30% absorb for 3 seconds or until target ship is bitten. This ship will do {dmg:.0f} damage with its current mass. Has a 15 second cooldown period."),{dmg=dmg})
      end,
      icon = "fangs.webp",
   },
   ["bite3"] = {
      --name = _("Cannibalism II"),
      name = _("Cannibal II"),
      tier = 3,
      requires = { "bite2" },
      shipvar = "cannibal2",
      desc = _("Cannibalizing boarded ships will now restore 2 points of armour per 3 points of armour cannibalized, and boarding will cause your ship to perform a full cooldown cycle."),
      icon = "food-chain.webp",
   },
   ["bite4"] = {
      name = _("Blood Lust"),
      tier = 4,
      requires = {"bite3"},
      desc = _("Lunge time increased to 5 seconds. On successful bite, weapon damage is increased by 25% for 10 seconds."),
      outfit = "The Bite - Blood Lust",
      slot = "the_bite",
      icon = "delighted.webp",
   },
   ["bite5"] = {
      name = _("Strong Jaws"),
      tier = 5,
      requires = {"bite4"},
      desc = _("Bite damage increased by 50%, and 25% of bitten armour is restored to the ship."),
      outfit = "The Bite - Improved",
      slot = "the_bite",
      icon = "gluttonous-smile.webp",
   },
}

-- Movement skill tree
-- Only for up to destroyers
-- Needs adrenal gland slot
skills.set.move = {
   ["move1"] = {
      name = _("Adrenal Gland I"),
      tier = 1,
      slot = "adrenal_gland",
      outfit = "Adrenal Gland I",
      desc = _("The ship is able to generate an overflow of adrenaline from energy, allowing for accelerated motion for a short time."),
      test = function( p )
         for k,o in ipairs(p:outfitsList()) do
            if o:typeBroad()=="Afterburner" then
               --if luatk.yesno( _("Remove Afterburner?"), _("Your ship already has an afterburner. Remove to learn the new skill?")A)
               luatk.msg(_("Afterburner"),_("Your ship already has an afterburner equipped! Please remove to be able to learn this skill."))
               return false
            end
         end
         return true
      end,
      icon = "lightning-electron.webp",
   },
   ["move2"] = {
      name = _("Adrenal Gland II"),
      tier = 2,
      requires = { "move1" },
      replaces = "move1",
      slot = "adrenal_gland",
      outfit = "Adrenal Gland II",
      desc = _("The adrenal gland is improved to both last longer and have a stronger effect, although at the cost of more energy usage."),
      icon = "lightning-electron.webp",
   },
   ["move3"] = {
      name = _("Wanderer"),
      tier = 3,
      requires = { "move2" },
      desc = _("Gives a 50% accel bonus."),
      outfit = "Wanderer",
      icon = "manta-ray.webp",
   },
   ["move4"] = {
      name = _("Adrenal Gland III"),
      tier = 4,
      requires = { "move3" },
      replaces = "move2",
      slot = "adrenal_gland",
      outfit = "Adrenal Gland III",
      desc = _("The intense adrenaline is also able to create an accelerated sense of time."),
      icon = "lightning-electron.webp",
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
      icon = "hunter-eyes.webp",
   },
   ["stealth2"] = {
      name =_("Hunter's Instinct"),
      tier = 2,
      requires = { "stealth1" },
      desc = _("Gives a +40% speed bonus when stealthed."),
      outfit = "Hunter's Instinct",
      icon = "hidden.webp",
   },
   ["stealth3"] = {
      name =_("Ambush Hunter"),
      tier = 3,
      requires = { "stealth2" },
      desc = _("+50% damage with weapons for 10 seconds after destealthing."),
      outfit = "Ambush Hunter",
      icon = "hidden.webp",
   },
   ["stealth4"] = {
      name =_("Silence"),
      tier = 4,
      requires = { "stealth3" },
      desc = _("Gives a 15% bonus to stealth."),
      outfit = "Silence",
      icon = "chameleon-glyph.webp",
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
      icon = "boar.webp",
   },
   ["health2"] = {
      name = _("Natural Healing I"),
      tier = 2,
      requires = { "health1" },
      desc = _("Gives 5 armour regeneration."),
      outfit = "Natural Healing I",
      icon = "rod-of-asclepius.webp",
   },
   ["health3"] = {
      name = _("Hard Shell"),
      tier = 3,
      requires = { "health2" },
      desc = _("Gives a 10% absorption bonus."),
      outfit = "Hard Shell",
      icon = "stegosaurus-scales.webp",
   },
   ["health4"] = {
      name = _("Natural Healing II"),
      tier = 4,
      requires = { "health3" },
      desc = _("Gives 5 additional armour regeneration."),
      outfit = "Natural Healing II",
      icon = "rod-of-asclepius.webp",
   },
   ["health5"] = {
      name = _("Reflective Shell"),
      tier = 5,
      requires = { "health4" },
      desc = _("Reflects 5% of damage back to attackers. Reflected damage is affected by turret damage bonus."),
      outfit = "Reflective Shell",
      icon = "spiked-shell.webp",
   },
}

-- Attack tree
-- For everyone!
-- Uses feral rage slot (only for ships that get tier 5)
skills.set.attack = {
   ["attack1"] = {
      name = _("Feral Rage I"),
      tier = 1,
      slot = "feral_rage",
      outfit = "Feral Rage I",
      desc = _("Upon receiving armour damage, the bioship enters a short 5 second trance during which forward and turret weapon damage is increased by 20%."),
      icon = "oni.webp",
   },
   ["attack2"] = {
      name = _("Adrenaline Hormones"),
      tier = 2,
      requires = { "attack1" },
      desc = _("Gives a 8% bonus to both turret and forward weapon fire rate."),
      outfit = "Adrenaline Hormones",
      icon = "internal-organ.webp",
   },
   ["attack3"] = {
      name = _("Feral Rage II"),
      tier = 3,
      requires = { "attack2" },
      slot = "feral_rage",
      outfit = "Feral Rage II",
      desc = _("The damage bonus is increased to 25% and the ship gains 25% speed and accel bonuses, and a 15% turning bonus."),
      icon = "oni.webp",
   },
   ["attack4"] = {
      name = _("Antenna Sensitivity"),
      tier = 4,
      requires = { "attack3" },
      desc = _("Gives a 30% tracking bonus."),
      outfit = "Tracking Antennae",
      icon = "long-antennae-bug.webp",
   },
   ["attack5"] = {
      name = _("Feral Rage III"),
      tier = 5,
      requires = { "attack4" },
      slot = "feral_rage",
      outfit = "Feral Rage III",
      desc = _("The state of feral rage duration is increased to 7 seconds with a 30% damage bonus, and can be triggered manually with a 30 second cooldown.."),
      icon = "oni.webp",
   },
}

-- Plasma tree
-- For everyone!
-- Requires plasma slot for tier 5
skills.set.plasma = {
   ["plasma1"] = {
      name = _("Corrosion I"),
      tier = 1,
      outfit = "Corrosion I",
      desc = _("Plasma burn duration increased by 50%. Bonus is halved for non-organ weapons."),
      icon = "acid-blob.webp",
   },
   ["plasma2"] = {
      name = _("Paralyzing Plasma"),
      tier = 2,
      outfit = "Paralyzing Plasma",
      requires = { "plasma1" },
      desc = _("Plasma burn slows enemy speed, accel, and turn by 25%."),
      icon = "chemical-bolt.webp",
   },
   ["plasma3"] = {
      name = _("Crippling Plasma"),
      tier = 3,
      outfit = "Crippling Plasma",
      requires = { "plasma2" },
      desc = _("Plasma burn slows enemy fire rate by 20%."),
      icon = "chemical-bolt.webp",
   },
   ["plasma4"] = {
      name = _("Corrosion II"),
      tier = 4,
      outfit = "Corrosion II",
      requires = { "plasma3" },
      desc = _("Plasma burn duration further increased by 50%. Bonus is halved for non-organ weapons."),
      icon = "acid-blob.webp",
   },
   ["plasma5"] = {
      name = _("Plasma Burst"),
      tier = 5,
      requires = { "plasma4" },
      desc = _("Creates an explosion of plasma affecting all ships around the pilot. Deals 100 damage with 50% penetration to all hostiles ships within 200 range. Deals an additional 200 damage over 10 seconds while lowering speed, accel, and turn by 25% and fire rate by 20%."),
      outfit = "Plasma Burst",
      icon = "goo-skull.webp",
      slot = "plasma_burst",
   },
}

-- Misc tree
-- For everyone!
-- All intrinsics
skills.set.misc = {
   ["misc1"] = {
      name = _("Cargo Sacs"),
      tier = 1,
      desc = _("Increases cargo space by 100% and lowers cargo inertia by 50%."),
      outfit = "Cargo Sacs",
      icon = "tumor.webp",
   },
   ["misc2"] = {
      name = _("Fuel Bladder"),
      tier = 2,
      requires = { "misc1" },
      desc = _("Increases fuel capacity by 100%."),
      outfit = "Fuel Bladder",
      icon = "jellyfish.webp",
   },
   ["misc3"] = {
      name = _("Adaptive Jump"),
      tier = 3,
      requires = { "misc2" },
      desc = _("Decreases jumping time by 50%, increases jump detection by 50%, and allowed jump distance by 100%."),
      outfit = "Adaptive Jump",
      icon = "galaxy.webp",
   },
   ["misc4"] = {
      name = _("Enhanced Smell"),
      tier = 4,
      requires = { "misc3" },
      desc = _("Increases boarding loot bonus by 100%."),
      outfit = "Enhanced Smell",
      icon = "snout.webp",
   },
   ["misc5"] = {
      name = _("Tunnelling Organ"),
      tier = 5,
      requires = { "misc4" },
      desc = _("Allows for instant jumping and the ship performs an active cooldown cycle on each jump."),
      outfit = "Tunnelling Organ",
      icon = "vortex.webp",
   },
}

function skills.get( sets )
   local s = {}
   for k,v in ipairs(sets) do
      tmerge_r( s, skills.set[v] )
   end
   return s
end

return skills
