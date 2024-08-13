--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Naevpedia">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
local naevpedia = require "naevpedia"

local function donaevpedia ()
   naevpedia.open("index")
end

function create ()
   player.infoButtonRegister( _("Naevpedia"), donaevpedia, 3, "N" )
end
