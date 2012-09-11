-- Removes all pilots belonging to a faction from the system.
-- Takes the name of the faction, as a string.
-- 
-- Example usage: pilot.clearSelect("Empire")
function pilot.clearSelect(facname)
   local ps = pilot.get({faction.get(facname)})
   for _, j in ipairs(ps) do
      if j:exists() then
         j:rm()
      end
   end
end
