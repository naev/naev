-- Script to help with faction standing requirements and edit them easily
return function( fctname, thr )
   local fct = faction.get(fctname)
   return math.floor(math.max(system.cur():reputation(fct),fct:reputationGlobal())+0.5) >= thr
end
