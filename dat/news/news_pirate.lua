--[[

   Random Pirate News

--]]


function news_greetPirate ()
   local greet, greettable

   -- Set the greeting
   greet = "Pirate News. News that matters."

   greettable = {
      "News that matters.",
      "Adopt a cat today!",
      "Laughing at the Emperor.",
      "On top of the world.",
      "Piracy has never been better."
   }

   greet = greet .. "  " .. greettable[ rnd.rnd(1,#greettable) ]

   return greet
end


function news_addPirate( rawtable )
   gtable = {
      --[[
         Science and technology
      --]]
      {
         title = "Skull and Bones Improving",
         desc = "The technology behind Skull and Bones is rising. Not only do they steal ships, but they improve on the original design. \"This gives us pirates an edge against the injustice of the Empire.\", says Millicent Felecia Black, lead Skull and Bones engineer."
      },
      --[[
         Business
      --]]
      {
         title = "Draconis Favorite Plundering Space",
         desc = "Draconis has recently passed Delta Pavonis in the pirate polls as favorite plundering space. The abundance of Traders and interference make it an excellent place to get some fast credits."
      },
      {
         title = "New Ships for Skull and Bones",
         desc = "The Skull and Bones was able to extract a few dozen high quality vessels from Caladan warehouses under the nose of the Empire. These ships will help keep production high and booming."
      },
      --[[
         Politics
      --]]
      {
         title = "Emperor Weaker than Ever",
         desc = "Recent actions demonstrate the inefficiency and weakness of the Emperor. One of the last irrational decisions left Eridani without a defense fleet to protect the traders. It's a great time to be a pirate."
      },
      --[[
         Human interest.
      --]]
      {
         title = "Cats in New Haven",
         desc = "An explosion in the cat population of New Haven has created an adoption campaign with the slogan \"Pirate Cats, for those lonely space trips.\". Is your space vessel full of vermin?  Adopt a cat today!"
      }
   }

   -- Insert into table
   for i,v in ipairs(gtable) do
      table.insert( rawtable, v )
   end
end

