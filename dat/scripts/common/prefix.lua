local pir = require "common.pirate"

local prefix = {}

function prefix.prefix( fct )
   fct = faction.get(fct)
   if fct == faction.get("Za'lek") then
      return require("common.zalek").prefix
   elseif fct == faction.get("Empire") then
      return require("common.empire").prefix
   elseif fct == faction.get("Dvaered") then
      return require("common.dvaered").prefix
   elseif fct == faction.get("Soromid") then
      return require("common.soromid").prefix
   elseif fct == faction.get("Sirius") then
      return require("common.sirius").prefix
   elseif fct == faction.get("Frontier") then
      return require("common.frontier").prefix
   elseif inlist( pir.factions, fct ) then
      return pir.prefix( fct )
   end
   return ""
end

return prefix
