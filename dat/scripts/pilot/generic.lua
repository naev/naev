require "numstring"

--[[
-- @brief Encodes a number as a roman numeral.
--
--    @param k Number to encode.
--    @return String representing the number as a roman numeral.
--]]
function roman_encode( k )
   local romans = {
      {1000, "M"},
      {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"},
      {90, "XC"}, {50, "L"}, {40, "XL"}, {10, "X"},
      {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"} }

   local s = ""
   for _, v in ipairs(romans) do --note that this is -not- ipairs.
      local val, let = table.unpack(v)
      while k >= val do
         k = k - val
         s = s .. let
      end
   end
   return s
end


--[[
-- @brief Generates generic pilot names
--]]
function pilot_name ()
   -- For translation, please keep in mind that these are names. It is
   -- better to translate in an inaccurate way than to translate in a
   -- way that works less well as a name. %s is replaced by a number or
   -- letter.
   local names = {
      _("Aurochs %s"),
      _("Axis %s"),
      _("Bear %s"),
      _("Beetle %s"),
      _("Black Hole %s"),
      _("Blizzard %s"),
      _("Blood %s"),
      _("Blue %s"),
      _("Boa %s"),
      _("Boulder %s"),
      _("Bullet %s"),
      _("Chainsaw %s"),
      _("Claw %s"),
      _("Cluster %s"),
      _("Cougar %s"),
      _("Crisis %s"),
      _("Crow %s"),
      _("Dart %s"),
      _("Death %s"),
      _("Deity %s"),
      _("Demon %s"),
      _("Diamond %s"),
      _("Dire Wolf %s"),
      _("Dragon %s"),
      _("Eagle %s"),
      _("Earthquake %s"),
      _("Electron %s"),
      _("Falcon %s"),
      _("Fang %s"),
      _("Fire %s"),
      _("Fox %s"),
      _("Glory %s"),
      _("Hammer %s"),
      _("Hawk %s"),
      _("Hornet %s"),
      _("Hunter %s"),
      _("Hurricane %s"),
      _("Ice %s"),
      _("Ion %s"),
      _("Lantern %s"),
      _("Leopard %s"),
      _("Light %s"),
      _("Lion %s"),
      _("Lizard %s"),
      _("Mammoth %s"),
      _("Mantis %s"),
      _("Mech %s"),
      _("Mosquito %s"),
      _("Neutron %s"),
      _("Nova %s"),
      _("Orca %s"),
      _("Panther %s"),
      _("Peril %s"),
      _("Photon %s"),
      _("Pride %s"),
      _("Proton %s"),
      _("Python %s"),
      _("Raven %s"),
      _("Rebirth %s"),
      _("Red %s"),
      _("Raptor %s"),
      _("Rex %s"),
      _("Rocket %s"),
      _("Rogue %s"),
      _("Saber %s"),
      _("Scorpion %s"),
      _("Scythe %s"),
      _("Serpent %s"),
      _("Shadow %s"),
      _("Shark %s"),
      _("Star Killer %s"),
      _("Spade %s"),
      _("Spider %s"),
      _("Talon %s"),
      _("Thunder %s"),
      _("Tiger %s"),
      _("Torch %s"),
      _("Tsunami %s"),
      _("Turtle %s"),
      _("Typhoon %s"),
      _("Velocity %s"),
      _("Vengeance %s"),
      _("Void %s"),
      _("Wasp %s"),
      _("Wind %s"),
      _("Wing %s"),
      _("Wolf %s"),
      _("Wraith %s"),
      _("Zombie %s"),
   }
   local name = names[rnd.rnd(1, #names)]
   local num = rnd.rnd(1, 99)
   local str
   local r = rnd.rnd()
   if r < 0.5 then
      str = tostring(num)
   elseif r < 0.85 then
      str = roman_encode(num)
   else
      local greek = { "α","β", "γ", "δ", "ε", "ζ", "Δ", "Σ", "Ψ", "Ω" }
      str = greek[ math.fmod( num, #greek )+1 ]
      if rnd.rnd() < 0.5 then
         str = str .. string.format("-%d", math.floor(num/#greek+0.5)+1)
      end
   end
   return name:format(str)
end
