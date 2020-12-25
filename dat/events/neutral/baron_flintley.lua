--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Flintley">
  <trigger>land</trigger>
  <chance>100</chance>
  <cond>player.misnActive("Baron") == false and planet.cur() == planet.get("Tau Station") and player.misnDone("Prince")</cond>
  <notes>
   <done_misn name="Prince"/>
   <campaign>Baron Sauterfeldt</campaign>
  </notes>
 </event>
 --]]
--[[
-- Flintley Event for the Baron mission string. Only used when NOT doing any Baron missions.
--]]

-- localization stuff, translators would work here
desc = _("Flintley is here. He nervously sips from his drink, clearly uncomfortable in this environment.")
title = _("Flintley")
text = _([[    Flintley greets you, relieved to see a friendly face. "Hello again, %s. What brings you here today? As you can see, I'm here on business again. Nothing too interesting, I'm afraid, just everyday stuff."
    You spend some time chatting with Flintley, then you get back to work.]])

function create ()
    evt.npcAdd("flintley", "Flintley", "neutral/unique/flintley", desc, 5)
end

function flintley()
    tk.msg(title, text:format(player.name()))
end
