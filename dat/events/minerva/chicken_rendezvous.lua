--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Chicken Rendezvous">
 <trigger>none</trigger>
 <chance>0</chance>
 <flags>
  <unique />
 </flags>
 <notes>
  <campaign>Minerva</campaign>
  <done_evt name="Spa Propaganda" />
  <requires name="Chicken Rendezvous" />
 </notes>
</event>
--]]

--[[
-- Triggered from station.lua
--]]

function create()
   evt.finish(false)
end
