--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Bioship Manager">
 <trigger>load</trigger>
 <chance>100</chance>
 <flags>
  <unique />
 </flags>
</event>
--]]
local fmt = require "format"

local remove_types = {
   ["Bioship Organ"] = true,
   ["Bioship Brain"] = true,
   ["Bioship Gene Drive"] = true,
   ["Bioship Shell"] = true,
   ["Bioship Weapon Organ"] = true,
}
function create ()
   -- Try to clean up outfits
   for k,o in ipairs(player.outfits()) do
      if remove_types[ o:type() ] then
         local q = player.numOutfit( o, true )
         player.outfitRm( o, q )
         print(fmt.f(_("Removing unobtainable Bioship outfit '{outfit}'!"),{outfit=o}))
      end
   end
end
