--[[

   Random news generator

--]]


include("dat/news/news_generic.lua")
include("dat/news/news_empire.lua")
include("dat/news/news_dvaered.lua")
include("dat/news/news_goddard.lua")


function news( n )

   -- Get the table and greeting
   local greet, rawtable = news_genTable()

   -- Case list is smaller
   if #rawtable < n then
      return greet, rawtable
   end

   -- Now get the n elements
   ntable = {}
   while #ntable < n do
      i = rnd.rnd(1, #rawtable)
      table.insert( ntable, rawtable[i] )
      table.remove( rawtable, i )
   end
   return greet, ntable
end


--[[
   Generates the greeting and table.
--]]
function news_genTable ()

   -- Get current planet.
   local curp, curs = planet.get()
   f = curp:faction()

   rawtable = {}

   -- Create the output
   if f:name() == "Empire" then
      greet = news_greetEmpire()
      news_addGeneric( rawtable )
      news_addEmpire( rawtable )
   elseif f:name() == "Dvaered" then
      greet = news_greetDvaered()
      news_addGeneric( rawtable )
      news_addEmpire( rawtable )
      news_addDvaered( rawtable )
   elseif f:name() == "Goddard" then
      greet = news_greetGoddard()
      news_addGeneric( rawtable )
      news_addEmpire( rawtable )
      news_addGoddard( rawtable )
   else
      greet = news_greetGeneric()
      news_addGeneric( rawtable )
   end

   -- Send output.
   return greet, rawtable
end


