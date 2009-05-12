
--[[
-- Creates unique
--]]


function create ()
   sys = system.get()

   -- Find possible uniques
   unique_list = {}
   if sys:hasPresence("Pirate") then
      unique_list[ #unique_list+1 ] = "Pirate"
   end

   -- Choose unique
   unique_class = unique_list[ rnd.rnd(1,#unique_list) ]

   -- Create unique
   if unique_class == "Pirate" then
      include("scripts/pilot/pirate.lua")
      pirate_create()
   end
end

