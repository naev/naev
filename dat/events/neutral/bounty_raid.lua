--[[
   Bountyhunters (independant combat pilots) enter the system and start to attack pilots that are hostiles to the main faction in system
   They are doing patrol missions or bountyhunt missions like the player.
--]]

function create()
   faction1 = system.faction(system.cur())

    if not evt.claim(system.cur()) then
       evt.finish(false)
    end

   --some factions don't use bountyhunters (or their ships are not the same)
   if faction1 ~= nil and faction1:name() ~= "Pirate" and faction1:name() ~= "FLF" and faction1:name() ~= "Thurion" and faction1:name() ~= "Proteron" then

      faction = faction1:name()
      --Look for enemies of the faction
      local atwar = false
      for i,j in ipairs( faction1:enemies() ) do
         if system.cur():presences()[j:name()] ~= nil then
            atwar = true
         end
      end

      if atwar == true then     --There won't be mercenaries in a peacefull system
         hook.timer(2000,"fleet")
         evt.finish()
         else
         evt.finish(false)
      end
   end
end

function fleet ()
   -- Choose and create mercenaries

   mercships = {"Hyena", "Hyena", "Shark", "Shark", "Ancestor", "Lancelot", "Lancelot", "Vendetta", "Phalanx", "Admonisher", "Vigilance", "Pacifier", "Kestrel", "Hawking", "Goddard"}
   proba = 0.5

   for i in ipairs(mercships) do
      if rnd.rnd() < proba then

         p = pilot.addRaw( mercships[i], "mercenary", nil, faction )[1]
         pilot.rename( p, "Bountyhunter" )
         proba = proba - 0.1*i  --This makes the last ships on the list less likely to spawn (I think)
         else
         proba = proba + 0.1
      end
   end
end
