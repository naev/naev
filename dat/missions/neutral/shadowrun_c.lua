--[[
-- This is a helper script for the Shadowrun mission.
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

   title = {}
   text = {}

   title[1] = "You were shooed away"
   text[1] = [["Leave me alone. Can't you see I'm busy?"]]
   
   -- Mission details
   bar_desc = "You see a soldier at a news kiosk. For some reason, he keeps reading the same articles over and over again."
   
end

function create ()
    misn.setNPC( "Soldier at the news kiosk", "empire2" )
    misn.setDesc( bar_desc ) 
end

function accept()
    tk.msg(title[1], text[1])
    misn.finish()
end