local formation = require "formation"

local function playerform()
   local form_names = tcopy( formation.names )
   form_names[#form_names+1] = p_("formation", "None")

   local choice = tk.choice(_("Formation"), _("Choose a formation."),
                            table.unpack(form_names))

   player.pilot():memory().formation = formation.keys[choice]
   var.push("player_formation", choice)
end

return playerform
