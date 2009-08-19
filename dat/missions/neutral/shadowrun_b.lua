--[[
-- This is a helper script for the Shadowrun mission.
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

   title = {}
   text = {}

   title[1] = "You were ignored"
   text[1] = "You try to strike a conversation with the officer, but he doesn't seem interested what you have to say, so you give up."
   
   -- Mission details
   bar_desc = "You see a military officer with a drink at the bar. He doesn't seem to be very interested in it, though..."
   
end

function create ()
    misn.setNPC( "Officer at the bar", "empire1" )
    misn.setDesc( bar_desc ) 
end

function accept()
    tk.msg(title[1], text[1])
    misn.finish()
end