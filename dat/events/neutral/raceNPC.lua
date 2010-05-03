--[[
    This event adds info on the racing system, but only on Racing Station
]]--

lang=naev.lang()
if lang=="es" then --not translated atm
else
   info_title = "Info"
   info_desc = "Information on the racing system"
   title = {}
   text = {}
   title[1] = "Welcome to the racetrack!"
   text[1] = [[You have arrived at the Racing Station, the only place in the Galaxy where you can participate in the dangerous, but highly lucrative Therdin races. So far, everything here is coming soon, but info will be added here as I add features.]]
end

function create ()
   info_NPC = evt.npcAdd("raceInfo", info_title, "none", info_desc, 1)
   hook.takeoff("leave")
end

function leave ()
   evt.finish() --finish on takeoff
end

function raceInfo ()
   tk.msg(title[1], text[1])
end