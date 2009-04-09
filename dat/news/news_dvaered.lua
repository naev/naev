--[[

   Dvaered News Generator

--]]


function news_greetDvaered ()
   local greet

   -- Set the greeting
   greet = "Welcome to the Dvaered News Centre.  All that happens.  In simple words.  So you can understand."

   return greet
end


function news_addDvaered( rawtable )
   gtable = {
      --[[
         Science and technology
      --]]
      {
         title = "New Mace Rockets",
         desc = "Dvaered Engineers are proud to present the new improved version of the Dvaered Mace rocket.  \"We have proven the new rocket to be nearly twice as destructive as the previous versions.\", says Chief Dvaered Engineer."
      }
      --[[
         Business
      --]]
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

