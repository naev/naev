
local fmt = require "format"

function onoutfitchange( p, po )
   if p and po then
      local o = p:outfitSlot('engines_secondary')
      if o and p:outfitRmSlot('engines_secondary') and p==player.pilot() then
         player.outfitAdd(o)
         print(fmt.f("{name} was removed as {myself} refuses its presence.",{
            name=o:nameRaw(),myself=po:outfit():nameRaw()
         }))
      end
   end
end
