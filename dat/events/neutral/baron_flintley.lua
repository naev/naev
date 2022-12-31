--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Flintley">
 <location>land</location>
 <chance>100</chance>
 <cond>player.misnActive("Baron") == false and spob.cur() == spob.get("Tau Station") and player.misnDone("Prince")</cond>
 <notes>
  <done_misn name="Prince"/>
  <campaign>Baron Sauterfeldt</campaign>
 </notes>
</event>
--]]
--[[
-- Flintley Event for the Baron mission string. Only used when NOT doing any Baron missions.
--]]

local fmt = require "format"


function create ()
    evt.npcAdd("flintley", _("Flintley"), "neutral/unique/flintley.webp", _("Flintley is here. He nervously sips from his drink, clearly uncomfortable in this environment."), 5)
end

function flintley()
    tk.msg(_("Flintley"), fmt.f(_([[    Flintley greets you, relieved to see a friendly face. "Hello again, {player}. What brings you here today? As you can see, I'm here on business again. Nothing too interesting, I'm afraid, just everyday stuff."
    You spend some time chatting with Flintley, then you get back to work.]]), {player=player.name()}))
end
