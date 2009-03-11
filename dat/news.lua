
--[[

   Random news generator

--]]


function news ()

   -- Only generates when landed
   curp, curs = space.getPlanet()
   if curp == nil then
      return
   end

   -- See what news to generate
   f = curp:faction()
   if f == nil then
      return
   end

   -- Pirate planets get special treatment
   if f:name() == "Pirate" then
      if rnd.rnd() < 0.5 then
         return news_pirate()
      else
         return news_generic()
      end
   end

   -- Empire specifics
   if f:name() == "Empire" and rnd.rnd() < 0.5 then
      return news_empire()
   end

   -- Dvaered specifics
   if f:name() == "Dvaered" and rnd.rnd() < 0.3 then
      return news_dvaered()
   end

   -- Frontier specifics
   if f:name() == "Frontier" and rnd.rnd() < 0.3 then
      return news_frontier()
   end

   -- Generic stuff
   if rnd.rnd() < 0.7 then
      return news_generic()
   else
      return news_neutral()
   end

end


--[[
   Empire news.
--]]
function news_empire ()

   ntable_empire = {
      "New recruits boost Empire's forces.",
      "Emperor's Fist terraforming progressing.",
      "Empire cuts down on piracy.",
      "Empire recruiting to keep the peace."
   }

   return ntable_empire[ rnd.rnd(1, #ntable_empire) ]
end


--[[
   House Dvaered news.
--]]
function news_dvaered ()
   ntable_dvaered = {
      "FLF terrorist detained, execution next STU.",
      "FLF ploy thwarted, terrorists cowardly escaped.",
      "FLF numbers wanings thanks to increase of House Dvaered patrols.",
      "House Dvaered increases patrol frequencies."
   }

   return ntable_dvaered[ rnd.rnd(1, #ntable_dvaered) ]
end


--[[
   Frontier News.
--]]
function news_frontier ()
   ntable_frontier = {
      "House Dvaered crush pacific demonstration, hundreds killed.",
      "Frontier citizens executed unlawfully as FLF members."
   }

   return ntable_frontier[ rnd.rnd(1, #ntable_frontier) ]
end


--[[
   Pirate news.
--]]
function news_pirate ()
   ntable_pirate = {
      "Pirates say piracy is on the rise.",
      "Skull and Bones steals shipment of ships.",
      "Emperor authority weaker then ever.",
      "Admonisher awarded pirate ship of excellence."
   }

   return ntable_pirate[ rnd.rnd(1, #ntable_pirate) ]
end


--[[
   Generic planet news.
--]]
function news_generic ()
   rndp, rnds = space.getPlanet( {
         faction.get("Empire"),
         faction.get("Independent"),
         faction.get("Dvaered"),
         faction.get("Frontier")
         })

   if rndp:services() > 0 then
      return news_habitable( rndp, rnds )
   else
      return news_inhabitable( rndp, rnds )
   end
end


--[[
   Neutral generic news.
--]]
function news_neutral ()
   ntable_neut = {
      "Traders claim more pirate attacks in the last STU then any other.",
      "Scientists claim that man came from so called \"monkey\" that used to live in the ill-fated earth.",
      "Fluctuations spotted in the nebulae.",
      "Trader ship missing.",
      "SPAM reaches new all-time high.",
      "Nexus recieves large Empire grant to develope new warships.",
      "Emperor inaugurates new carrier.",
      "100 STU celebration of the creation of the original Mule.",
      "New memorial to be created for victims of the Incident.",
   }

   return ntable_neut[ rnd.rnd(1, #ntable_neut) ]
end



--[[
   Inhabitable planet news.
--]]
function news_inhabitable( planet, system )

   ntable_inhab = {
      "%s, new garbage planet of the %s system.",
      "Traders claim %s in the %s system could be made habitable and suitable for commercial exploitation.",
      "Scientists discover anomalies in the gravity field of %s in the %s system.",
   }

   return string.format( ntable_inhab[ rnd.rnd(1, #ntable_inhab) ],
         planet:name(), system:name() )
end


--[[
   Habitable planet news.
--]]
function news_habitable( planet, system)

   ntable_inhab = {
      "Official holiday declared at %s in the %s system.",
      "New species found at %s in the %s system.",
      "Population of %s in the %s system deemed the happiest in the universe."
   }

   return string.format( ntable_inhab[ rnd.rnd(1, #ntable_inhab) ],
         planet:name(), system:name() )
end



