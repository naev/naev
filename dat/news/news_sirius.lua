--[[

   Sirius News Generator

--]]


function news_greetSirius ()
   local greet

   -- Set the greeting
   greet = "Sirius News Reel. Words of the Sirichana for all."

   return greet
end


function news_addSirius( rawtable )
   gtable = {
      --[[
         Science and technology
      --]]
      --[[
         Business
      --]]
      {
         title = "Trade Meeting at Lorelei",
         desc = "Lorelei in the Churchill systems is currently being one of the centers of major trade negotiations between the Fyrra and Space Traders Guild. The Fyrra Arch-Canter has indicated that opening up trade routes is a major goal."
      },
      --[[
         Politics
      --]]
      {
         title = "Dvaered extorting pilgrims",
         desc = "Recent pilgrims headed to Mutris have been telling stories of extortion and violations caused by Dvaered. Dvaered War Lord Kra'tok claims that these are \"delusions of the touched\". Official complaints have been filed to the Emperor."
      },
      --[[
         Human interest.
      --]]
      {
         title = "Words of Tranquility",
         desc = "We welcome many new Touched who have recently joined the Shaira echelon after their long pilgrimage on Mutris. House Sirius is still a refugee for the orphans lost in this Universe"
      }
   }

   -- Insert into table
   for i,v in ipairs(gtable) do
      table.insert( rawtable, v )
   end
end

