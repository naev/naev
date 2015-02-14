--[[

   Creates unique

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]


function create ()
   sys = system.cur()

   -- Find possible uniques
   unique_list = {}
   if sys:presences()["Pirate"] then
      unique_list[ #unique_list+1 ] = "Pirate"
   end
   if sys:presences()["Empire"] then
      unique_list[ #unique_list+1 ] = "Empire"
   end

   -- Choose unique
   unique_class = unique_list[ rnd.rnd(1,#unique_list) ]

   -- Create unique
   if unique_class == "Pirate" then
      include("pilot/pirate.lua")
      pirate_create()
   elseif unique_class == "Empire" then
      include("pilot/empire.lua")
      empire_create()
   end

   evt.finish()
end

