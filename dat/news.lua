
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
   if f:name() == "Empire" and rnd.rnd() > 0.5 then
      return news_empire()
   else
      if rnd.rnd() < 0.5 then
         return news_generic()
      else
         return news_neutral()
      end
   end

end


function news_empire ()

   ntable_empire = {
      "New recruits boost Empire's forces.",
      "Emperor's Fist terraforming progressing.",
      "Empire cuts down on piracy."
   }

   return ntable_empire[ rnd.rnd(1, #ntable_empire) ]
end


function news_generic ()
   rndp, rnds = space.getPlanet(true)

   if rndp:services() > 0 then
      return news_habitable( rndp, rnds )
   else
      return news_inhabitable( rndp, rnds )
   end
end


function news_neutral ()
   ntable_neut = {
      "Traders claim more pirate attacks in the last STU then any other.",
       "Scientists claim that man came from so called \"monkey\" that used to live in the ill-fated earth.",
       "Fluctuations spotted in the nebulae.",
       "Trader ship missing."
   }

   return ntable_neut[ rnd.rnd(1, #ntable_neut) ]
end


function news_inhabitable( planet, system )

   ntable_inhab = {
      "%s, new garbage planet of the %s system.",
      "Traders claim %s in the %s system could be made habitable and suitable for commercial exploitation.",
      "Scientists discover anomalies in the gravity field of %s in the %s system."
   }

   return string.format( ntable_inhab[ rnd.rnd(1, #ntable_inhab) ],
         planet:name(), system:name() )
end

function news_habitable( planet, system)

   ntable_inhab = {
      "Official holiday declared at %s in the %s system.",
      "New species found at %s in the %s system.",
      "Population of %s in the %s system deemed the happiest in the universe."
   }

   return string.format( ntable_inhab[ rnd.rnd(1, #ntable_inhab) ],
         planet:name(), system:name() )
end



