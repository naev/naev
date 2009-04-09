--[[

   Random Generic News

--]]


function news_greetEmpire ()
   local greet, greettable

   -- Set the greeting
   greet = "Welcome to the Empire News Centre."

   -- Add a little phrase
   greettable = {
      "Fresh news from around the Empire.",
      "Remembering the Incident.",
      "Keeping you informed."
   }

   greet = greet .. "  " .. greettable[ rnd.rnd(1,#greettable) ]

   return greet
end


function news_addEmpire( rawtable )
   gtable = {
      --[[
         Science and technology
      --]]
      {
         title = "Terraforming Emperor's Fist",
         desc = "New bleeding edge terraforming techniques to be tried on Emperor's Fist.  Studies show that these techniques could speed up the terraforming process by up to 40%."
      },
      --[[
         Business
      --]]
      {
         title = "Empire Keeping Traders Safe",
         desc = "Recent studies show that piracy reports on Trader vessels have gone down by up to 40% in some sectors.  This is part of the Empire's commitment to eradicating piracy."
      },
      --[[
         Politics
      --]]
      {
         title = "New Empire Recruits",
         desc = "The Emperor's recruiting strategy a success.  Many new soldiers joining the Empire Armada.  \"We haven't had such a successful campaign in ages!\", said Raid Steele, spokesman for the recruiting campaign."
      },
      --[[
         Human interest.
      --]]
      {
         title = "New Cat in the Empire Family",
         desc = "The Emperor's daughter was recently gifted a cat.  Cat could be named \"Snuggles\" and seems to be all white."
      }
   }

   -- Insert into table
   for i,v in ipairs(gtable) do
      table.insert( rawtable, v )
   end
end

