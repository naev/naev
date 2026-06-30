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
   elseif fct == faction.get("Goddard") then
      return require("common.goddard").prefix
   elseif fct == faction.get("Proteron") then
      return require("common.proteron").prefix
   elseif fct == faction.get("Yetmer") then
      return require("common.yetmer").prefix
   elseif fct == faction.get("Traders Society") then
      return require("common.trader").prefix
   elseif inlist( pir.factions, fct ) then
      return pir.prefix( fct )
   end
   return ""
end

function prefix.colour( fct )
   fct = faction.get(fct)
   if fct == faction.get("Za'lek") then
      return require("common.zalek").colour
   elseif fct == faction.get("Empire") then
      return require("common.empire").colour
   elseif fct == faction.get("Dvaered") then
      return require("common.dvaered").colour
   elseif fct == faction.get("Soromid") then
      return require("common.soromid").colour
   elseif fct == faction.get("Sirius") then
      return require("common.sirius").colour
   elseif fct == faction.get("Frontier") then
      return require("common.frontier").colour
   elseif fct == faction.get("Goddard") then
      return require("common.goddard").colour
   elseif fct == faction.get("Proteron") then
      return require("common.proteron").colour
   elseif fct == faction.get("Yetmer") then
      return require("common.yetmer").colour
   elseif fct == faction.get("Traders Society") then
      return require("common.trader").colour
   elseif inlist( pir.factions, fct ) then
      return pir.colour
   end
   return ""
end

return prefix
