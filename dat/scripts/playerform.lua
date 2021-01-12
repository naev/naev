local formation = require "scripts/formation"

local function playerform()
   local form_names = {}
   for k, v in ipairs(formation.keys) do
      form_names[k] = v:gsub("_", " "):gsub("^%l", string.upper)
   end

   form_names[#form_names+1] = "None"

   local choice = tk.choice("Formation", "Choose a formation.",
                            table.unpack(form_names))

   player.pilot():memory().formation = formation.keys[choice]
end

return playerform
