--[[

   Random news generator

--]]


-- News files to include
include("dat/news/news_generic.lua")
include("dat/news/news_empire.lua")
include("dat/news/news_dvaered.lua")
include("dat/news/news_goddard.lua")
include("dat/news/news_pirate.lua")
include("dat/news/news_sirius.lua")


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
   local curp, curs = planet.cur()
   f = curp:faction()

   -- Planets with no faction have no news.
   if f == nil then
      return
   end

   rawtable = {}

   -- Empire news
   if f:name() == "Empire" then
      greet = news_greetEmpire()
      news_addGeneric( rawtable )
      news_addEmpire( rawtable )

   -- Dvaered news
   elseif f:name() == "Dvaered" then
      greet = news_greetDvaered()
      news_addGeneric( rawtable )
      news_addEmpire( rawtable )
      news_addDvaered( rawtable )

   -- Goddard news
   elseif f:name() == "Goddard" then
      greet = news_greetGoddard()
      news_addGeneric( rawtable )
      news_addEmpire( rawtable )
      news_addGoddard( rawtable )

   -- Pirate news
   elseif f:name() == "Pirate" then
      greet = news_greetPirate()
      news_addGeneric( rawtable )
      news_addPirate( rawtable )

   -- Sirius news
   elseif f:name() == "Sirius" then
      greet = news_greetSirius()
      news_addGeneric( rawtable )
      news_addEmpire( rawtable )
      news_addSirius( rawtable )

   -- Generic news
   else
      greet = news_greetGeneric()
      news_addGeneric( rawtable )
   end

   -- Send output.
   return greet, rawtable
end


