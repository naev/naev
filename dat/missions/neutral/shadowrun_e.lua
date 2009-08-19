--[[
-- This is a helper script for the Shadowrun mission.
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

   title = {}
   text = {}

   title[1] = "They didn't need a third man"
   text[1] = "They don't seem to appreciate your company. You decide to leave them to their game."
   
   -- Mission details
   bar_desc = "Two soldiers are sharing a table near the exit, playing cards. Neither of them seems very into the game."
   
end

function create ()
    misn.setNPC( "Card-playing soldier B", "empire2" )
    misn.setDesc( bar_desc ) 
end

function accept()
    tk.msg(title[1], text[1])
    misn.finish()
end