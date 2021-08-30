--[[
   Ship Ratings Module

   Contains a rater for all the ship in the game, used for spawning and things like that.
--]]
local shiprater = {}

local rating_table = {
   -- Generic
   ["Schroedinger"]  = 8,
   ["Llama"]         = 8,
   ["Gawain"]        = 8,
   ["Koala"]         = 20,
   ["Quicksilver"]   = 20,
   ["Mule"]          = 50,
   ["Rhino"]         = 70,
   ["Hyena"]         = 13,
   ["Shark"]         = 20,
   ["Lancelot"]      = 30,
   ["Vendetta"]      = 30,
   ["Ancestor"]      = 25,
   ["Admonisher"]    = 45,
   ["Phalanx"]       = 40,
   ["Starbridge"]    = 60,
   ["Vigilance"]     = 70,
   ["Pacifier"]      = 70,
   ["Kestrel"]       = 90,
   ["Hawking"]       = 105,
   ["Goddard"]       = 120,
   -- Empire
   ["Empire Shark"]     = 20,
   ["Empire Lancelot"]  = 30,
   ["Empire Admonisher"]= 45,
   ["Empire Pacifier"]  = 75,
   ["Empire Hawking"]   = 110,
   ["Empire Peacemaker"]= 150,
   -- Dvaered
   ["Dvaered Vendetta"] = 25,
   ["Dvaered Ancestor"] = 20,
   ["Dvaered Phalanx"]  = 45,
   ["Dvaered Vigilance"]= 70,
   ["Dvaered Goddard"]  = 120,
   -- Soromid
   ["Soromid Brigand"]  = 15,
   ["Soromid Reaver"]   = 25,
   ["Soromid Marauder"] = 25,
   ["Soromid Odium"]    = 45,
   ["Soromid Nyx"]      = 75,
   ["Soromid Ira"]      = 140,
   ["Soromid Arx"]      = 160,
   ["Soromid Vox"]      = 200,
   -- Za'lek
   ["Za'lek Scout Drone"]  = 5,
   ["Za'lek Light Drone"]  = 5,
   ["Za'lek Bomber Drone"] = 8,
   ["Za'lek Heavy Drone"]  = 10,
   ["Za'lek Sting"]        = 45,
   ["Za'lek Demon"]        = 85,
   ["Za'lek Mephisto"]     = 140,
   ["Za'lek Diablo"]       = 150,
   ["Za'lek Hephaestus"]   = 200,
   -- Sirius
   ["Sirius Fidelity"]     = 20,
   ["Sirius Shaman"]       = 25,
   ["Sirius Preacher"]     = 45,
   ["Sirius Divinity"]     = 120,
   ["Sirius Dogma"]        = 140,
   ["Sirius Reverence"]    = 0, -- Mission only
   -- Proteron
   ["Proteron Derivative"] = 20,
   ["Proteron Kahan"]      = 65,
   ["Proteron Archimedes"] = 140,
   ["Proteron Watson"]     = 150,
   -- Thurion
   ["Thurion Perspicacity"]= 20,
   ["Thurion Ingenuity"]   = 25,
   ["Thurion Scintillation"]= 25,
   ["Thurion Virtuosity"]  = 45,
   ["Thurion Apprehension"]= 75,
   ["Thurion Taciturnity"] = 80,
   ["Thurion Certitude"]   = 140,
   -- Pirate
   ["Pirate Shark"]     = 20,
   ["Pirate Vendetta"]  = 30,
   ["Pirate Ancestor"]  = 25,
   ["Pirate Phalanx"]   = 45,
   ["Pirate Admonisher"]= 45,
   ["Pirate Rhino"]     = 60,
   ["Pirate Starbridge"]= 70,
   ["Pirate Kestrel"]   = 125,
   -- Collective
   ["Drone"]            = 20,
   ["Heavy Drone"]      = 30,
   -- Misc
   ["Drone (Hyena)"]    = 0, -- Event only
   ["Raelid Outpost"]   = 0, -- Mission only
   ["Raglan Outpost"]   = 0, -- Mission only
   ["Sindbad"]          = 0, -- Mission only
}
-- TODO allow adding entries for mods

if __debugging then
   for k,s in ipairs(ship.getAll()) do
      local name = s:nameRaw()
      local r = rating_table[name]
      if not r then
         warn(string.format(_("Ship '%s' doesn't have rating table entry!"),name))
      end
   end
end

function shiprater.rate( s )
   local name = s
   return rating_table[name]
end

return shiprater
