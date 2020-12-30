--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Minerva Station Altercation 1">
 <trigger>land</trigger>
 <chance>100</chance>
 <cond>planet.cur():name()=="Minerva Station" and player.misnDone("Maikki's Father 2")</cond>
 <flags>
  <unique />
 </flags>
</event>
--]]

function create()
   evt.finish(false)
end
