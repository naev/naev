local lib = {}

--[[--
Sets a factions as known or unknown.
--]]
function lib.setKnown( fct, state )
   fct:setKnown(state)
   if state then
      for k,p in ipairs(pilot.get(fct)) do
         local pm = p:memory()
         if pm.natural and pm.__name then
            p:rename( pm.__name )
         end
      end
   else
      for k,p in ipairs(pilot.get(fct)) do
         local pm = p:memory()
         if pm.natural and not pm.__name then
            pm.__name = p:name()
            p:rename(_("Unknown"))
         end
      end
   end
end

return lib
