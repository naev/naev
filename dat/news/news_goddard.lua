--[[

   Random Goddard News

--]]


function news_greetGoddard ()
   local greet

   -- Set the greeting
   greet = "Welcome to Goddard News Centre. We bring you the news from around the Empire."

   return greet
end


function news_addGoddard( rawtable )
   gtable = {
      --[[
         Science and technology
      --]]
      {
         title = "Goddard: Raising the Bar",
         desc = "Many new scientists are being contracted by House Goddard to investigate on possible improvements. This new strategy will increase the gap with the competing ship fabricators."
      },
      --[[
         Business
      --]]
      {
         title = "Goddard Earnings on the Rise",
         desc = "House Goddard has once again increased its earnings. \"Our investment in technology and quality has paid off.\", said Kari Baker, responsible of House Goddard marketing."
      },
      {
         title = "Goddard Awarded Best Ship",
         desc = "Once again the Goddard class Cruiser was awarded the Best Overall Ship award by the Dvaered Armada's annual Ship Awards. \"Very few ships have reliability like the Goddard.\", said Lord Warthon upon receiving the award for House Goddard."
      }
      --[[
         Politics
      --]]
      --[[
         Human interest.
      --]]
   }

   -- Insert into table
   for i,v in ipairs(gtable) do
      table.insert( rawtable, v )
   end
end

