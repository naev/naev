local flow = require "ships.lua.lib.flow"

local setup = {}

-- Note that afterburners get added separately now
local usable_outfits = {
   ["Emergency Shield Booster"]  = "shield_booster",
   ["Berserk Chip"]              = "berserk_chip",
   ["Combat Hologram Projector"] = "hologram_projector",
   ["Neural Accelerator Interface"] = "neural_interface",
   ["Flicker Drive"]             = "blink_drive",
   ["Blink Drive"]               = "blink_drive",
   ["Hyperbolic Blink Engine"]   = "blink_engine",
   ["Unicorp Jammer"]            = "jammer",
   ["Milspec Jammer"]            = "jammer",
   ["Weapons Ionizer"]           = "ionizer",
   -- Mining stuff, not strictly combat...
   ["S&K Plasma Drill"]          = "plasma_drill",
   ["S&K Heavy Plasma Drill"]    = "plasma_drill",
   -- Bioships
   ["Feral Rage III"]            = "feral_rage",
   ["The Bite"]                  = "bite",
   ["The Bite - Improved"]       = "bite",
   ["The Bite - Blood Lust"]     = {"bite", "bite_lust"},
   ["Plasma Burst"]              = "plasma_burst",
   -- Flow
   ["Seeking Chakra"]            = "seeking_chakra",
   ["Feather Drive"]             = "feather_drive",
   ["Astral Projection"]         = "astral_projection",
   ["Avatar of the Sirichana"]   = "avatar_sirichana",
   ["Cleansing Flames"]          = "cleansing_flames",
   ["House of Mirrors"]          = "house_mirrors",
   ["Reality Rip"]               = "reality_rip",
}

-- We ignore if plugins are enabled
if __debugging  and #naev.plugins() <= 0 then
   for k,v in pairs(usable_outfits) do
      if not outfit.get(k) then
         warn(_("Unknown outfit"))
      end
   end
end

function setup.setup( p )
   local added = false

   -- Clean up old stuff
   local m = p:memory()
   m._o = nil
   local o = {}

   -- Set up some defaults
   -- 1 is primary
   -- 2 is secondary
   -- 3 has point defense
   -- 4 has fighter bays
   -- 5 is turret weapons

   -- Check flow
   flow.recalculate( p )
   o.flow = flow.has( p )

   -- Check out what interesting outfits there are
   for k,v in ipairs(p:outfits()) do
      if v then
         local var = usable_outfits[ v:nameRaw() ]
         if var then
            if type(var)=="table" then
               for i,t in ipairs(var) do
                  o[t] = k
               end
            else
               o[var] = k
            end
            added = true
         else
            local t = v:type()
            if t=="Afterburner" then
               o["afterburner"] = k
               added = true
            end
         end
      end
   end

   -- Set up the weapon sets
   p:weapsetAuto()

   -- Some tweaks to default AI for certain cases
   -- TODO probably move atk.choose here if we use this in all cases we initialize pilots (see issue #2197)

   -- Set up some ammo variables
   m.ranged_ammo = p:weapsetAmmo(1) -- secondary set
   m.equipopt_params = m.equipopt_params or {}

   -- Actually added an outfit, so we set the list
   if added then
      m._o = o
   end
end

return setup
