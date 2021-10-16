local fmt = require "format"
local utility = require 'pilotname.utility'

-- For translation, please keep in mind that these are names. It is
-- better to translate in an inaccurate way than to translate in a
-- way that works less well as a name.
local names = {
   _("Aurochs"),
   _("Axis"),
   _("Bear"),
   _("Beetle"),
   _("Black Hole"),
   _("Blizzard"),
   _("Blood"),
   _("Blue"),
   _("Boa"),
   _("Boulder"),
   _("Bullet"),
   _("Chainsaw"),
   _("Claw"),
   _("Cluster"),
   _("Cougar"),
   _("Crisis"),
   _("Crow"),
   _("Dart"),
   _("Death"),
   _("Deity"),
   _("Demon"),
   _("Diamond"),
   _("Dire Wolf"),
   _("Dragon"),
   _("Eagle"),
   _("Earthquake"),
   _("Electron"),
   _("Falcon"),
   _("Fang"),
   _("Fire"),
   _("Fox"),
   _("Glory"),
   _("Hammer"),
   _("Hawk"),
   _("Hornet"),
   _("Hunter"),
   _("Hurricane"),
   _("Ice"),
   _("Ion"),
   _("Lantern"),
   _("Leopard"),
   _("Light"),
   _("Lion"),
   _("Lizard"),
   _("Mammoth"),
   _("Mantis"),
   _("Mech"),
   _("Mosquito"),
   _("Neutron"),
   _("Nova"),
   _("Orca"),
   _("Panther"),
   _("Peril"),
   _("Photon"),
   _("Pride"),
   _("Proton"),
   _("Python"),
   _("Raven"),
   _("Rebirth"),
   _("Red"),
   _("Raptor"),
   _("Rex"),
   _("Rocket"),
   _("Rogue"),
   _("Saber"),
   _("Scorpion"),
   _("Scythe"),
   _("Serpent"),
   _("Shadow"),
   _("Shark"),
   _("Star Killer"),
   _("Spade"),
   _("Spider"),
   _("Talon"),
   _("Thunder"),
   _("Tiger"),
   _("Torch"),
   _("Tsunami"),
   _("Turtle"),
   _("Typhoon"),
   _("Velocity"),
   _("Vengeance"),
   _("Void"),
   _("Wasp"),
   _("Wind"),
   _("Wing"),
   _("Wolf"),
   _("Wraith"),
   _("Zombie"),
}

--[[
-- @brief Generates generic pilot names
--]]
local function generic ()
   local name = names[rnd.rnd(1, #names)]
   local num = rnd.rnd(1, 99)
   local str
   local r = rnd.rnd()
   if r < 0.5 then
      str = tostring(num)
   elseif r < 0.85 then
      str = utility.roman_encode(num)
   else
      local greek = { "α","β", "γ", "δ", "ε", "ζ", "Δ", "Σ", "Ψ", "Ω" }
      str = greek[ math.fmod( num, #greek )+1 ]
      if rnd.rnd() < 0.5 then
         str = fmt.f(_("{letter}-{number}"), {letter=str, number=math.floor(num/#greek+0.5)+1})
      end
   end
   return fmt.f(_("{name} {suffix}"), {name=name, suffix=str})
end

return generic
