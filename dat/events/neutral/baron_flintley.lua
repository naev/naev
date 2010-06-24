--[[
-- Flintley Event for the Crazy Baron mission string. Only used when NOT doing any Baron missions.
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    desc = "Flintley is here. He nervously sips from his drink, clearly uncomfortable in this environment."
    title = "Flintley"
    text = [[    Flintly greets you, relieved to see a friendly face. "Hello again, %s. What brings you here today? As you can see, I'm here on business again. Nothing too interesting, I'm afraid, just everyday stuff."
    You spend some time chatting with Flintley, then you get back to work.]]
end

function create ()
    evt.npcAdd("flintley", "Flintley", none, desc, 5) -- TODO: portrait for Flintley
end

function flintley()
    tk.msg(title, text:format(player.name()))
end