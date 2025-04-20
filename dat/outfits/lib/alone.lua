
--local fmt = require "format"

local prvdesc=descextra
descextra=function ( p, o, po)
   return prvdesc( p, o, po) .. "\n#b".._("This outfit prevents other engines from being equipped.").."#0"
end

local slotname = 'engines_secondary'
function onoutfitchange( p, po )
   if p and po and p:outfitHasSlot(slotname) then
      local o = p:outfitSlot(slotname)
      if o and p:outfitRmSlot(slotname) and p==player.pilot() then
         player.outfitAdd(o)
         --print(fmt.f("{name} was removed as {myself} refuses its presence.",{
         --   name=o:nameRaw(),myself=po:outfit():nameRaw()
         --}))
      end
   end
end

