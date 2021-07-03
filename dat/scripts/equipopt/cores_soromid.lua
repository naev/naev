--[[
   Exceptions for Soromid Ships
--]]
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
local function cores_soromid( cores )

--[[
      STANDARD SYSTEMS
--]]
cores.standard.systems["Soromid Brigand"] = function( heavy )
   return choose_one{
      "Ultralight Brain Stage 1", "Ultralight Brain Stage 2",
      "Ultralight Brain Stage X",
   }
end
cores.standard.systems["Soromid Reaver"] = function( heavy )
   return choose_one{
      "Light Brain Stage 1", "Light Brain Stage 2",
      "Light Brain Stage 3", "Light Brain Stage X",
   }
end
cores.standard.systems["Soromid Marauder"] = cores.standard.systems["Soromid Reaver"]
cores.standard.systems["Soromid Odium"] = function( heavy )
   return choose_one{
      "Medium Brain Stage 1", "Medium Brain Stage 2",
      "Medium Brain Stage 3", "Medium Brain Stage 4",
      "Medium Brain Stage X",
   }
end
cores.standard.systems["Soromid Nyx"] = function( heavy )
   return choose_one{
      "Medium-Heavy Brain Stage 1", "Medium-Heavy Brain Stage 2",
      "Medium-Heavy Brain Stage 3", "Medium-Heavy Brain Stage 4",
      "Medium-Heavy Brain Stage 5", "Medium-Heavy Brain Stage X",
   }
end
cores.standard.systems["Soromid Ira"] = function( heavy )
   return choose_one{
      "Heavy Brain Stage 1", "Heavy Brain Stage 2",
      "Heavy Brain Stage 3", "Heavy Brain Stage 4",
      "Heavy Brain Stage 5", "Heavy Brain Stage 6",
      "Heavy Brain Stage X",
   }
end
cores.standard.systems["Soromid Arx"] = function( heavy )
   return choose_one{
      "Superheavy Brain Stage 1", "Superheavy Brain Stage 2",
      "Superheavy Brain Stage 3", "Superheavy Brain Stage 4",
      "Superheavy Brain Stage 5", "Superheavy Brain Stage 6",
      "Superheavy Brain Stage 7", "Superheavy Brain Stage X",
   }
end
cores.standard.systems["Soromid Vox"] = cores.standard.systems["Soromid Arx"]

--[[
      STANDARD HULLS
--]]
cores.standard.hulls["Soromid Brigand"] = function( heavy )
   return choose_one{
      "Ultralight Shell Stage 1", "Ultralight Shell Stage 2",
      "Ultralight Shell Stage X",
   }
end
cores.standard.hulls["Soromid Reaver"] = function( heavy )
   return choose_one{
      "Light Shell Stage 1", "Light Shell Stage 2",
      "Light Shell Stage 3", "Light Shell Stage X",
   }
end
cores.standard.hulls["Soromid Marauder"] = cores.standard.hulls["Soromid Reaver"]
cores.standard.hulls["Soromid Odium"] = function( heavy )
   return choose_one{
      "Medium Shell Stage 1", "Medium Shell Stage 2",
      "Medium Shell Stage 3", "Medium Shell Stage 4",
      "Medium Shell Stage X",
   }
end
cores.standard.hulls["Soromid Nyx"] = function( heavy )
   return choose_one{
      "Medium-Heavy Shell Stage 1", "Medium-Heavy Shell Stage 2",
      "Medium-Heavy Shell Stage 3", "Medium-Heavy Shell Stage 4",
      "Medium-Heavy Shell Stage 5", "Medium-Heavy Shell Stage X",
   }
end
cores.standard.hulls["Soromid Ira"] = function( heavy )
   return choose_one{
      "Heavy Shell Stage 1", "Heavy Shell Stage 2",
      "Heavy Shell Stage 3", "Heavy Shell Stage 4",
      "Heavy Shell Stage 5", "Heavy Shell Stage 6",
      "Heavy Shell Stage X",
   }
end
cores.standard.hulls["Soromid Arx"] = function( heavy )
   return choose_one{
      "Superheavy Shell Stage 1", "Superheavy Shell Stage 2",
      "Superheavy Shell Stage 3", "Superheavy Shell Stage 4",
      "Superheavy Shell Stage 5", "Superheavy Shell Stage 6",
      "Superheavy Shell Stage 7", "Superheavy Shell Stage X",
   }
end
cores.standard.hulls["Soromid Vox"] = cores.standard.hulls["Soromid Arx"]

--[[
      STANDARD ENGINES
--]]
cores.standard.engines["Soromid Brigand"] = function( heavy )
   return choose_one{
      "Ultralight Fast Gene Drive Stage 1", "Ultralight Fast Gene Drive Stage 2",
      "Ultralight Fast Gene Drive Stage X",
   }
end
cores.standard.engines["Soromid Reaver"] = function( heavy )
   return choose_one{
      "Light Fast Gene Drive Stage 1", "Light Fast Gene Drive Stage 2",
      "Light Fast Gene Drive Stage 3", "Light Fast Gene Drive Stage X",
   }
end
cores.standard.engines["Soromid Marauder"] = cores.standard.engines["Soromid Reaver"]
cores.standard.engines["Soromid Odium"] = function( heavy )
   return choose_one{
      "Medium Fast Gene Drive Stage 1", "Medium Fast Gene Drive Stage 2",
      "Medium Fast Gene Drive Stage 3", "Medium Fast Gene Drive Stage 4",
      "Medium Fast Gene Drive Stage X",
   }
end
cores.standard.engines["Soromid Nyx"] = function( heavy )
   return choose_one{
      "Medium-Heavy Fast Gene Drive Stage 1", "Medium-Heavy Fast Gene Drive Stage 2",
      "Medium-Heavy Fast Gene Drive Stage 3", "Medium-Heavy Fast Gene Drive Stage 4",
      "Medium-Heavy Fast Gene Drive Stage 5", "Medium-Heavy Fast Gene Drive Stage X",
   }
end
cores.standard.engines["Soromid Ira"] = function( heavy )
   return choose_one{
      "Heavy Fast Gene Drive Stage 1", "Heavy Fast Gene Drive Stage 2",
      "Heavy Fast Gene Drive Stage 3", "Heavy Fast Gene Drive Stage 4",
      "Heavy Fast Gene Drive Stage 5", "Heavy Fast Gene Drive Stage 6",
      "Heavy Fast Gene Drive Stage X",
   }
end
cores.standard.engines["Soromid Arx"] = function( heavy )
   return choose_one{
      "Superheavy Strong Gene Drive Stage 1", "Superheavy Strong Gene Drive Stage 2",
      "Superheavy Strong Gene Drive Stage 3", "Superheavy Strong Gene Drive Stage 4",
      "Superheavy Strong Gene Drive Stage 5", "Superheavy Strong Gene Drive Stage 6",
      "Superheavy Strong Gene Drive Stage 7", "Superheavy Strong Gene Drive Stage X",
   }
end
cores.standard.engines["Soromid Vox"] = cores.standard.engines["Soromid Arx"]


--[[
      ELITE SYSTEMS
--]]
cores.elite.systems["Soromid Brigand"] = function( heavy )
   return "Ultralight Brain Stage X"
end
cores.elite.systems["Soromid Reaver"] = function( heavy )
   return "Light Brain Stage X"
end
cores.elite.systems["Soromid Marauder"] = cores.elite.systems["Soromid Reaver"]
cores.elite.systems["Soromid Odium"] = function( heavy )
   return "Medium Brain Stage X"
end
cores.elite.systems["Soromid Nyx"] = function( heavy )
   return "Medium-Heavy Brain Stage X"
end
cores.elite.systems["Soromid Ira"] = function( heavy )
   return "Heavy Brain Stage X"
end
cores.elite.systems["Soromid Arx"] = function( heavy )
   return "Superheavy Brain Stage X"
end
cores.elite.systems["Soromid Vox"] = cores.elite.systems["Soromid Arx"]

--[[
      ELITE HULLS
--]]
cores.elite.hulls["Soromid Brigand"] = function( heavy )
   return "Ultralight Shell Stage X"
end
cores.elite.hulls["Soromid Reaver"] = function( heavy )
   return "Light Shell Stage X"
end
cores.elite.hulls["Soromid Marauder"] = cores.elite.hulls["Soromid Reaver"]
cores.elite.hulls["Soromid Odium"] = function( heavy )
   return "Medium Shell Stage X"
end
cores.elite.hulls["Soromid Nyx"] = function( heavy )
   return "Medium-Heavy Shell Stage X"
end
cores.elite.hulls["Soromid Ira"] = function( heavy )
   return "Heavy Shell Stage X"
end
cores.elite.hulls["Soromid Arx"] = function( heavy )
   return "Superheavy Shell Stage X"
end
cores.elite.hulls["Soromid Vox"] = cores.elite.hulls["Soromid Arx"]

--[[
      ELITE ENGINES
--]]
cores.elite.engines["Soromid Brigand"] = function( heavy )
   return "Ultralight Fast Gene Drive Stage X"
end
cores.elite.engines["Soromid Reaver"] = function( heavy )
   return "Light Fast Gene Drive Stage X"
end
cores.elite.engines["Soromid Marauder"] = cores.elite.engines["Soromid Reaver"]
cores.elite.engines["Soromid Odium"] = function( heavy )
   return "Medium Fast Gene Drive Stage X"
end
cores.elite.engines["Soromid Nyx"] = function( heavy )
   return "Medium-Heavy Fast Gene Drive Stage X"
end
cores.elite.engines["Soromid Ira"] = function( heavy )
   return "Heavy Fast Gene Drive Stage X"
end
cores.elite.engines["Soromid Arx"] = function( heavy )
   return "Superheavy Strong Gene Drive Stage X"
end
cores.elite.engines["Soromid Vox"] = cores.elite.engines["Soromid Arx"]

end

return cores_soromid
