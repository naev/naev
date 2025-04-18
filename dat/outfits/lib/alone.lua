
local fmt = require "format"

local prvdesc=descextra
descextra=function ( p, o, po)
   return prvdesc( p, o, po) .. "\n#b".."This outfit only works alone.".."#0"
end

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

